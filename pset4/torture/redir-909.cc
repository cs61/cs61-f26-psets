// Test: Standard output and standard error redirections
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("sh -c \"echo Output; echo Error 1>&2\" > fa.txt 2> fb.txt");
    assert(t.stdout() == "");
    assert(t.stderr() == "");
    assert(t.file_contents("fa.txt") == "Output\n");
    assert(t.file_contents("fb.txt") == "Error\n");
}
