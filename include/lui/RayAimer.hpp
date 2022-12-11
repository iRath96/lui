#pragma once

#include <lore/lens/LensSchema.h>

struct RayAimer {
    void compute(const lore::LensSchema<float> &lens);
    void draw();

private:
    struct Point {
        float x;
        float y;
        float dy;
    };

    std::vector<Point> points;
};
