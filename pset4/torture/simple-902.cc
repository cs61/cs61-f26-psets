// Test: A simple command with several arguments
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Double Hooray");
    assert(t.stdout() == "Double Hooray\n");
}
