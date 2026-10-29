// Test: A conditional chain runs in the background
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.after(0.6, [&] {
        assert(t.stdout() == "First\nSecond\n");
    });
    t.run("sleep 0.4 && echo Second & sleep 0.2 && echo First");
}
