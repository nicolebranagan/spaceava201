/*
    Cutscenes
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#define SYSTEM_VRAM 0x1000
#define CURSOR_VRAM (SYSTEM_VRAM + (MIRRORSYS_SIZE / 2))

#define TOP_Y 32
#define TOP_X 32
#define GRID_WIDTH 9
#define GRID_HEIGHT 10

#define SPR_SIZE_16x16 0x40

#incbin(systempal, "palettes/mirrorsys.pal");
#incbin(cursorpal, "palettes/cursors.pal");

#incbin(systembat, "bats/mirrorsys.bin");

char x, y, timer;

initialize()
{
    cls();
    x = 0;
    y = 0;
    timer = 0;
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    scroll(0, 0, 0, 0, 223, 0xC0);

    cd_loadvram(IMAGE_OVERLAY, MIRRORSYS_SECTOR_OFFSET, SYSTEM_VRAM, MIRRORSYS_SIZE);
    cd_loadvram(IMAGE_OVERLAY, CURSORS_SECTOR_OFFSET, CURSOR_VRAM, CURSORS_SIZE);
    load_palette(1, systempal, 1);
    load_palette(16, cursorpal, 1);
    load_vram(0, systembat, 16 * 16 * 4);
}

draw_cursor()
{
    spr_set(0);
    spr_y(TOP_Y + (y << 4));
    spr_x(TOP_X + (x << 4));
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern((timer >> 4 & 1) ? CURSOR_VRAM : (CURSOR_VRAM + SPR_SIZE_16x16));
    spr_pal(0);
    spr_show();
}

main()
{
    initialize();
    for (;;)
    {
        timer++;
        vsync();
        draw_cursor();
        satb_update();
    }
}