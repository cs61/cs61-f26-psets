// Test: A background shell runs while the shell continues
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("cmd.sh", "echo Hello; sleep 4\n");
    t.run("sh61 -q cmd.sh &\n"
          "sleep 0.3");
    assert(t.stdout() == "Hello\n");
    assert(t.elapsed() < 3);
    int nsleep = 0;
    for (auto& p : t.processes()) {
        if (p.name == "sleep" && p.state == sh61_process_state::running) {
            ++nsleep;
        }
    }
    assert(nsleep == 1);
}
