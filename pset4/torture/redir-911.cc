// Test: A failed output redirection prints an error
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo > /tmp/directorydoesnotexist/foo");
    assert(t.stdout() == "");
    assert(t.stderr().contains("No such file or directory"));
}
