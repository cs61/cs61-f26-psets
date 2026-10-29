// Test: Multiple `cd` commands in a conditional
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("cd /tmp && cd / && pwd");
    assert(t.stdout() == "/\n");
}
