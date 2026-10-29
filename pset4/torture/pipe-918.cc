// Test: Pipelines on both sides of `&&`
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Yes | grep -q Y && echo fs | tr f Y");
    assert(t.stdout() == "Ys\n");
}
