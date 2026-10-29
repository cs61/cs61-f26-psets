// Test: `cd` changes the shell's directory
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("cd / ; pwd");
    assert(t.stdout() == "/\n");
}
