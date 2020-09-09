#ifndef CLASSIC
#define CLASSIC 1

#include "adpcm/adpcm.h"
#include "cd.h"

#define EUREKA_LOC 0
#define DIE_LOC (EUREKA_LOC + EUREKA_SIZE)
#define PHOTON_LOC (DIE_LOC + DIE_SIZE)
#define CANNON_LOC (PHOTON_LOC + PHOTON_SIZE)
#define WILHELM_LOC (CANNON_LOC + CANNON_SIZE)
#define SHOVE_LOC (WILHELM_LOC + MINIWILHELM_SIZE)
#define TILE_SND_LOC (SHOVE_LOC + SHOVE_SIZE)

#define SPR_SIZE_16x16 0x40

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

#define AVA_VRAM 0x5000

#define TOP_HALF_START 0
#define BOTTOM_HALF_START 32

char ava_x;
char ava_y;
char ava_facing;

char last_ava_x;
char last_ava_y;

char timer;

void wait_for_sync(char cycles);

#endif
