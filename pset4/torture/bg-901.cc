// Test: A background command runs while the shell continues
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "Original\n");
    t.run("sh -c \"sleep 0.2; echo Replaced > f.txt\" &");
    // The shell has exited, but the background command is still sleeping
    assert(t.file_contents("f.txt") == "Original\n");
    usleep(400000);
    assert(t.file_contents("f.txt") == "Replaced\n");
}
