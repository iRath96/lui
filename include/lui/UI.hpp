#pragma once

#include <lore/lens/LensSchema.h>

#include "SpotDiagram.hpp"
#include "RayInterceptCurve.hpp"

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
    RayInterceptCurve rayInterceptCurve;
};
