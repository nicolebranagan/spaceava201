#ifndef CLASSIC
#define CLASSIC 1

#include "adpcm/adpcm.h"

#define ADPCM_OVERLAY 2

#define EUREKA_LOC 0
#define PHOTON_LOC (EUREKA_LOC + EUREKA_SIZE)
#define CANNON_LOC (PHOTON_LOC + PHOTON_SIZE)

#define SPR_SIZE_16x16 0x40
#define IMAGE_OVERLAY 5

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

#define AVA_VRAM 0x5000

int sx, sy; // scroll x, scroll y
char ava_x;
char ava_y;
char ava_facing;
char timer;

void wait_for_sync(char cycles);

#endif
