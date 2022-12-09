#pragma once

#include <lore/lens/LensSchema.h>

struct SpotDiagram {
    void compute(const lore::LensSchema<float> &lens);
    void draw();

private:
    std::vector<lore::Vector2<float>> points;
    float spotRMS;
};

struct UI {
    UI();
    
    void draw();

private:
    void drawLens(lore::LensSchema<float> &lens);

    bool showImguiDemo = false;
    bool showImplotDemo = false;
    bool lensChanged = false;

    lore::LensSchema<float> lens;

    SpotDiagram spotDiagram;
};
