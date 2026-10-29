// Test: The shell reaps several background commands that have exited
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    // At 0.4 sec, the shell is running `sleep 0.5`
    t.after(0.4, [&] {
        for (auto& p : t.processes()) {
            assert(p.state != sh61_process_state::zombie);
        }
    });
    t.run("sleep 0.05 & sleep 0.05 & sleep 0.05 & sleep 0.05 &\n"
          "sleep 0.1\n"
          "sleep 0.1\n"
          "sleep 0.5");
}
