// Test: Conditional chains with output
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Start && false || false && false || echo End");
    assert(t.stdout() == "Start\nEnd\n");
}
