// Test: Input and output redirections in a pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("wanted.txt", "Wanted\n");
    t.run("cat < wanted.txt | cat > output.txt");
    assert(t.stdout() == "");
    assert(t.file_contents("output.txt") == "Wanted\n");
}
