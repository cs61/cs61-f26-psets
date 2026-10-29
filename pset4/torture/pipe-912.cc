// Test: A successful pipeline has success status
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("true | true && echo True");
    assert(t.stdout() == "True\n");
}
