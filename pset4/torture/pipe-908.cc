// Test: A four-command pipeline
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("echo GoHangASalamiImALasagnaHog | rev | rev | rev");
    assert(t.stdout() == "goHangasaLAmIimalaSAgnaHoG\n");
}
