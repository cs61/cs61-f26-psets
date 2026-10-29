// Test: Input redirection
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "pqle\n");
    t.run("tr pq Fi < f.txt ; echo Done");
    assert(t.stdout() == "File\nDone\n");
}
