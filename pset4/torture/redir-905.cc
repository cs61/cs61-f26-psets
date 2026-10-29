// Test: Input and output redirections in a longer pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("xoqted.txt", "xoqted\n");
    t.run("cat < xoqted.txt | tr xoq Wan | cat > output.txt");
    assert(t.stdout() == "");
    assert(t.file_contents("output.txt") == "Wanted\n");
}
