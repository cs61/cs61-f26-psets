// Test: Input redirection overrides a pipe
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("unwanted.txt", "Unwanted\n");
    t.write_file("wanted.txt", "Wanted\n");
    t.run("cat unwanted.txt | cat < wanted.txt");
    assert(t.stdout() == "Wanted\n");
}
