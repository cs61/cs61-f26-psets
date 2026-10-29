// Test: A redirection can appear before the arguments
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo > out.txt the redirection can occur anywhere && cat out.txt");
    assert(t.stdout() == "the redirection can occur anywhere\n");
}
