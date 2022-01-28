#pragma once
#include <ngdevkit/neogeo.h>

struct FrameDef {
    s8 tileIndex;
    s8 paletteIndex;
};

struct AnimationDef {
    const struct FrameDef* frames;
    s8 frameCount;
    s8 frameDuration;
};

