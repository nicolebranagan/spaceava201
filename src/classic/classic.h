#ifndef CLASSIC

#define CLASSIC 1

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
