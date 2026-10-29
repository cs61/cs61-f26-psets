// Test: Multiple `cd` commands
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("cd / ; cd /usr ; pwd");
    assert(t.stdout() == "/usr\n");
}
