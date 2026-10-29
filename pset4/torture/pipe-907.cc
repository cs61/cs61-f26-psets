// Test: A pipeline whose first command reads a file
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "goHangasaLAmIimalaSAgnaHoG\n");
    t.run("rev f.txt | rev");
    assert(t.stdout() == "goHangasaLAmIimalaSAgnaHoG\n");
}
