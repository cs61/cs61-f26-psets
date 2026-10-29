// Test: Pipelines in a semicolon list
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo Sedi | tr d m ; echo Calan | tr a o");
    assert(t.stdout() == "Semi\nColon\n");
}
