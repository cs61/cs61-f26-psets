// Test: A command with a failed output redirection fails
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo notshown > /tmp/directorydoesnotexist/foo && echo Unwanted");
    assert(t.stdout() == "");
    assert(t.stderr().contains("No such file or directory"));
}
