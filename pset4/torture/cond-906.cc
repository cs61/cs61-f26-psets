// Test: `&&` respects the failure status of a command with output
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "One\nTwo\nThree\n");
    t.run("grep -c NotThere f.txt && echo Unwanted");
    assert(t.stdout() == "0\n");
}
