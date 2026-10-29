// Test: A failed `cd` has failure status for `||`
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("cd /doesnotexist 2> /dev/null > /dev/null || echo does not exist");
    assert(t.stdout() == "does not exist\n");
}
