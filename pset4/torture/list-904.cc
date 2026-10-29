// Test: Semicolons separate many commands, with extra spaces
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Hello ;   echo There ; echo Who ; echo Are ; echo You ; echo ?");
    assert(t.stdout() == "Hello\nThere\nWho\nAre\nYou\n?\n");
}
