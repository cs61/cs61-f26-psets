// Test: A four-command pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.write_file("f.txt", "goHangasaLAmIimalaSAgnaHoG\n");
    t.run("rev f.txt | md5sum | tr a-z A-Z | cut -c 1-32");
    assert(t.stdout() == "502B109B37EC769342948826736FA063\n");
}
