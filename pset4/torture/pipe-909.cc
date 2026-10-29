// Test: A four-command pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "goHangasaLAmIimalaSAgnaHoG\n");
    t.run("cat f.txt | tr A-Z a-z | md5sum | cut -c 1-32");
    assert(t.stdout() == "8e21d03f7955611616bcd2337fe9eac1\n");
}
