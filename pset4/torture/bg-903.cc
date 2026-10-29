// Test: The shell exits without waiting for a background command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "Still here\n");
    t.run("sh -c \"sleep 1; rm -f f.txt\" &");
    // The shell has exited, but the background command is still sleeping
    assert(t.file_contents("f.txt") == "Still here\n");
}
