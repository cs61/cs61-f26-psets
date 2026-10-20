// Test: Commands separated by `;` run in order
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Semi ; echo Colon ; echo Rocks");
    assert(t.stdout() == "Semi\nColon\nRocks\n");
}
