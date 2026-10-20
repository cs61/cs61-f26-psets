// Test: A failed redirection prints an error and the command fails
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Unwanted < nonexistent.txt || echo Wanted");
    assert(t.stdout() == "Wanted\n");
    assert(t.stderr().contains("No such file or directory"));
}
