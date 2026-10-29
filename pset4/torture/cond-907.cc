// Test: Conditional chains without output
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("true && false || true && echo Good");
    assert(t.stdout() == "Good\n");
}
