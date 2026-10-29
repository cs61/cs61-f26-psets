// Test: A pipeline runs in the background
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    // `wc -c` prints 0, possibly padded with spaces, and `sed` replaces it
    t.after(0.6, [&] {
        assert(t.stdout() == "First\nSecond\n");
    });
    t.run("sleep 0.4 | wc -c | sed \"s/.*0/Second/\" & "
          "sleep 0.2 | wc -c | sed \"s/.*0/First/\"");
}
