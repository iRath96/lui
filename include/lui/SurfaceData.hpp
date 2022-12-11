#pragma once

#include <lore/lens/LensSchema.h>

struct SurfaceData {
    void draw(lore::LensSchema<float> &lens, bool &lensChanged);

private:
    std::vector<std::string> apertureFlags;
};
