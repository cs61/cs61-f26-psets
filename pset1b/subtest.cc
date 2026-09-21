#include "utils.hh"
#include <sys/wait.h>
#include <format>
#include <iostream>

// subtest.cc
//    Implementation of `run_subtest` and `subtest_result` (see utils.hh).

subtest_result run_subtest(std::function<void()> f) {
    int pipefd[2];
    int r = pipe(pipefd);
    assert(r == 0);
    fflush(stdout);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        // This code runs only in the subtest.
        // Redirect stdout and stderr back to the parent
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        // Call `f`
        f();
        // Flush its output and exit
        fflush(stdout);
        _exit(0);
    }
    // This code runs only in the parent.
    // Read the subtest’s output until it exits
    close(pipefd[1]);
    subtest_result result;
    char buf[4096];
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        result.output.append(buf, n);
    }
    close(pipefd[0]);
    r = waitpid(pid, &result.status, 0);
    assert(r == pid);
    return result;
}

bool subtest_result::exited_ok() const {
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool subtest_result::sanitizer_error() const {
    return contains("Sanitizer") || contains("runtime error:");
}

bool subtest_result::aborted() const {
    return (WIFSIGNALED(status)
            && WTERMSIG(status) == SIGABRT
            && !sanitizer_error())
        || (WIFEXITED(status)
            && WEXITSTATUS(status) == 1);
}

bool subtest_result::contains(const std::string& text) const {
    return output.find(text) != std::string::npos;
}

void subtest_result::print(const std::string& what) const {
    std::print(stderr, "subtest \"{}\" ", what);
    if (WIFEXITED(status)) {
        std::print(stderr, "exited with status {}", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT) {
        std::print(stderr, "aborted");
    } else if (WIFSIGNALED(status)) {
        std::print(stderr, "was killed by signal {}", WTERMSIG(status));
    }
    if (sanitizer_error()) {
        std::print(stderr, " {}after a sanitizer error{}", termcolor::red, termcolor::plain);
    }
    if (output.empty()) {
        std::print(stderr, "\n");
        return;
    }
    std::print(stderr, " with this output:\n");

    // print the output, each line marked with "  | "
    std::string_view str(output);
    size_t pos = 0;
    while (pos < str.size()) {
        size_t nl = str.find('\n', pos);
        if (nl == std::string::npos) {
            nl = str.size();
        }
        std::print(stderr, "{}  | {}{}\n", termcolor::blue,
            str.substr(pos, nl - pos), termcolor::plain);
        pos = nl + 1;
    }
}


// Terminal color globals

int termcolor::wantcolor;

termcolor termcolor::plain, termcolor::red(31), termcolor::green(32),
    termcolor::blue(34), termcolor::orange(220, 100, 0);
