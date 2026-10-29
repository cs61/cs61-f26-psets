// Test: `&&` skips a whole pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("false && echo vnexpected | tr v u ; echo expected");
    assert(t.stdout() == "expected\n");
}
