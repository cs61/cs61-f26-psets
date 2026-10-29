// Test: A simple command runs and prints its output
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Hooray");
    assert(t.stdout() == "Hooray\n");
}
