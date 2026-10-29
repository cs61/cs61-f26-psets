// Test: A background conditional chain runs alongside foreground commands
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo first && sleep 0.2 && echo third & "
          "sleep 0.1 ; echo second ; sleep 0.2 ; echo fourth");
    assert(t.stdout() == "first\nsecond\nthird\nfourth\n");
}
