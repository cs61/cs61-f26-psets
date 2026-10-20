// Test: The shell does not wait for a background command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    // `sleep 0.5` gives the background command time to start
    t.run("sleep 10 &\n"
          "sleep 0.5");
    assert(t.elapsed() < 2);
    // The background `sleep 10` should still be running
    int nsleep = 0;
    for (auto& p : t.processes()) {
        if (p.name == "sleep" && p.state == sh61_process_state::running) {
            ++nsleep;
        }
    }
    assert(nsleep == 1);
}
