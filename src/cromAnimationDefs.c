#include "cromAnimationDefs.h"

// start ball

const struct FrameDef frameDefs_ball_spin[4] = {
    {
        .tileIndex = 3,
        .paletteIndex = 2
    },
    {
        .tileIndex = 4,
        .paletteIndex = 2
    },
    {
        .tileIndex = 5,
        .paletteIndex = 2
    },
    {
        .tileIndex = 6,
        .paletteIndex = 2
    }
};

const struct AnimationDef animationDef_ball_spin = {
    .frames = frameDefs_ball_spin,
    .frameCount = 4,
    .frameDuration = 10
};


// end ball
