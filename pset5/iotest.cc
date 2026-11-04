#include "iotest.hh"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

// iotest.cc
//    Implementation of `io61_tester`.
//    You shouldn’t need to read this, but do read iotest.hh.
//
//    `run` forks a child process that runs `bash -c COMMAND` in its own
//    process group, with standard input from `/dev/null` and standard output
//    and error redirected to `stdout` and `stderr` in the run directory.
//    `PATH` starts with `bin`, which links to the IO61 programs. For a
//    different library, such as `IO61=stdio`, `PATH` starts with
//    `bin/LIBRARY`, which holds programs built with that library, then `bin`.
//
//    The tester measures the command's elapsed time itself. `wait4` reports
//    the command's CPU time and maximum memory use, including all the
//    processes it ran, such as the processes in a pipeline.
//
//    `text_file` and `binary_file` generate each input file once and cache
//    it in `out/files` (`/tmp/cs61-iotest/files` in Docker). The run
//    directory gets a hard link to the cached file, which is read-only so
//    commands can't modify it.

namespace fs = std::filesystem;

namespace {

io61_tester* current_tester = nullptr;

double now() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

// Closes every file descriptor above 2
void close_extra_fds() {
    long max = sysconf(_SC_OPEN_MAX);
    if (max < 0 || max > 4096) {
        max = 4096;
    }
    for (int fd = 3; fd < max; ++fd) {
        close(fd);
    }
}

// Signal handler for SIGALRM. It does nothing, but receiving the signal
// interrupts `wait4`.
void wake(int) {
}

// Pseudorandom number generator whose output is the same on every machine
struct splitmix64 {
    uint64_t state;
    explicit splitmix64(uint64_t seed)
        : state(seed) {
    }
    uint64_t operator()() {
        uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
};

const char* const words[] = {
    "a", "about", "above", "after", "again", "against", "air", "all",
    "almost", "along", "also", "always", "among", "an", "and", "animal",
    "another", "answer", "any", "are", "around", "as", "ask", "at", "away",
    "back", "be", "because", "been", "before", "began", "begin", "being",
    "below", "between", "big", "book", "both", "boy", "but", "by", "call",
    "came", "can", "car", "carry", "change", "children", "city", "close",
    "come", "could", "country", "cut", "day", "did", "different", "do",
    "does", "don't", "down", "each", "earth", "eat", "end", "enough",
    "even", "every", "example", "eye", "face", "family", "far", "father",
    "feet", "few", "find", "first", "follow", "food", "for", "form",
    "found", "four", "from", "get", "girl", "give", "go", "good", "got",
    "great", "group", "grow", "had", "hand", "hard", "has", "have", "he",
    "head", "hear", "help", "her", "here", "high", "him", "his", "home",
    "house", "how", "idea", "if", "important", "in", "Indian", "into",
    "is", "it", "its", "it's", "just", "keep", "kind", "know", "land",
    "large", "last", "later", "learn", "leave", "left", "let", "letter",
    "life", "light", "like", "line", "list", "little", "live", "long",
    "look", "made", "make", "man", "many", "may", "me", "mean", "men",
    "might", "mile", "miss", "more", "most", "mother", "mountain", "move",
    "much", "must", "my", "name", "near", "need", "never", "new", "next",
    "night", "no", "not", "now", "number", "of", "off", "often", "old",
    "on", "once", "one", "only", "open", "or", "other", "our", "out",
    "over", "own", "page", "paper", "part", "people", "picture", "place",
    "plant", "play", "point", "put", "question", "quick", "quickly",
    "quite", "read", "really", "right", "river", "run", "said", "same",
    "saw", "say", "school", "sea", "second", "see", "seem", "sentence",
    "set", "she", "should", "show", "side", "small", "so", "some",
    "something", "sometimes", "song", "soon", "sound", "spell", "start",
    "state", "still", "stop", "story", "study", "such", "take", "talk",
    "tell", "than", "that", "the", "their", "them", "then", "there",
    "these", "they", "thing", "think", "this", "those", "thought", "three",
    "through", "time", "to", "together", "too", "took", "tree", "try",
    "turn", "two", "under", "until", "up", "us", "use", "very", "walk",
    "want", "was", "watch", "water", "way", "we", "well", "went", "were",
    "what", "when", "where", "which", "while", "white", "who", "why",
    "will", "with", "without", "word", "work", "world", "would", "write",
    "year", "you", "young", "your"
};

// Writes `size` bytes of generated data of type `type` (`t` for text, `b`
// for binary) to `out`
void generate(std::ostream& out, char type, size_t size, unsigned seed) {
    splitmix64 rng((uint64_t(seed) << 8) | uint64_t(type));
    std::string buf;
    while (size > 0) {
        buf.clear();
        if (type == 't') {
            // Lines of 1–12 words
            while (buf.size() < 65536) {
                unsigned nwords = rng() % 12 + 1;
                for (unsigned i = 0; i != nwords; ++i) {
                    if (i != 0) {
                        buf += ' ';
                    }
                    buf += words[rng() % std::size(words)];
                }
                buf += '\n';
            }
        } else {
            while (buf.size() < 65536) {
                uint64_t x = rng();
                buf.append(reinterpret_cast<const char*>(&x), sizeof(x));
            }
        }
        size_t n = std::min(buf.size(), size);
        out.write(buf.data(), n);
        size -= n;
    }
}

const char* color(FILE* f, const char* code) {
    const char* term = getenv("TERM");
    return isatty(fileno(f)) && term && strcmp(term, "dumb") != 0 ? code : "";
}

}


io61_tester::io61_tester(int argc, char* argv[]) {
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
    // In Docker, writing to the pset directory (shared with the host) is
    // very slow, so test files go in /tmp
    std::string out_dir = test_dir_ + "/out";
    if (access("/usr/bin/cs61-docker-version", F_OK) == 0) {
        out_dir = "/tmp/cs61-iotest";
    }
    files_dir_ = out_dir + "/files";
    current_tester = this;
    signal(SIGABRT, abort_handler);
    std::error_code ec;
    fs::create_directories(files_dir_, ec);
    if (ec) {
        fprintf(::stderr, "iotest: cannot create %s: %s\n", files_dir_.c_str(), ec.message().c_str());
        exit(1);
    }
    if (keep_run_dir_) {
        if (!fs::is_directory(run_dir_)) {
            fprintf(::stderr, "%s: %s: No such directory\n", name_.c_str(), run_dir_.c_str());
            exit(1);
        }
        return;
    }
    run_dir_ = out_dir + "/" + name_ + "-" + std::to_string(getpid());
    fs::remove_all(run_dir_, ec);
    fs::create_directories(run_dir_, ec);
    if (ec) {
        fprintf(::stderr, "iotest: cannot create %s: %s\n", run_dir_.c_str(), ec.message().c_str());
        exit(1);
    }
}

io61_tester::~io61_tester() {
    if (!keep_run_dir_) {
        std::error_code ec;
        fs::remove_all(run_dir_, ec);
    }
    current_tester = nullptr;
}


void io61_tester::write_file(std::string_view name, std::string_view contents) {
    std::string path = run_dir_ + "/" + std::string(name);
    unlink(path.c_str());
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(contents.data(), contents.size());
    if (!f) {
        fail("cannot write " + path);
    }
    input_size_ += contents.size();
    inputs_.emplace_back(name);
}

void io61_tester::text_file(std::string_view name, size_t size, unsigned seed) {
    make_input(name, 't', size, seed);
}

void io61_tester::binary_file(std::string_view name, size_t size, unsigned seed) {
    make_input(name, 'b', size, seed);
}

void io61_tester::make_input(std::string_view name, char type, size_t size,
                             unsigned seed) {
    if (started_) {
        fail("input file created after `run`");
    }
    std::string cached = files_dir_ + "/" + (type == 't' ? "text" : "binary")
        + "-" + std::to_string(seed) + "-" + std::to_string(size);
    struct stat st;
    if (stat(cached.c_str(), &st) != 0 || size_t(st.st_size) != size) {
        std::string tmp = cached + ".tmp" + std::to_string(getpid());
        {
            std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
            generate(f, type, size, seed);
            if (!(f << std::flush)) {
                fail("cannot write " + tmp);
            }
        }
        if (chmod(tmp.c_str(), 0444) != 0
            || rename(tmp.c_str(), cached.c_str()) != 0) {
            fail("cannot create " + cached);
        }
    }

    // Link to the cached file, or copy it if the user can write read-only
    // files
    std::string path = run_dir_ + "/" + std::string(name);
    unlink(path.c_str());
    if (geteuid() == 0 || link(cached.c_str(), path.c_str()) != 0) {
        std::error_code ec;
        fs::copy_file(cached, path, ec);
        if (ec) {
            fail("cannot create " + path);
        }
    }
    input_size_ += size;
    inputs_.emplace_back(name);
}

void io61_tester::set_time_limit(double limit) {
    time_limit_ = limit;
}

void io61_tester::set_output_limit(size_t limit) {
    output_limit_ = limit;
}


void io61_tester::run(std::string_view command) {
    if (started_) {
        fail("`run` called twice");
    }
    started_ = true;
    command_ = command;
    const char* s = getenv("STRACE");
    strace_ = s && *s && strcmp(s, "0") != 0;
    const char* library = getenv("IO61");
    timing_ = execute(library ? library : "");
}

// collect_traces()
//    Moves `strace` output from the run directory to the test directory,
//    replacing any older output there.
void io61_tester::collect_traces() {
    // `strace.out` or `strace.PID.out`
    auto is_trace = [](const std::string& name) {
        if (name == "strace.out") {
            return true;
        }
        return name.size() > 11 && name.starts_with("strace.")
            && name.ends_with(".out")
            && std::all_of(name.begin() + 7, name.end() - 4, isdigit);
    };
    std::error_code ec;
    for (auto& e : fs::directory_iterator(test_dir_, ec)) {
        if (is_trace(e.path().filename().string())) {
            fs::remove(e.path(), ec);
        }
    }
    std::string names;
    for (auto& e : fs::directory_iterator(run_dir_, ec)) {
        std::string name = e.path().filename().string();
        if (is_trace(name)) {
            std::string dest = test_dir_ + "/" + name;
            fs::rename(e.path(), dest, ec);
            if (ec) {
                fs::copy_file(e.path(), dest, fs::copy_options::overwrite_existing, ec);
            }
            names += (names.empty() ? "" : ", ") + name;
        }
    }
    fprintf(::stderr, "%s: strace output in %s\n", name_.c_str(),
            names.empty() ? "(none)" : names.c_str());
}

// library_dir(library)
//    Returns the directory holding the IO61 programs that use `library`
//    (empty for the main versions).
std::string io61_tester::library_dir(const std::string& library) const {
    if (library.empty()) {
        return "";
    }
    std::string dir = test_dir_ + "/bin/" + library;
    if (library.find('/') != std::string::npos
        || !fs::is_directory(dir)) {
        fprintf(::stderr, "%s: IO61=%s: No such library (no `bin/%s` directory)\n",
                name_.c_str(), library.c_str(), library.c_str());
        exit(1);
    }
    return dir;
}

// execute(library)
//    Runs the command using `library` and returns its timing. If `strace_`
//    is true, the programs run under `strace`, and their output moves to the
//    test directory. Fails the test if the command fails.
io61_tester::timing io61_tester::execute(const std::string& library) {
    std::string libdir = library_dir(library);
    std::string path_env = (libdir.empty() ? "" : libdir + ":") + test_dir_
        + "/bin:" + (getenv("PATH") ? getenv("PATH") : "/usr/bin:/bin");
    size_t output_limit = output_limit_;
    if (output_limit == 0) {
        output_limit = std::max(size_t(16) << 20, 4 * input_size_);
    }

    // Remove input files from the buffer cache so every run starts cold
#ifdef POSIX_FADV_DONTNEED
    for (auto& name : inputs_) {
        int fd = open((run_dir_ + "/" + name).c_str(), O_RDONLY | O_CLOEXEC);
        if (fd >= 0) {
            posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
            close(fd);
        }
    }
#endif

    int outfd = open((run_dir_ + "/stdout").c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    int errfd = open((run_dir_ + "/stderr").c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    int infd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    if (outfd < 0 || errfd < 0 || infd < 0) {
        fail("cannot open files in " + run_dir_);
    }

    fflush(::stdout);
    fflush(::stderr);
    double start_time = now();
    pid_t child = fork();
    if (child < 0) {
        fail("cannot fork");
    } else if (child == 0) {
        setpgid(0, 0);
        for (int sig : { SIGINT, SIGQUIT, SIGPIPE, SIGCHLD, SIGTERM,
                         SIGALRM, SIGXFSZ, SIGABRT }) {
            signal(sig, SIG_DFL);
        }
        // A process that writes too much to any file gets SIGXFSZ
        struct rlimit rl = { rlim_t(output_limit), rlim_t(output_limit) };
        setrlimit(RLIMIT_FSIZE, &rl);
        if (chdir(run_dir_.c_str()) < 0) {
            _exit(126);
        }
        setenv("PATH", path_env.c_str(), 1);
        if (!strace_) {
            unsetenv("STRACE");
        }
        dup2(infd, STDIN_FILENO);
        dup2(outfd, STDOUT_FILENO);
        dup2(errfd, STDERR_FILENO);
        close_extra_fds();
        execlp("bash", "bash", "-o", "pipefail", "-c", command_.c_str(), nullptr);
        fprintf(::stderr, "iotest: cannot run bash: %s\n", strerror(errno));
        _exit(127);
    }
    setpgid(child, child);
    close(outfd);
    close(errfd);
    close(infd);

    // Wait for the command to exit. SIGALRM, which arrives every 10ms
    // after the time limit, interrupts `wait4` so the tester can kill the
    // command.
    struct sigaction sa = {};
    sa.sa_handler = wake;
    sigaction(SIGALRM, &sa, nullptr);
    struct itimerval timer = {};
    timer.it_value.tv_sec = time_t(time_limit_);
    timer.it_value.tv_usec = suseconds_t((time_limit_ - time_t(time_limit_)) * 1e6) + 1;
    timer.it_interval.tv_usec = 10000;
    setitimer(ITIMER_REAL, &timer, nullptr);
    int status;
    struct rusage ru;
    while (wait4(child, &status, 0, &ru) < 0) {
        if (errno == EINTR && now() - start_time >= time_limit_) {
            kill(-child, SIGKILL);
            if (strace_) {
                collect_traces();
            }
            fail("command did not exit within the time limit");
        } else if (errno != EINTR) {
            fail("lost track of command");
        }
    }
    double end_time = now();
    struct itimerval no_timer = {};
    setitimer(ITIMER_REAL, &no_timer, nullptr);
    if (strace_) {
        collect_traces();
    }
    timing t;
    t.time = end_time - start_time;
    t.utime = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec * 1e-6;
    t.stime = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec * 1e-6;
#ifdef __APPLE__
    t.maxrss = ru.ru_maxrss / 1024;  // macOS reports bytes, not KiB
#else
    t.maxrss = ru.ru_maxrss;
#endif
    // Kill any processes the command left behind
    kill(-child, SIGKILL);

    // Check for failure
    std::string with = library.empty() ? "" : " (with IO61=" + library + ")";
    int exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : 0;
    // `bash` reports a signal that killed a command as status 128 + signal
    int sig = 0;
    if (WIFSIGNALED(status)) {
        sig = WTERMSIG(status);
    } else if (exit_status > 128 && exit_status < 128 + NSIG) {
        sig = exit_status - 128;
    }
    if (sig == SIGXFSZ) {
        fail("command wrote more than " + std::to_string(output_limit)
             + " bytes to a file" + with);
    } else if (sig != 0) {
        fail("command terminated by signal: " + std::string(strsignal(sig)) + with);
    } else if (exit_status != 0) {
        fail("command exited with status " + std::to_string(exit_status) + with);
    }
    std::string err = stderr();
    if (err.find("Sanitizer") != std::string::npos
        || err.find("runtime error:") != std::string::npos) {
        fail("sanitizer reported an error" + with);
    }

    return t;
}


void io61_tester::report_timing() {
    if (!started_) {
        fail("`report_timing` called before `run`");
    }
    int trials = 1;
    if (const char* s = getenv("TRIALS"); s && atoi(s) > 0) {
        trials = atoi(s);
    }
    // Time new runs, since the first run's output files didn't exist yet.
    // Only the first run is traced.
    strace_ = false;
    const char* library = getenv("IO61");
    std::vector<timing> yours, stdio;
    for (int i = 0; i != trials; ++i) {
        stdio.push_back(execute("stdio"));
        yours.push_back(execute(library ? library : ""));
    }
    auto median = [](std::vector<timing>& v) {
        std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.time < b.time; });
        return v[v.size() / 2];
    };
    timing y = median(yours), s = median(stdio);
    double ratio = s.time / std::max(y.time, 1e-6);
    // Red: much slower than stdio; blue: about the same; green: much faster
    const char* ratio_color = ratio < 0.75 ? "\033[0;31m"
        : (ratio < 1.9 ? "\033[1;36m" : "\033[1;32m");
    printf("%s: %.4fs (%.0f MiB), stdio %.4fs (%.0f MiB): %s%.2fx stdio%s\n",
           name_.c_str(), y.time, y.maxrss / 1024.0, s.time, s.maxrss / 1024.0,
           color(::stdout, ratio_color), ratio, color(::stdout, "\033[m"));
    fflush(::stdout);
}


std::string io61_tester::stdout() const {
    return file_contents("stdout");
}

std::string io61_tester::stderr() const {
    return file_contents("stderr");
}

double io61_tester::elapsed() const {
    return timing_.time;
}

std::string io61_tester::file_contents(std::string_view name) const {
    std::ifstream f(run_dir_ + "/" + std::string(name), std::ios::binary);
    std::ostringstream buf;
    buf << f.rdbuf();
    return buf.str();
}


// Failure reporting

// print_context()
//    Prints the command and the beginning of its output to stderr.
void io61_tester::print_context() const {
    if (!started_) {
        return;
    }
    fprintf(::stderr, "--- command\n%s\n", command_.c_str());
    for (const char* name : { "stdout", "stderr" }) {
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
        // Replace control characters, since output may be binary
        for (auto& ch : text) {
            if ((unsigned char) ch < 32 && ch != '\n' && ch != '\t') {
                ch = '?';
            }
        }
        fprintf(::stderr, "--- %s%s\n%s", name, text.empty() ? " (empty)" : "",
                text.c_str());
    }
    if (!keep_run_dir_) {
        fprintf(::stderr, "(Run with `-d DIR` to keep the command's files.)\n");
    }
}

void io61_tester::fail(const std::string& description) {
    fprintf(::stderr, "%s%s FAILED: %s%s\n", color(::stderr, "\033[1;31m"),
            name_.c_str(), description.c_str(), color(::stderr, "\033[m"));
    print_context();
    if (!keep_run_dir_) {
        std::error_code ec;
        fs::remove_all(run_dir_, ec);
    }
    exit(1);
}

// abort_handler(signo)
//    Called when an `assert` fails. Prints context and cleans up.
void io61_tester::abort_handler(int) {
    signal(SIGABRT, SIG_DFL);
    if (io61_tester* t = current_tester) {
        current_tester = nullptr;
        t->print_context();
        if (!t->keep_run_dir_) {
            std::error_code ec;
            fs::remove_all(t->run_dir_, ec);
        }
    }
    raise(SIGABRT);
}
