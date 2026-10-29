// Test: A command killed by a signal has failure status for `&&`
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("timeout.sh 0.02 sleep 10 && echo no");
    assert(t.stdout() == "");
}
