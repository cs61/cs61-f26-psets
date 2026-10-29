// Test: A command with a failed input redirection fails
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Hello < nonexistent.txt || echo Wanted");
    assert(t.stdout() == "Wanted\n");
    assert(t.stderr().contains("No such file or directory"));
}
