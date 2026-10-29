// Test: A redirection can appear between the arguments
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo the redirection > out.txt can really occur anywhere && cat out.txt");
    assert(t.stdout() == "the redirection can really occur anywhere\n");
}
