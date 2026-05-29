#include "../include/Engine.h"
#include <iostream>

int main() {
    // Instantiate the engine with default window size and title
    Engine engine(800, 600, "2D Vector CAD Editor");

    if (!engine.initialize()) {
        std::cerr << "Failed to initialize the graphics engine." << std::endl;
        return -1;
    }

    // Example setup: Set a world window coordinate system (e.g., -100 to 100)
    engine.setWorldWindow(-100.0f, 100.0f, -100.0f, 100.0f);

    std::cout << "Starting 2D Vector CAD Editor..." << std::endl;

    // Start the application frame loop
    engine.run();

    return 0;
}

