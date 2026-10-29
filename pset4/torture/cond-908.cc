// Test: Conditional chains with early output
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Start && false || false && echo Bad");
    assert(t.stdout() == "Start\n");
}
