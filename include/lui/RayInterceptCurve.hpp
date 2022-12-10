#pragma once

#include <lore/lens/LensSchema.h>

#include <string>
#include <vector>

struct RayInterceptCurve {
    void compute(const lore::LensSchema<float> &lens);
    void draw();

private:
    struct SubPlot {
        float wavelength;
        std::vector<lore::Vector2<float>> points;
    };
    std::vector<SubPlot> subplots;
};
