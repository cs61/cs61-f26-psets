// Test: A failed `cd` with redirections leaves the directory unchanged
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("cd / ; cd /doesnotexist 2> /dev/null > /dev/null ; pwd");
    assert(t.stdout() == "/\n");
}
