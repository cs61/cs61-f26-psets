// Test: Quoted `&&` and `||` are ordinary arguments
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo \"&&\" hello && echo \"||\" hello");
    assert(t.stdout() == "&& hello\n|| hello\n");
}
