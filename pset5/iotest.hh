#ifndef IOTEST_HH
#define IOTEST_HH
#include <cassert>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>
#include <unistd.h>

// iotest.hh
//    Test harness for IO61. A test is a program that sets up an
//    `io61_tester`, creates input files, runs a shell command that uses IO61
//    programs, and then checks the results with `assert`:
//
//        #include "iotest.hh"
//
//        int main(int argc, char* argv[]) {
//            io61_tester t(argc, argv);
//            t.text_file("in.txt", 32 << 10);
//            t.run("cat61 -o out.txt in.txt");
//            assert(t.file_contents("out.txt") == t.file_contents("in.txt"));
//        }
//
//    The command runs in `bash` with the `pipefail` option, so it can use
//    pipes, redirections, and other shell features. It can run the IO61
//    programs, such as `cat61`, `blockcat61`, and `reverse61`, by name.
//
//    The test implicitly asserts that the command exits with status 0 and
//    doesn't crash, cause a sanitizer error, run longer than its time limit,
//    or write too much data to any file.
//
//    The command runs in a run directory, `out/TESTNAME-PID`, which is
//    removed when the test exits. (In Docker, the run directory is
//    `/tmp/cs61-iotest/TESTNAME-PID`, since writing files to the pset
//    directory is very slow there.) To keep the run directory's files, give
//    the test an existing directory to use instead, as in `t/cat-001 -d
//    mydir`.
//
//    To run a test with a different IO61 library, set `IO61`. For instance,
//    `IO61=stdio t/cat-001` runs the test with programs that use our stdio
//    version of IO61. Every test should pass with `IO61=stdio`.
//
//    To see the system calls the command makes (on Linux), set `STRACE`, as
//    in `STRACE=1 t/cat-001`. The IO61 programs in the command then run
//    under `strace`, which writes its output to `strace.out` in the current
//    directory. With `STRACE=2`, each program writes to its own file,
//    `strace.PID.out`. Only the first run is traced, not the reruns in
//    `report_timing`.
//
//    You shouldn’t need to read iotest.cc.

struct io61_tester {
    io61_tester(int argc, char* argv[]);
    ~io61_tester();
    io61_tester(const io61_tester&) = delete;
    io61_tester& operator=(const io61_tester&) = delete;


    // SETUP FUNCTIONS (before `run`)

    // Create a file in the run directory named `name` containing `contents`
    void write_file(std::string_view name, std::string_view contents);

    // Create a file in the run directory named `name` containing `size`
    // bytes of text: lines of words separated by spaces. Files with the same
    // `size` and `seed` have the same contents, and a smaller file is a
    // prefix of a larger file with the same `seed`.
    void text_file(std::string_view name, size_t size, unsigned seed = 0);

    // Create a file in the run directory named `name` containing `size`
    // bytes of random binary data, determined by `size` and `seed`
    void binary_file(std::string_view name, size_t size, unsigned seed = 0);

    // Stop the test by killing the command after `limit` seconds
    // (default 10 seconds)
    void set_time_limit(double limit);

    // Stop the test if the command writes more than `limit` bytes to any one
    // file (default 16 MiB or 4 times the size of the run directory's
    // files, whichever is larger)
    void set_output_limit(size_t limit);


    // EXECUTION

    // run(command)
    //    Runs `command` in the run directory and waits for it to exit. Can
    //    be called only once.
    void run(std::string_view command);


    // RESULTS (after `run`)

    // Return command’s standard output
    std::string stdout() const;

    // Return command’s standard error
    std::string stderr() const;

    // Return number of seconds the command took
    double elapsed() const;

    // Return contents of file named `name` in the run directory (empty if absent)
    std::string file_contents(std::string_view name) const;


    // PERFORMANCE (after checking results)

    // report_timing()
    //    Runs `command` again using the stdio version of IO61, then again
    //    using your version, and prints how long each version took.
    //    Performance tests call this after checking their results, since
    //    rerunning the command overwrites its output files. To time more runs
    //    of each version and report the median, set `TRIALS`, as in
    //    `TRIALS=5 t/perf-001`.
    void report_timing();


private:
    struct timing {
        double time = 0;             // seconds
        double utime = 0;            // user-mode seconds
        double stime = 0;            // system-mode seconds
        long maxrss = 0;             // maximum memory use, in KiB
    };

    std::string name_;               // test name, such as "cat-001"
    std::string test_dir_;           // directory where the test started
    std::string run_dir_;            // such as "out/cat-001-123"
    std::string files_dir_;          // cache for `text_file`/`binary_file`
    bool keep_run_dir_ = false;      // set by `-d RUNDIR`
    double time_limit_ = 10;
    size_t output_limit_ = 0;        // 0 means use the default
    size_t input_size_ = 0;          // total size of files from setup
    std::vector<std::string> inputs_;    // names of files from setup
    std::string command_;
    bool started_ = false;           // has `run` been called?
    timing timing_;                  // timing for `run`
    bool strace_ = false;            // trace programs with `strace`?

    void make_input(std::string_view name, char type, size_t size, unsigned seed);
    timing execute(const std::string& library);
    std::string library_dir(const std::string& library) const;
    void collect_traces();
    void print_context() const;
    [[noreturn]] void fail(const std::string& description);
    static void abort_handler(int);
};

#endif
