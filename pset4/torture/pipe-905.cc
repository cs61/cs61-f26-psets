// Test: Commands in a pipeline run in parallel
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    // `yes` prints forever, so the shell must not wait for it to exit
    // before starting `head`
    t.set_time_limit(2);
    t.run("yes | head -n 5");
    assert(t.stdout() == "y\ny\ny\ny\ny\n");
}
