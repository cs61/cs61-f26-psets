// Test: Byte I/O copies a small binary file
#include "iotest.hh"

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.binary_file("in.bin", 32 << 10);
    t.run("cat61 -o out.bin in.bin");
    assert(t.file_contents("out.bin") == t.file_contents("in.bin"));
}
