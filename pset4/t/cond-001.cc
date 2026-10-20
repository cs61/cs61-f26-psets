// Test: `&&` runs its second command only if the first succeeds
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("true && echo True\n"
          "false && echo False");
    assert(t.stdout() == "True\n");
}
