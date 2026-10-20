#include "shtest.hh"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <termios.h>

// shtest.cc
//    Implementation of `sh61_tester`.
//    You shouldn’t need to read this, but do read shtest.hh.
//
//    At root, `sh61_tester` forks a child process that runs `sh61` on a
//    script file. But rather than a single child, `sh61_tester` runs `sh61`
//    and its children on a new *pseudo-terminal*: a software terminal, like
//    the one behind your terminal window, that belongs to this test alone.
//    This lets the tester find every process the shell creates (including
//    zombies), type Control-C the way a user would, and kill leftover
//    processes when the test ends. A process called the *governor* manages
//    the pseudo-terminal:
//
//    +--------------+  fork   +--------------+  fork   +--------------+
//    |    tester    |-------->|   governor   |-------->|  sh61 and    |
//    |  (runs the   |         | (manages the |         |  its child   |
//    | test `main`) |<========|   terminal)  |         |  processes   |
//    +--------------+  exit   +--------------+         +--------------+
//                     report
//                      pipe
//
//    `run` forks the governor. The governor starts a new session whose
//    controlling terminal is the pseudo-terminal, then forks `sh61` into a
//    new process group in the foreground of that terminal. `sh61` reads its
//    standard input from the run directory's `script` file and writes its
//    standard output and error to `stdout` and `stderr` there; the
//    pseudo-terminal is only its controlling terminal (`/dev/tty`).
//
//    When `sh61` exits, the governor writes its exit status to the pipe. To
//    interrupt the shell, the tester types Control-C on the pseudo-terminal's
//    master side, and the terminal sends SIGINT to its foreground process
//    group. When the tester exits or sends the governor SIGTERM, the governor
//    kills every process remaining on the pseudo-terminal, removes the run
//    directory, and exits.
//
//    The governor must outlive the shell's processes. When a session leader
//    exits, its session loses its controlling terminal, and leftover
//    processes would no longer show up in `ps -t`.

namespace fs = std::filesystem;

namespace {

const rlim_t output_limit = 100000;
volatile sig_atomic_t governor_stopping = 0;

double now() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

// Message from the governor when the shell exits
struct exit_report {
    int status;           // shell's wait status
    double elapsed;       // seconds the shell ran
};

void set_cloexec(int fd) {
    fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

// Signal handlers for the governor. `wake` does nothing, but receiving a
// signal interrupts the governor's `poll`.
void wake(int) {
}

void request_stop(int) {
    governor_stopping = 1;
}

// close_extra_fds()
//    Closes every file descriptor above 2, so the shell starts with only
//    standard input, output, and error open.
void close_extra_fds() {
#ifdef __linux__
    if (close_range(3, ~0U, 0) == 0) {
        return;
    }
#endif
    long max = sysconf(_SC_OPEN_MAX);
    if (max < 0 || max > 4096) {
        max = 4096;
    }
    for (int fd = 3; fd < max; ++fd) {
        close(fd);
    }
}

}


sh61_tester::sh61_tester(int argc, char* argv[]) {
    name_ = fs::path(argv[0]).filename().string();
    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        if (opt != 'd') {
            fprintf(::stderr, "Usage: %s [-d RUNDIR]\n", name_.c_str());
            exit(1);
        }
        run_dir_ = fs::absolute(optarg).string();
        keep_run_dir_ = true;
    }
    if (optind != argc) {
        fprintf(::stderr, "Usage: %s [-d RUNDIR]\n", name_.c_str());
        exit(1);
    }
    test_dir_ = fs::current_path().string();
    if (keep_run_dir_) {
        if (!fs::is_directory(run_dir_)) {
            fprintf(::stderr, "%s: %s: No such directory\n", name_.c_str(), run_dir_.c_str());
            exit(1);
        }
        return;
    }
    run_dir_ = test_dir_ + "/out/" + name_ + "-" + std::to_string(getpid());
    std::error_code ec;
    fs::remove_all(run_dir_, ec);
    fs::create_directories(run_dir_, ec);
    if (ec) {
        fprintf(::stderr, "shtest: cannot create %s: %s\n", run_dir_.c_str(), ec.message().c_str());
        exit(1);
    }
}

sh61_tester::~sh61_tester() {
    stop_governor();
    if (!keep_run_dir_) {
        std::error_code ec;
        fs::remove_all(run_dir_, ec);
    }
}

void sh61_tester::write_file(std::string_view name, std::string_view contents) {
    std::string path = run_dir_ + "/" + std::string(name);
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(contents.data(), contents.size());
    if (!f) {
        fprintf(::stderr, "shtest: cannot write %s\n", path.c_str());
        exit(1);
    }
}

std::string sh61_tester::file_contents(std::string_view name) const {
    std::ifstream f(std::format("{}/{}", run_dir_, name), std::ios::binary);
    std::ostringstream buf;
    buf << f.rdbuf();
    return buf.str();
}


void sh61_tester::run(std::string_view script) {
    if (started_) {
        fail("`run` called twice");
    }
    started_ = true;
    std::ofstream script_file(run_dir_ + "/script", std::ios::binary);
    script_file << script;
    if (script.empty() || script.back() != '\n') {
        script_file << '\n';
    }
    if (!(script_file << std::flush)) {
        fail("cannot write script");
    }

    master_fd_ = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd_ < 0 || grantpt(master_fd_) < 0 || unlockpt(master_fd_) < 0) {
        fail("cannot create pseudo-terminal");
    }
    std::string slave_name = ptsname(master_fd_);
    tty_name_ = slave_name.starts_with("/dev/") ? slave_name.substr(5) : slave_name;
    int reportpipe[2];
    if (pipe(reportpipe) < 0) {
        fail("cannot create pipe");
    }
    set_cloexec(master_fd_);
    set_cloexec(reportpipe[0]);
    set_cloexec(reportpipe[1]);

    fflush(::stdout);
    fflush(::stderr);
    start_time_ = now();
    governor_pid_ = fork();
    if (governor_pid_ < 0) {
        fail("cannot fork");
    } else if (governor_pid_ == 0) {
        close(reportpipe[0]);
        run_governor(reportpipe[1]);
    }
    close(reportpipe[1]);
    report_fd_ = reportpipe[0];
    fcntl(report_fd_, F_SETFL, fcntl(report_fd_, F_GETFL) | O_NONBLOCK);
    running_ = true;

    // Wait for the shell to exit and for the `after` functions to run
    std::stable_sort(afters_.begin(), afters_.end(),
                     [](auto& a, auto& b) { return a.first < b.first; });
    size_t next_after = 0;
    useconds_t delay = 100;
    while (true) {
        check_exit();
        double t = now() - start_time_;
        if (next_after < afters_.size() && t >= afters_[next_after].first) {
            afters_[next_after].second();
            ++next_after;
        } else if (!running_ && next_after == afters_.size()) {
            return;
        } else if (running_ && t > time_limit_) {
            elapsed_ = t;
            fail("shell did not exit within the time limit");
        } else {
            useconds_t sleep_time = delay;
            if (next_after < afters_.size()) {
                double until_after = (afters_[next_after].first - t) * 1e6;
                if (until_after < sleep_time) {
                    sleep_time = useconds_t(until_after);
                }
            }
            usleep(sleep_time);
            delay = std::min(delay * 2, useconds_t(5000));
        }
    }
}

std::vector<std::string> sh61_tester::shell_command() const {
    std::vector<std::string> words;
    if (const char* env = getenv("SHTEST_SHELL"); env && *env) {
        std::istringstream in(env);
        std::string word;
        while (in >> word) {
            words.push_back(word);
        }
    } else {
        words = { test_dir_ + "/sh61", "-q" };
    }
    return words;
}

// run_governor(fd)
//    Runs in the governor. Starts the shell on the pseudo-terminal, reading
//    `script` and writing `stdout` and `stderr` in the run directory, and
//    writes its exit report to `fd`. When the tester exits or sends SIGTERM,
//    cleans up and exits.
void sh61_tester::run_governor(int fd) {
    pid_t tester = getppid();
    int infd = open((run_dir_ + "/script").c_str(), O_RDONLY | O_CLOEXEC);
    int outfd = open((run_dir_ + "/stdout").c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    int errfd = open((run_dir_ + "/stderr").c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (infd < 0 || outfd < 0 || errfd < 0) {
        fprintf(::stderr, "shtest: cannot open files in %s: %s\n", run_dir_.c_str(), strerror(errno));
        _exit(1);
    }
    setsid();
    int tty_fd = open(ptsname(master_fd_), O_RDWR | O_CLOEXEC);
    if (tty_fd < 0) {
        _exit(1);
    }
    ioctl(tty_fd, TIOCSCTTY, 0);
    // A process outside the foreground process group that calls `tcsetpgrp`
    // gets SIGTTOU, so ignore it. Ignore SIGPIPE so the governor survives
    // writing its report after the tester exits.
    signal(SIGTTOU, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    // SIGCHLD interrupts `poll`, so the governor notices the shell's exit
    struct sigaction sa = {};
    sa.sa_handler = wake;
    sigaction(SIGCHLD, &sa, nullptr);
    sa.sa_handler = request_stop;
    sigaction(SIGTERM, &sa, nullptr);

    std::vector<std::string> words = shell_command();
    std::vector<char*> argv;
    for (auto& w : words) {
        argv.push_back(w.data());
    }
    argv.push_back(nullptr);
    std::string path_env = test_dir_ + ":" + test_dir_ + "/build:"
        + (getenv("PATH") ? getenv("PATH") : "/usr/bin:/bin");

    double start_time = now();
    pid_t shell = fork();
    if (shell == 0) {
        // In the shell: enter the foreground, restore default signal
        // handling, then set up the environment and files and run `sh61`
        setpgid(0, 0);
        tcsetpgrp(tty_fd, getpid());
        for (int sig : { SIGTTOU, SIGTTIN, SIGTSTP, SIGINT, SIGQUIT, SIGPIPE,
                         SIGCHLD, SIGHUP, SIGTERM, SIGALRM, SIGXFSZ }) {
            signal(sig, SIG_DFL);
        }
        sigset_t mask;
        sigemptyset(&mask);
        sigprocmask(SIG_SETMASK, &mask, nullptr);
        // A process that writes too much to any file gets SIGXFSZ
        struct rlimit rl = { output_limit, output_limit };
        setrlimit(RLIMIT_FSIZE, &rl);
        if (chdir(run_dir_.c_str()) < 0) {
            _exit(126);
        }
        setenv("PATH", path_env.c_str(), 1);
        setenv("TTY", tty_name_.c_str(), 1);
        setenv("ASAN_OPTIONS", "detect_leaks=0", 0);
        dup2(infd, STDIN_FILENO);
        dup2(outfd, STDOUT_FILENO);
        dup2(errfd, STDERR_FILENO);
        close_extra_fds();
        execvp(argv[0], argv.data());
        fprintf(::stderr, "shtest: cannot run %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    // The shell does this too. Doing it in both processes means it's done
    // before either process continues, whichever runs first.
    setpgid(shell, shell);
    tcsetpgrp(tty_fd, shell);
    close(infd);
    close(outfd);
    close(errfd);

    while (!governor_stopping && getppid() == tester) {
        int status;
        pid_t p;
        while ((p = waitpid(-1, &status, WNOHANG)) > 0) {
            if (p == shell) {
                exit_report r = { status, now() - start_time };
                (void) !write(fd, &r, sizeof(r));
            }
        }
        // Discard anything written to the pseudo-terminal
        struct pollfd pfd = { master_fd_, POLLIN, 0 };
        if (poll(&pfd, 1, 20) > 0 && (pfd.revents & POLLIN)) {
            char buf[4096];
            (void) !read(master_fd_, buf, sizeof(buf));
        }
    }

    signal(SIGCHLD, SIG_DFL);
    signal(SIGTERM, SIG_IGN);
    kill_all();
    if (!keep_run_dir_) {
        std::error_code ec;
        fs::remove_all(run_dir_, ec);
    }
    _exit(0);
}

// list_processes(exclude)
//    Returns the processes on the pseudo-terminal, except for `exclude`.
std::vector<sh61_process_info> sh61_tester::list_processes(pid_t exclude) const {
    std::vector<sh61_process_info> result;
    if (tty_name_.empty()) {
        return result;
    }
    // The program name is `comm` on Linux and `ucomm` on macOS. The first
    // line of output is the process ID of `ps` itself.
#ifdef __APPLE__
    std::string command = "echo $$; exec ps -o pid=,stat=,ucomm= -t " + tty_name_;
#else
    std::string command = "echo $$; exec ps -o pid=,stat=,comm= -t " + tty_name_;
#endif
    FILE* f = popen(command.c_str(), "r");
    if (!f) {
        return result;
    }
    int ps_pid = -1;
    (void) !fscanf(f, "%d", &ps_pid);
    sh61_process_info p;
    char stat[32], name[256];
    while (fscanf(f, "%d %31s %255[^\n]", &p.pid, stat, name) == 3) {
        if (p.pid == ps_pid || p.pid == exclude) {
            continue;
        }
        p.name = name;
        p.name.erase(p.name.find_last_not_of(' ') + 1);
        p.name.resize(std::min(p.name.size(), size_t(15)));
        if (stat[0] == 'Z') {
            p.state = sh61_process_state::zombie;
        } else if (stat[0] == 'T' || stat[0] == 't') {
            p.state = sh61_process_state::stopped;
        } else {
            p.state = sh61_process_state::running;
        }
        result.push_back(p);
    }
    pclose(f);
    return result;
}

std::vector<sh61_process_info> sh61_tester::processes() {
    check_exit();
    return list_processes(governor_pid_);
}

// kill_all()
//    Runs in the governor. Kills every other process on the pseudo-terminal
//    and reaps exited children.
void sh61_tester::kill_all() {
    for (int round = 0; round != 200; ++round) {
        bool any = false;
        for (auto& p : list_processes(getpid())) {
            if (p.state != sh61_process_state::zombie) {
                kill(p.pid, SIGKILL);
                any = true;
            }
        }
        while (waitpid(-1, nullptr, WNOHANG) > 0) {
        }
        if (!any) {
            return;
        }
        usleep(1000);
    }
}

// stop_governor()
//    Tells the governor to clean up and waits for it to exit.
void sh61_tester::stop_governor() {
    if (governor_pid_ > 0) {
        kill(governor_pid_, SIGTERM);
        while (waitpid(governor_pid_, nullptr, 0) < 0 && errno == EINTR) {
        }
        governor_pid_ = -1;
    }
    if (report_fd_ >= 0) {
        close(report_fd_);
        report_fd_ = -1;
    }
    if (master_fd_ >= 0) {
        close(master_fd_);
        master_fd_ = -1;
    }
}

// check_exit()
//    Collects the shell's status if it has exited, and fails the test if
//    it crashed.
void sh61_tester::check_exit() {
    // collect governor status if necessary
    pid_t p;
    while ((p = waitpid(-1, nullptr, WNOHANG)) > 0) {
        if (p == governor_pid_) {
            governor_pid_ = -1;
        }
    }
    // maybe already done
    if (!running_) {
        return;
    }
    // collect exit report
    exit_report r;
    ssize_t n = read(report_fd_, &r, sizeof(r));
    if (n < 0 && (errno == EAGAIN || errno == EINTR)) {
        return;
    } else if (n != sizeof(r)) {
        fail("lost contact with governor");
    }
    running_ = false;
    status_ = r.status;
    elapsed_ = r.elapsed;
    // check for termination signal or sanitizer report
    if (WIFSIGNALED(status_)) {
        fail(strsignal(WTERMSIG(status_)));
    }
    std::string err = stderr();
    if (err.find("Sanitizer") != std::string::npos
        || err.find("runtime error:") != std::string::npos) {
        fail("sanitizer reported an error");
    }
}

void sh61_tester::set_time_limit(double seconds) {
    time_limit_ = seconds;
}

void sh61_tester::after(double seconds, std::function<void()> f) {
    if (started_) {
        fail("`after` called after `run`");
    }
    afters_.emplace_back(seconds, std::move(f));
}

bool sh61_tester::is_running() {
    check_exit();
    return running_;
}

void sh61_tester::interrupt() {
    check_exit();
    assert(running_);
    // Type Control-C on the terminal
    (void) !write(master_fd_, "\003", 1);
}


std::string sh61_tester::stdout() const {
    return file_contents("stdout");
}

std::string sh61_tester::stderr() const {
    return file_contents("stderr");
}

const int& sh61_tester::wait_status() const {
    return status_;
}

int sh61_tester::exit_status() const {
    if (running_ || !started_) {
        return -1;
    }
    return WIFEXITED(status_) ? WEXITSTATUS(status_) : -2;
}

double sh61_tester::elapsed() const {
    return running_ ? now() - start_time_ : elapsed_;
}


// Failure reporting

namespace {

const char* color(const char* code) {
    static int want = -1;
    if (want < 0) {
        const char* term = getenv("TERM");
        want = isatty(STDERR_FILENO) && term && strcmp(term, "dumb") != 0;
    }
    return want ? code : "";
}

}

// print_context()
//    Prints the script and the beginning of the shell's output to stderr.
void sh61_tester::print_context() const {
    if (!started_) {
        return;
    }
    for (const char* name : { "script", "stdout", "stderr" }) {
        // Show at most 20 lines and 2000 characters
        std::string text = file_contents(name);
        size_t end = 0;
        for (int line = 0; line != 20 && end < text.size(); ++line) {
            end = std::min(text.find('\n', end), text.size() - 1) + 1;
        }
        end = std::min(end, size_t(2000));
        if (end < text.size()) {
            text.resize(end);
            text += "...";
        }
        if (!text.empty() && text.back() != '\n') {
            text += '\n';
        }
        fprintf(::stderr, "--- %s%s\n%s", name, text.empty() ? " (empty)" : "",
                text.c_str());
    }
    if (!keep_run_dir_) {
        fprintf(::stderr, "(Run with `-d DIR` to keep the shell's files.)\n");
    }
}

void sh61_tester::fail(const char* description) {
    fprintf(::stderr, "%s%s FAILED: %s%s\n", color("\033[1;31m"), name_.c_str(),
            description, color("\033[m"));
    print_context();
    stop_governor();
    exit(1);
}
