// Test: The shell does not wait for a second background command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("sleep 4 & sleep 4 & echo OK");
    assert(t.stdout() == "OK\n");
    assert(t.elapsed() < 3);
}
