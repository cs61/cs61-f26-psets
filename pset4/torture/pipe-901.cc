// Test: A two-command pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Pipe | wc -c");
    // `wc` pads its output with spaces on some systems
    assert(t.stdout() == "5\n" || t.stdout() == "       5\n");
}
