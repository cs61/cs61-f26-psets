// Test: The shell does not wait for background commands
#include "shtest.hh"

int main(int argc, char* argv[]) {
    sh61_tester t(argc, argv);
    t.run("sh -c \"sleep 0.2; echo Second\" & "
          "sh -c \"sleep 0.1; echo First\" & "
          "sleep 0.3");
    assert(t.stdout() == "First\nSecond\n");
}
