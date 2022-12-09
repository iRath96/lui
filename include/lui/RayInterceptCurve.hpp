#pragma once

#include <lore/lens/LensSchema.h>

#include <string>
#include <vector>

struct RayInterceptCurve {
    void compute(const lore::LensSchema<float> &lens);
    void draw();

private:
    struct SubPlot {
        std::string name;
        lore::Vector2<float> points;
    };
    std::vector<SubPlot> subPlots;
};
