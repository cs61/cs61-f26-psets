// Test: A pipeline passes output to the next command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Good | grep -n G");
    assert(t.stdout() == "1:Good\n");
}
