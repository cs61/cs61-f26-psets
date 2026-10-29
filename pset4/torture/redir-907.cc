// Test: Input redirection and a pipeline into a shell
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("cmd.sh", "head -n 2 ; echo First && echo Good\n");
    t.write_file("f.txt", "hello\nbye\n");
    t.run("tr hb HB < f.txt | sort | sh61 -q cmd.sh");
    assert(t.stdout() == "Bye\nHello\nFirst\nGood\n");
}
