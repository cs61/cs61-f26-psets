// Test: Input and output redirections on one command
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("fa.txt", "Hello\nBye\n");
    t.run("sort < fa.txt > fb.txt ; tail -n 2 fb.txt ; rm -f fa.txt fb.txt");
    assert(t.stdout() == "Bye\nHello\n");
}
