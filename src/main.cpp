#include "cli/CliApp.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        pixelforge::cli::CliApp app;
        return app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }
}
