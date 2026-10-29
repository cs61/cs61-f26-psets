// Test: `&` ends a background command in a list
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("cmd.sh", "echo Hello; sleep 4& echo Bye\n");
    t.after(0.3, [&] {
        int nsleep = 0;
        for (auto& p : t.processes()) {
            if (p.name == "sleep" && p.state == sh61_process_state::running) {
                ++nsleep;
            }
        }
        assert(nsleep == 1);
    });
    t.run("sh61 -q cmd.sh");
    assert(t.stdout() == "Hello\nBye\n");
    assert(t.elapsed() < 3);
}
