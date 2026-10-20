// Test: The shell waits for a foreground command to finish
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "Original\n");
    t.run("sh -c \"sleep 0.1; echo Replaced > f.txt\"");
    assert(t.file_contents("f.txt") == "Replaced\n");
}
