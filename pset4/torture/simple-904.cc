// Test: Commands on separate lines run in order
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Line 1\n"
          "echo Line 2\n"
          "echo Line 3");
    assert(t.stdout() == "Line 1\nLine 2\nLine 3\n");
}
