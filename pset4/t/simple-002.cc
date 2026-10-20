// Test: The shell handles quoted arguments
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "Original\n");
    t.run("echo \">\" _ \";\" _ \"<\"");
    assert(t.stdout() == "> _ ; _ <\n");
}
