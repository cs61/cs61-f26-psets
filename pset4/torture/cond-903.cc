// Test: `||` respects success status
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo True || echo False");
    assert(t.stdout() == "True\n");
}
