// Test: Output redirection
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Start ; echo File > f.txt");
    assert(t.stdout() == "Start\n");
    assert(t.file_contents("f.txt") == "File\n");
}
