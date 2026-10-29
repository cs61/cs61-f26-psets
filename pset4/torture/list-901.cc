// Test: A list can end with a semicolon
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Semi ;");
    assert(t.stdout() == "Semi\n");
}
