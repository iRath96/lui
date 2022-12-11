#pragma once

#include <lore/lens/LensSchema.h>

#include "SpotDiagram.hpp"
#include "RayInterceptCurve.hpp"
#include "ParaxialConstants.hpp"
#include "SurfaceData.hpp"
#include "RayAimer.hpp"

#include <vector>
#include <string>

struct UI {
    UI();
    
    void draw();

private:
    void drawLens(lore::LensSchema<float> &lens);
    void loadLens(const std::string &path);

    std::vector<std::string> availableLenses;

    bool showImguiDemo = false;
    bool showImplotDemo = false;
    bool lensChanged = false;

    lore::LensSchema<float> lens;

    SurfaceData surfaceData;
    SpotDiagram spotDiagram;
    RayInterceptCurve rayInterceptCurve;
    ParaxialConstants paraxialConstants;
    RayAimer rayAimer;
};
