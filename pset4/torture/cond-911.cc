// Test: Long `&&` chains skip all their commands after a failure
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("false && echo no && echo no && echo no && echo no || echo yes");
    assert(t.stdout() == "yes\n");
}
