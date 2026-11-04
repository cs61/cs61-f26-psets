// Test: Block I/O with an odd block size copies a file through pipes
#include "iotest.hh"

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.text_file("in.txt", 32 << 10);
    t.run("cat in.txt | blockcat61 -b 1021 | cat > out.txt");
    assert(t.file_contents("out.txt") == t.file_contents("in.txt"));
}
