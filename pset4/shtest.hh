#ifndef SHTEST_HH
#define SHTEST_HH
#include <cassert>
#include <cstdio>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

// shtest.hh
//    Test harness for sh61. A test is a program that sets up a `sh61_tester`,
//    runs the shell on a command file, and then checks the results with
//    `assert`:
//
//        #include "shtest.hh"
//
//        int main(int argc, char* argv[]) {
//            sh61_tester t(argc, argv);
//            t.run("echo Hooray");
//            assert(t.stdout() == "Hooray\n");
//        }
//
//    The test implicitly asserts that the shell doesn’t crash, cause a
//    sanitizer error, run longer than its time limit, or write more than
//    100,000 bytes to any file.
//
//    The shell runs in a run directory, `out/TESTNAME-PID`, which is removed
//    when the test exits. The run directory holds the files from
//    `write_file`, plus the script (`script`) and the shell’s output
//    (`stdout`/`stderr`). To keep these files, give the test an existing
//    directory to use instead, as in `t/simple-001 -d mydir`; the test then
//    leaves that directory's files in place, both before and after the run.
//
//    The shell test is `./sh61 -q` by default; to check a different shell,
//    set `SHTEST_SHELL`, as in `SHTEST_SHELL=bash t/simple-001`. The shell
//    runs on its own terminal, whose name is in the environment variable
//    `TTY`. Scripts can run `sh61` and `timeout.sh` by name. Processes left
//    running when the test exits are killed.
//
//    You shouldn’t need to read shtest.cc.

enum class sh61_process_state { running, stopped, zombie };

struct sh61_process_info {
    pid_t pid;
    std::string name;                // program name, at most 15 characters
    sh61_process_state state;
};

struct sh61_tester {
    sh61_tester(int argc, char* argv[]);
    ~sh61_tester();
    sh61_tester(const sh61_tester&) = delete;
    sh61_tester& operator=(const sh61_tester&) = delete;


    // SETUP FUNCTIONS (before `run`)

    // Create a file in the run directory named `name` containing `contents`
    void write_file(std::string_view name, std::string_view contents);

    // Call `f()` exactly `delay` seconds after the shell starts
    void after(double delay, std::function<void()> f);

    // Stop the test by killing the shell after `limit` seconds
    void set_time_limit(double limit);


    // EXECUTION FUNCTIONS (`run` and during `run`)

    // run(script)
    //    Runs the shell on `script`. Returns once the shell has exited and all
    //    `after` functions have been called. Can be called only once.
    //    Background commands may keep running afterward.
    void run(std::string_view script);

    // Return true iff the shell is still running
    bool is_running();

    // interrupt()
    //    Sends SIGINT to the terminal's foreground process group, as if the
    //    user typed Control-C. The shell must be running.
    void interrupt();

    // Return the processes on the shell's terminal, including the shell.
    // Zombies are reaped once the shell exits, so check for zombies while
    // the shell is running (in an `after` function).
    std::vector<sh61_process_info> processes();


    // RESULTS (after `run`)

    // Return shell’s standard output so far
    std::string stdout() const;

    // Return shell’s standard error so far
    std::string stderr() const;

    // Return shell’s exit status (-1 if shell has not exited, -2 if shell
    // terminated abnormally)
    int exit_status() const;

    // Return raw shell status from `waitpid` (-1 if shell is still running)
    const int& wait_status() const;

    // Return number of seconds the shell ran, or has run so far
    double elapsed() const;

    // Return contents of file named `name` in the run directory (empty if absent)
    std::string file_contents(std::string_view name) const;


private:
    std::string name_;               // test name, such as "simple-001"
    std::string test_dir_;           // directory where the test started
    std::string run_dir_;            // such as "out/simple-001-123"
    bool keep_run_dir_ = false;      // set by `-d RUNDIR`
    double time_limit_ = 5;
    std::vector<std::pair<double, std::function<void()>>> afters_;
    bool started_ = false;           // has `run` been called?
    bool running_ = false;
    int status_ = -1;                // shell's wait status
    double start_time_ = 0;
    double elapsed_ = 0;
    std::string tty_name_;           // pseudo-terminal name, such as "pts/3"
    pid_t governor_pid_ = -1;
    int report_fd_ = -1;             // tester's end of the exit report pipe
    int master_fd_ = -1;             // pseudo-terminal master

    std::vector<std::string> shell_command() const;
    [[noreturn]] void run_governor(int fd);
    std::vector<sh61_process_info> list_processes(pid_t exclude) const;
    void kill_all();
    void stop_governor();
    void check_exit();
    void print_context() const;
    [[noreturn]] void fail(const char* description);
};

#endif
