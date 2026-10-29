// Test: A failed `cd` has failure status for `&&`
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo go ; cd /doesnotexist 2> /dev/null > /dev/null && pwd");
    assert(t.stdout() == "go\n");
}
