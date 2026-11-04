// Test: Byte I/O copies a small text file
#include "iotest.hh"

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.text_file("in.txt", 32 << 10);
    t.run("cat61 -o out.txt in.txt");
    assert(t.file_contents("out.txt") == t.file_contents("in.txt"));
}
