#pragma once
#include <ngdevkit/neogeo.h>

struct TileDef
{
    const u16 index;
    const u8 palette;
    const u8 autoAnimation;
};

struct TileImageDef
{
    const struct TileDef* tiles;
    const u8 width;
    const u8 height;
};
