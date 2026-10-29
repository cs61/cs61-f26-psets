// Test: Pipelines on both sides of `||`
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Hello | grep -q X || echo poqs | tr pq NX");
    assert(t.stdout() == "NoXs\n");
}
