// Test: A successful `cd` has success status
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("cd / && pwd");
    assert(t.stdout() == "/\n");
}
