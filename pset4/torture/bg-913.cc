// Test: A pipeline runs after a background command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("sleep 4 & sleep 0.2; echo Sleeping | cat | head -n 1");
    assert(t.stdout() == "Sleeping\n");
    assert(t.elapsed() < 3);
    int nsleep = 0;
    for (auto& p : t.processes()) {
        if (p.name == "sleep" && p.state == sh61_process_state::running) {
            ++nsleep;
        }
    }
    assert(nsleep == 1);
}
