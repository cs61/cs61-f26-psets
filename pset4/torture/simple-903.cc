// Test: A simple command reads a file
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f3.txt", "Triple Hooray\n");
    t.run("cat f3.txt");
    assert(t.stdout() == "Triple Hooray\n");
}
