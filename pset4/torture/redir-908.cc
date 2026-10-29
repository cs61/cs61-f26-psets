// Test: Standard error redirection
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("sh -c \"echo Output; echo Error 1>&2\" 2> f.txt");
    assert(t.stdout() == "Output\n");
    assert(t.stderr() == "");
    assert(t.file_contents("f.txt") == "Error\n");
}
