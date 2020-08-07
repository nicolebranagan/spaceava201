#ifndef NEPTUNE_H
#define NEPTUNE_H 1

#include "images/images.h"
#include "adpcm/adpcm.h"

#define NEPTUNE_VRAM 0x2000
#define AVA_VRAM (NEPTUNE_VRAM + (NEPTUNE_SIZE / 2))
#define SIDEOBJ_VRAM (AVA_VRAM + (AVA_SIZE / 2))
#define SIDENMY_VRAM (SIDEOBJ_VRAM + (OBJSIDE_SIZE / 2))

#define PCM_JUMP 0
#define PCM_DIE (PCM_JUMP + JUMP_SIZE)
#define PCM_PHOTON (PCM_DIE + DIE_SIZE)
#define PCM_EUREKA (PCM_PHOTON + PHOTON_SIZE)

#define SIDEOBJ_PAL (17)
#define SIDENMY_PAL (18)

#define SPR_SIZE_16x16 0x40

#define LEVEL_WIDTH_16x16 64
#define LEVEL_HEIGHT_16x16 32

#define AVA_STATE_STANDING 0
#define AVA_STATE_FALLING 1
#define AVA_STATE_FALL_NO 2
#define AVA_STATE_ON_LADDER 3

#define TILE_EMPTY 0
#define TILE_SOLID 1
#define TILE_LADDER 2

#define AVA_SPRITE 0
#define POINTER_SPR_UP 63
#define POINTER_SPR_DOWN 62
#define POINTER_SPR_LEFT 61
#define POINTER_SPR_RIGHT 60

#define AVA_STANDING AVA_VRAM
#define AVA_WALK_1 (AVA_VRAM + SPR_SIZE_16x16)
#define AVA_WALK_2 (AVA_VRAM + (2 * SPR_SIZE_16x16))
#define AVA_JUMP (AVA_VRAM + (3 * SPR_SIZE_16x16))
#define AVA_LADDER (AVA_VRAM + (4 * SPR_SIZE_16x16))

const int AVA_WALK_FRAMES[] = {
    AVA_STANDING,
    AVA_STANDING,
    AVA_WALK_1,
    AVA_WALK_1,
    AVA_WALK_1,
    AVA_WALK_1,
    AVA_STANDING,
    AVA_STANDING,
    AVA_STANDING,
    AVA_STANDING,
    AVA_WALK_2,
    AVA_WALK_2,
    AVA_WALK_2,
    AVA_WALK_2,
    AVA_STANDING,
    AVA_STANDING};

void wait_for_sync(char cycles);

#define POINTERU_VRAM (AVA_VRAM + (14 * SPR_SIZE_16x16))
#define POINTERL_VRAM (AVA_VRAM + (15 * SPR_SIZE_16x16))

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

char timer;
int sx, sy;

char ava_x, ava_y, ava_state, step, facing_left;
char allowed_up, allowed_down, allowed_left, allowed_right;

#endif