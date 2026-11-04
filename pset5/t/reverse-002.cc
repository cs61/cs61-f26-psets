// Test: Writing a file backwards one byte at a time
#include "iotest.hh"
#include <algorithm>

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    t.text_file("in.txt", 32 << 10);
    t.run("reversewrite61 -o out.txt in.txt");
    std::string expected = t.file_contents("in.txt");
    std::reverse(expected.begin(), expected.end());
    assert(t.file_contents("out.txt") == expected);
}
