// Test: The shell waits for a foreground command to exit
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f5.txt", "no\n");
    t.run("sh -c \"sleep 0.1; echo > f5.txt\"");
    assert(t.file_contents("f5.txt") == "\n");
}
