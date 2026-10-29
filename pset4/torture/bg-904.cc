// Test: A background command does not leave an extra shell running
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    // At 0.25 sec, `echo` has exited and the shell is running `sleep 0.5`
    t.after(0.25, [&] {
        int nother = 0;
        for (auto& p : t.processes()) {
            if (p.name != "sleep" && p.state != sh61_process_state::zombie) {
                ++nother;
            }
        }
        // The only other live process is the shell itself
        assert(nother == 1);
    });
    t.run("echo &\n"
          "sleep 0.5");
}
