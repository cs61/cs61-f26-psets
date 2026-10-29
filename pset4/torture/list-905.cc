// Test: Every command in a semicolon list runs
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "no\n");
    t.run("rm -f f.txt ; echo Removed");
    assert(t.stdout() == "Removed\n");
    assert(t.file_contents("f.txt") == "");
}
