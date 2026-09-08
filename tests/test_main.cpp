#include "TestRunner.hpp"

int main() {
    return pixelforge::test::TestRegistry::instance().run_all();
}
