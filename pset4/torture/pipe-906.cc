// Test: A three-command pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Line | cat | wc -l");
    // `wc` pads its output with spaces on some systems
    assert(t.stdout() == "1\n" || t.stdout() == "       1\n");
}
