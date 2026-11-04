// Test: Copying a file in blocks of random sizes
#include "iotest.hh"

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.text_file("in.txt", 32 << 10);
    // Each block is between 1 and 256 bytes long
    t.run("varblockcat61 -b 256 -o out.txt in.txt");
    assert(t.file_contents("out.txt") == t.file_contents("in.txt"));
}
