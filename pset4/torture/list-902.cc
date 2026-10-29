// Test: A semicolon separates two commands
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Semi ; echo Colon");
    assert(t.stdout() == "Semi\nColon\n");
}
