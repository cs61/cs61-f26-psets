// Test: A command with a failed output redirection fails
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo notshown > /tmp/directorydoesnotexist/foo || echo Wanted");
    assert(t.stdout() == "Wanted\n");
    assert(t.stderr().contains("No such file or directory"));
}
