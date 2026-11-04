// Test: Reading a file backwards one byte at a time
#include "iotest.hh"
#include <algorithm>

int main(int argc, char* argv[]) {
    io61_tester t(argc, argv);
    // Create a 32 KiB input file containing lines of text
    t.text_file("in.txt", 32 << 10);
    t.run("reverse61 -o out.txt in.txt");
    // The output should be the input, reversed
    std::string expected = t.file_contents("in.txt");
    std::reverse(expected.begin(), expected.end());
    assert(t.file_contents("out.txt") == expected);
}
