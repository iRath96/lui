#pragma once

#include <lore/lens/LensSchema.h>

struct UI {
    UI();
    
    void draw();

private:
    bool showImguiDemo = false;
    bool showImplotDemo = false;

    lore::LensSchema<float> lens;
};
