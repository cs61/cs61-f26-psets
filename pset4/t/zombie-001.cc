// Test: The shell reaps a background command that has exited
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    // At 1.5 sec, the shell has finished `sleep 0.5` and is running `sleep 2`
    t.after(1.5, [&] {
        for (auto& p : t.processes()) {
            assert(p.state != sh61_process_state::zombie);
        }
    });
    t.run("sleep 0.05 &\n"
          "sleep 0.5\n"
          "sleep 2");
}
