// Bring in the main ngdevkit include file, this provides us access
// to all areas of the Neo Geo hardware. ngdevkit has also
// defined simple typedefs for the various numeric types such
// as s8 (signed 8 bit value, ie a signed byte), u16 (unsigned 16 bit value), etc.
#include <ngdevkit/neogeo.h>
// pull in our palette definitions
#include "animationDef.h"
#include "cromAnimationDefs.h"
#include "paletteDefs.h"

typedef u8 BOOL;
#define TRUE 1
#define FALSE 0

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

struct Entity
{
    s16 x;
    s16 y;
    s16 velX;
    s16 velY;
};

struct BallAnimation
{
    u8 currentFrame;
    u8 currentDuration;
};

struct Entity paddle = { .x = 16, .y = 200 };
struct Entity ball = { .x = 152, .y = 32, .velX = 1, .velY = 1 };

struct BallAnimation ballAnimation;

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

    u8 palette = 1;

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

// of the 381 sprites available to us, we will use the first one for the paddle
// we will also use sprite index 1 and 2, as our paddle is three tiles wide
#define PADDLE_SPRITE_INDEX 0
// and the fourth sprite for the ball
#define BALL_SPRITE_INDEX 3

// sets up the first three sprites in VRAM to be our paddle. Neo Geo
// sprites are always a single column, so since our paddle is 3 tiles wide,
// we need three sprites to display it
void load_paddle() {
    // if you look in paletteDefs.c, you will see the third palette (index 2)
    // is the correct palette for the paddle
    const u8 palette = 2;

    for (u8 tx = 0; tx < 3; ++tx) {
        // set the vram address register to the location for this sprite's tile map
        *REG_VRAMADDR = ADDR_SCB1 + (PADDLE_SPRITE_INDEX + tx) * SCB1_SPRITE_ENTRY_SIZE;
        // every time we write to vram, VRAMADDR will increase by one
        *REG_VRAMMOD = 1;

        // the first sprite is not sticky, it is the "control" sprite. the other
        // two are sticky and chain up to the first sprite
        u8 sticky = tx != 0;

        // set our tile index, we know the paddle starts at tile index 0 by
        // looking at the tiles in neospriteviewer
        *REG_VRAMRW = tx;
        // set the palette for this tile. This word also allows setting other things
        // such as whether the tile is flipped or not, but we don't need any of that today
        *REG_VRAMRW = palette << 8;

        // move to sprite control bank 2 to set the sprite's scaling
        *REG_VRAMADDR = ADDR_SCB2 + PADDLE_SPRITE_INDEX + tx;
        // this allows us to jump right into SCB3 and then SCB4 after we write to SCB2
        *REG_VRAMMOD = SCB234_SIZE;

        // set scale (horizontal and vertical)
        *REG_VRAMRW = 0xFFF;

        // now in SCB3, we set the sprite's y location and x location
        // as well as whether it is sticky or not, and its tile height (1 in this simple case)
        *REG_VRAMRW = (TO_SCREEN_Y(paddle.y) << 7) | (sticky << 6) | 1;
        // now in SCB4, we set x position
        *REG_VRAMRW = TO_SCREEN_X(paddle.x) << 7;
    }
}

// sets up the fourth sprite in VRAM to be our paddle.
void load_ball() {
    u8 ballTileIndex = animationDef_ball_spin.frames[ballAnimation.currentFrame].tileIndex;
    u8 paletteIndex = animationDef_ball_spin.frames[ballAnimation.currentFrame].paletteIndex;

    // set the vram address register to the location for this sprite's tile map
    *REG_VRAMADDR = ADDR_SCB1 + BALL_SPRITE_INDEX * SCB1_SPRITE_ENTRY_SIZE;
    // every time we write to vram, VRAMADDR will increase by one
    *REG_VRAMMOD = 1;

    // set our tile index
    *REG_VRAMRW = ballTileIndex;
    // set the palette for this tile. This word also allows setting other things
    // such as whether the tile is flipped or not, but we don't need any of that today
    *REG_VRAMRW = paletteIndex << 8;

    // move to sprite control bank 2 to set the sprite's scaling
    *REG_VRAMADDR = ADDR_SCB2 + BALL_SPRITE_INDEX;
    // this allows us to jump right into SCB3 and then SCB4 after we write to SCB2
    *REG_VRAMMOD = SCB234_SIZE;

    // set scale (horizontal and vertical)
    *REG_VRAMRW = 0xFFF;

    // now in SCB3, we set the sprite's y location and x location
    // as well as whether it is sticky or not, and its tile height (1 in this simple case)
    *REG_VRAMRW = (TO_SCREEN_Y(ball.y) << 7) | 1;
    // now in SCB4, we set x position
    *REG_VRAMRW = TO_SCREEN_X(ball.x) << 7;
}

// updates the current ball tile to match the current state of the animation
// notice we are only updating the ball tile index and the palette. Actually updating the sprite's
// location is still handled by move_ball below
void update_tile_ball() {
    u8 ballTileIndex = animationDef_ball_spin.frames[ballAnimation.currentFrame].tileIndex;
    u8 paletteIndex = animationDef_ball_spin.frames[ballAnimation.currentFrame].paletteIndex;

    // set the vram address register to the location for this sprite's tile map
    *REG_VRAMADDR = ADDR_SCB1 + BALL_SPRITE_INDEX * SCB1_SPRITE_ENTRY_SIZE;
    // every time we write to vram, VRAMADDR will increase by one
    *REG_VRAMMOD = 1;

    // set our tile index
    *REG_VRAMRW = ballTileIndex;
    // set the palette for this tile. This word also allows setting other things
    // such as whether the tile is flipped or not, but we don't need any of that today
    *REG_VRAMRW = paletteIndex << 8;
}

// updates the paddle sprite's location. since we are only changing the location,
// we only need to set a small number of values. Also since the paddle's second
// and third sprite were set to sticky, we don't need to set their location. changing
// the first sprite's location will cause them to automatically change too
void move_paddle() {
    // jump to SCB3 for y, then use mod to automatically jump to SCB4 for x
    *REG_VRAMADDR = ADDR_SCB3 + PADDLE_SPRITE_INDEX;
    *REG_VRAMMOD = SCB234_SIZE;

    // set the new y and also set height to 1 again. since these two values are packed
    // into the same word, we need to make sure we set both here, otherwise the sprite
    // would disappear on screen as height would get set to zero.
    // we don't need to set the sticky bit this time, since we are only working with the
    // control sprite, we know it is not sticky so not setting it here defaults it to zero
    *REG_VRAMRW = (TO_SCREEN_Y(paddle.y) << 7) | 1;

    // set new x
    *REG_VRAMRW = TO_SCREEN_X(paddle.x) << 7;
}

void move_ball() {
    // jump to SCB3 for y, then use mod to automatically jump to SCB4 for x
    *REG_VRAMADDR = ADDR_SCB3 + BALL_SPRITE_INDEX;
    *REG_VRAMMOD = SCB234_SIZE;

    // set the new y and also set height to 1 again. since these two values are packed
    // into the same word, we need to make sure we set both here, otherwise the sprite
    // would disappear on screen as height would get set to zero.
    // we don't need to set the sticky bit this time, since we are only working with the
    // control sprite, we know it is not sticky so not setting it here defaults it to zero
    *REG_VRAMRW = (TO_SCREEN_Y(ball.y) << 7) | 1;

    // set new x
    *REG_VRAMRW = TO_SCREEN_X(ball.x) << 7;
}

void ball_logic() {
    ball.x += ball.velX;
    ball.y += ball.velY;

    if (ball.x < 0) {
        ball.x = 0;
        ball.velX *= -1;
    }

    if (ball.x > 304) {
        ball.x = 304;
        ball.velX *= -1;
    }

    if (ball.y < 0) {
        ball.y = 0;
        ball.velY *= -1;
    }

    if (ball.y > 208) {
        ball.y = 208;
        ball.velY *= -1;
    }
}

BOOL ball_animation_logic() {
    ballAnimation.currentDuration += 1;

    if (ballAnimation.currentDuration == animationDef_ball_spin.frameDuration) {
        ballAnimation.currentFrame += 1;

        if (ballAnimation.currentFrame == animationDef_ball_spin.frameCount) {
            ballAnimation.currentFrame = 0;
        }

        ballAnimation.currentDuration = 0;

        return TRUE;
    }

    return FALSE;
}

// this is set to volatile because it will get changed in our
// vblank callback. that callback is invoked by the bios, so gcc
// doesn't understand how or why this could change. the volatile
// keyword is our way of telling gcc "be aware something outside
// our own code can change this value, so don't assume it is whatever
// we last set it to"
volatile u8 vblank = 0;

// ngdevkit calls this function whenever vblank happens. It must
// have this exact name, as that is how ngdevkit will find it
void rom_callback_VBlank() {
    vblank = 1;
}

// we want to wait for vblank before changing vram, so we
// just loop and continually check if the vblank callback has happened
void wait_vblank() {
    while (!vblank)
        ;
    vblank = 0;
}

// ngdevkit will call main() once it is ready to run your game,
int main() {
    // set the auto animation speed
    *REG_LSPCMODE = (6 << 8);

    init_palettes();
    fix_clear();

    // these two calls are changing VRAM and we have no idea
    // what state the screen is currently in. so these two
    // calls could cause a bit of onscreen garbage. but for
    // this simple demo, it's fine
    fix_print(3, 4, "Use left and right to move paddle");
    load_paddle();
    load_ball();

    for (;;) {
        //  bios_p1current has the current state of player 1's joystick and
        // A, B, C and D buttons. If the CNT_LEFT bit is high, that means left
        // is currently being pressed
        if (bios_p1current & CNT_LEFT) {
            paddle.x -= 3;
        }

        // and same thing for right
        if (bios_p1current & CNT_RIGHT) {
            paddle.x += 3;
        }

        ball_logic();
        // update the current state of the ball's animation
        // if this function returns TRUE, then it is time to display the next frame
        // of the animation, which we do below during vblank
        BOOL tileNeedsUpdating = ball_animation_logic();

        // we don't want to change the paddle's sprite location
        // until vblank has occured, that way the screen always
        // appears consistent to the player
        wait_vblank();
        // now that vblank has occured, we have a very small amount of time
        // to update VRAM. thankfully we aren't doing much at all, so we
        // will be just fine
        move_paddle();
        move_ball();

        if (tileNeedsUpdating) {
            // above ball_animation_logic told us to update the animation
            update_tile_ball();
        }
    }

    // we never actually get to this return, but we will
    // in more advanced code examples later
    return 0;
}
