#pragma once

#include <lore/lens/LensSchema.h>

struct ParaxialConstants {
    void compute(const lore::LensSchema<float> &lens);
    void draw();

private:
    double effectiveFocalLength;
    double numericalAperture;
    double workingFNumber;
    double lagrangeInvariant;
    double lateralMagnification;
    double gaussianImageHeight;
    double petzvalRadius;
};
