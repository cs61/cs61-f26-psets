// Test: Performance of byte I/O on a medium file, read in reverse
#include "iotest.hh"
#include <algorithm>

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.text_file("in.txt", 12 << 20);
    t.run("reverse61 -o out.txt in.txt");
    std::string expected = t.file_contents("in.txt");
    std::reverse(expected.begin(), expected.end());
    assert(t.file_contents("out.txt") == expected);
    t.report_timing();
}
