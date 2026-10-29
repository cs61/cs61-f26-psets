// Test: A pipeline has the status of its last command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Hello | grep -q X || echo NoXs");
    assert(t.stdout() == "NoXs\n");
}
