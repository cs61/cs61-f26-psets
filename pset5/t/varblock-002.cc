// Test: Mixing `io61_readc` and `io61_read` calls
#include "iotest.hh"

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.text_file("in.txt", 32 << 10);
    // Each block is 1, 2, 4, ..., or 256 bytes long; one-byte blocks use
    // `io61_readc` and `io61_writec`
    t.run("varblockcat61 -b 256 -X -o out.txt in.txt");
    assert(t.file_contents("out.txt") == t.file_contents("in.txt"));
}
