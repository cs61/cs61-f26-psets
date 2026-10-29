// Test: A pipeline has the status of its last command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Yes | grep -q Y && echo Ys");
    assert(t.stdout() == "Ys\n");
}
