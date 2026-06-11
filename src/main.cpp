#include "Engine.h"
#include <iostream>

int main() {
    Engine engine(800, 600, "2D Vector CAD Editor");

    if (!engine.initialize()) {
        std::cerr << "Failed to initialize the graphics engine." << std::endl;
        return -1;
    }

    engine.setWorldWindow(-100.0f, 100.0f, -100.0f, 100.0f);

    engine.run();

    return 0;
}

