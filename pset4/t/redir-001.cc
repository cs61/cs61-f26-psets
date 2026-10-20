// Test: Output redirection sends a command's output to a file
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Start ; echo File > out.txt");
    assert(t.stdout() == "Start\n");
    assert(t.file_contents("out.txt") == "File\n");
}
