#include "cromImageDefs.h"

const struct TileDef campFire_tileDefs[8] = {
    { 
        .index = 0, 
        .palette = 2,
        .autoAnimation = 4
    },
    { 
        .index = 4, 
        .palette = 2,
        .autoAnimation = 4
    },
    { 
        .index = 8, 
        .palette = 2,
        .autoAnimation = 4
    },
    { 
        .index = 12, 
        .palette = 2,
        .autoAnimation = 4
    },
    { 
        .index = 16, 
        .palette = 2,
        .autoAnimation = 4
    },
    { 
        .index = 20, 
        .palette = 2,
        .autoAnimation = 4
    },
    { 
        .index = 24, 
        .palette = 2,
        .autoAnimation = 4
    },
    { 
        .index = 28, 
        .palette = 2,
        .autoAnimation = 4
    }
};

const struct TileImageDef campFire_cromImageDef = {
    .tiles = campFire_tileDefs,
    .width = 2,
    .height = 4,
};

