// Test: A background command keeps running after its shell exits
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("cmd.sh", "echo Hello; sleep 4 & echo Bye; sleep 0.1\n");
    t.run("sh61 -q cmd.sh; echo Done");
    assert(t.stdout() == "Hello\nBye\nDone\n");
    assert(t.elapsed() < 3);
    int nsleep = 0;
    for (auto& p : t.processes()) {
        if (p.name == "sleep" && p.state == sh61_process_state::running) {
            ++nsleep;
        }
    }
    assert(nsleep == 1);
}
