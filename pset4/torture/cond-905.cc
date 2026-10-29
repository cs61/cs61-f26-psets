// Test: `&&` respects the success status of a command with output
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "One\nTwo\nThree\n");
    t.run("grep -cv NotThere f.txt && echo Wanted");
    assert(t.stdout() == "3\nWanted\n");
}
