// Test: A background shell runs every command in its list
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("cmd.sh", "echo Hello; sleep 1\n");
    // At 0.5 sec, the background `sh61` has run `echo Hello` and is running
    // `sleep 1`
    t.after(0.5, [&] {
        assert(t.stdout() == "Hello\n");
        int nsleep = 0;
        for (auto& p : t.processes()) {
            if (p.name == "sleep" && p.state == sh61_process_state::running) {
                ++nsleep;
            }
        }
        assert(nsleep == 1);
    });
    t.run("sh61 -q cmd.sh &");
}
