// Test: The shell does not wait for a background command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    // At 0.2 sec, `sleep 2` is still running
    t.after(0.2, [&] {
        int nsleep = 0;
        for (auto& p : t.processes()) {
            if (p.name == "sleep" && p.state == sh61_process_state::running) {
                ++nsleep;
            }
        }
        assert(nsleep == 1);
    });
    t.run("sleep 2 &");
    assert(t.elapsed() < 1);
}
