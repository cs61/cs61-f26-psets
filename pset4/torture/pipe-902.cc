// Test: A pipeline whose second command ignores its input
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Ignored | echo Desired");
    assert(t.stdout() == "Desired\n");
}
