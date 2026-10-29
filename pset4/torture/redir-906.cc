// Test: Input redirection overrides a pipe
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("lower.txt", "LOWER\n");
    t.run("echo Ignored | cat < lower.txt | tr A-Z a-z");
    assert(t.stdout() == "lower\n");
}
