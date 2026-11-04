// Test: Performance of byte I/O on a medium file
#include "iotest.hh"

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.text_file("in.txt", 12 << 20);
    t.run("cat61 -o out.txt in.txt");
    assert(t.file_contents("out.txt") == t.file_contents("in.txt"));
    t.report_timing();
}
