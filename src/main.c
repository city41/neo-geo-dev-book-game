// Bring in the main ngdevkit include file, this provides us access
// to all areas of the Neo Geo hardware. ngdevkit has also
// defined simple typedefs for the various numeric types such
// as s8 (signed 8 bit value, ie a signed byte), u16 (unsigned 16 bit value), etc.
#include <ngdevkit/neogeo.h>

// bring in our campfire definition
#include "cromImageDefs.h"
// pull in our palette definitions
#include "paletteDefs.h"

// Address of Sprite Control Block in VRAM
#define ADDR_SCB1 0
#define ADDR_SCB2 0x8000
#define ADDR_SCB3 0x8200
#define ADDR_SCB4 0x8400

// how large, in words, the SCB2, SCB3 and SCB4 sections are (0x200)
#define SCB234_SIZE (ADDR_SCB4 - ADDR_SCB3)

// how many words one sprite entry in SCB1 takes up
#define SCB1_SPRITE_ENTRY_SIZE 64

#define TO_SCREEN_Y(inputY) (-((inputY) + 496) - 32)
#define TO_SCREEN_X(inputX) (inputX)

// Load our palettes into RAM
// MMAP_PALBANK1 is the location in main RAM where
// palette data goes.
void init_palettes() {
    for (u8 i = 0; i < NUM_PALETTE_ENTRIES; ++i) {
        MMAP_PALBANK1[i] = palettes[i];
    }
}

// Clears out the fix layer by writing tile index 0xFF (255)
// to all of the fix tile locations. Tile index 0xFF should
// be blank in a real Neo Geo game as the eyecatcher uses this tile
// and it assumes the tile is blank. Also the Neo Geo's BIOS has a fix clear
// routine that also uses 0xFF and it too assumes the tile will be blank.
void fix_clear() {
    u8 palette = 0;
    u16 tileValue = (palette << 12) | 0xFF;

    *REG_VRAMADDR = ADDR_FIXMAP;
    *REG_VRAMMOD = 1;

    for (u16 i = 0; i < 40 * 32; ++i) {
        *REG_VRAMRW = tileValue;
    }
}

// sets up the fourth sprite in VRAM to be our paddle.
void load_campfire() {
    const u8 spriteIndex = 0;

    const u8 width = campFire_cromImageDef.width;
    const u8 height = campFire_cromImageDef.height;

    const u16 screenX = 144;
    const u16 screenY = 96;

    // setup each sprite for however wide the campfire is in tiles
    for (u8 tx = 0; tx < width; ++tx) {
        // SCB1: tile definitions
        *REG_VRAMADDR = ADDR_SCB1 + (spriteIndex + tx) * SCB1_SPRITE_ENTRY_SIZE;
        *REG_VRAMMOD = 1;

        u8 sticky = tx != 0;

        // load the tiles for however tall the campfire is
        for (u8 ty = 0; ty < height; ++ty) {
            const struct TileDef* tile = campFire_cromImageDef.tiles + tx + ty * width;
            *REG_VRAMRW = tile->index;

            // here we are specifying auto animation, if you look in cromImageDefs.c, you will
            // see this value is 4. 4 corresponds to the correct bit for setting up a 4 frame
            // auto animation
            *REG_VRAMRW = (tile->palette << 8) | tile->autoAnimation;
        }

        *REG_VRAMMOD = SCB234_SIZE;
        *REG_VRAMADDR = ADDR_SCB2 + spriteIndex + tx;

        // set scale (horizontal and vertical)
        *REG_VRAMRW = 0xfff;

        // y position, sticky bit, height in tiles
        *REG_VRAMRW = (TO_SCREEN_Y(screenY) << 7) | (sticky << 6) | height;
        // x position
        *REG_VRAMRW = TO_SCREEN_X(screenX) << 7;
    }
}

// ngdevkit will call main() once it is ready to run your game,
int main() {
    // set the auto animation speed
    *REG_LSPCMODE = (4 << 8);

    init_palettes();
    fix_clear();

    load_campfire();

    for (;;) { }

    // we never actually get to this return, but we will
    // in more advanced code examples later
    return 0;
}
