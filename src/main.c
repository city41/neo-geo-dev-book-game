// Bring in the main ngdevkit include file, this provides us access
// to all areas of the Neo Geo hardware. ngdevkit has also
// defined simple typedefs for the various numeric types such
// as s8 (signed 8 bit value, ie a signed byte), u16 (unsigned 16 bit value), etc.
#include <ngdevkit/neogeo.h>

// These are the values for the colors black and white in the Neo Geo's
// 16 bit color depth format. To convert an RGB color to this format,
// there is a handy tool at the bottom of this page:
// https://wiki.neogeodev.org/index.php?title=Colors

#define BLACK 0x8000
#define WHITE 0x7FFF

// The very simple palette we will load into RAM.
// On real hardware, the first color of the first palette
// must be black. The hardware uses this color to calibrate
// various things, and so if it is not black, you can get a messed up display.
#define PALETTE_SIZE 2
const u16 palette[PALETTE_SIZE] = { BLACK, WHITE };

// Takes our simple palette and loads it into RAM.
// MMAP_PALBANK1 is the location in main RAM where
// palette data goes.
void init_palette() {
    for (u8 i = 0; i < PALETTE_SIZE; ++i) {
        MMAP_PALBANK1[i] = palette[i];
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

// Takes an ASCII string as defined in the text variable,
// and writes it into the fix layer for display. This is only
// certain to work with our own S ROM, because we need to know
// where in the ROM the tiles are in order to pick the correct indices.
void fix_print(u16 x, u16 y, const u8* text) {

    // set the VRAM address register to the area in
    // the fix map we want to set our tiles
    *REG_VRAMADDR = ADDR_FIXMAP + (x * 32) + y;

    // every time we write to REG_VRAMRW, REG_VRAMADDR will
    // increment by this many words. Since the fix map
    // is stored in columns, 32 jumps us to the next x location
    *REG_VRAMMOD = 32;

    u8 palette = 0;

    while (*text) {
        // the S ROM used by this game has the tiles stored just one
        // off from the ASCII encoding, so we add a one to get the
        // tile index we need.
        u16 tileIndex = *text + 1;

        // send the palette and tile data to the fix map
        *REG_VRAMRW = (palette << 12) | tileIndex;

        text += 1;
    }
}

// ngdevkit will call main() once it is ready to run your game,
int main() {
    init_palette();
    fix_clear();

    // normally we only want to touch video RAM when the screen
    // is not being drawn. But for such a simple app we don't
    // need to worry about that
    fix_print(10, 14, "Hello Neo Geo!");

    // an infinite loop that allows our game to keep running
    for (;;) { }

    // we never actually get to this return, but returning an int
    // is convention for main() in most C environments
    return 0;
}
