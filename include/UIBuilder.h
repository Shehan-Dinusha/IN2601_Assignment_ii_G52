#ifndef UIBUILDER_H
#define UIBUILDER_H

class Engine;

class UIBuilder {
public:
    explicit UIBuilder(Engine& engine);

    void buildUI();

private:
    Engine& engine;
};

#endif
