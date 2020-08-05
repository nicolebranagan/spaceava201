/*
    Side-scroller style Space Ava gameplay
*/

#include <huc.h>

#include "cd.h"
#include "neptune/neptune.h"

#incbin(avapal, "palettes/avaside.pal");
#incbin(neptunepal, "palettes/neptune.pal");

initialize()
{
    ad_reset();
    cd_loadvram(IMAGE_OVERLAY, NEPTUNE_SECTOR_OFFSET, NEPTUNE_VRAM, NEPTUNE_SIZE);
    cd_loadvram(IMAGE_OVERLAY, AVASIDE_SECTOR_OFFSET, AVA_VRAM, AVASIDE_SIZE);

    disp_off();
    reset_satb();
    set_xres(256);
    set_screen_size(SCR_SIZE_64x32);
    cls();

    load_palette(0, neptunepal, 1);
    load_palette(16, avapal, 1);
    init_ava();
    draw_map();
    scroll(0, 0, 0, 0, 223, 0xC0);
    disp_on();
}

#define MAP_BYTE_WIDTH (LEVEL_WIDTH_16x16 * 2)
#define TILE_START ((NEPTUNE_VRAM) >> 4)
draw_map()
{
    char x, y;
    int addr;
    int row1bytes[MAP_BYTE_WIDTH];
    int row2bytes[MAP_BYTE_WIDTH];

    for (y = 0; y < LEVEL_HEIGHT_16x16; y++)
    {
        for (x = 0; x < LEVEL_WIDTH_16x16; x++)
        {
            row1bytes[x << 1] = TILE_START + 0;
            row1bytes[(x << 1) + 1] = TILE_START + 1;
            row2bytes[x << 1] = TILE_START + 2;
            row2bytes[(x << 1) + 1] = TILE_START + 3;
        }
        addr = vram_addr(0, y << 1);
        load_vram(addr, row1bytes, MAP_BYTE_WIDTH);
        addr = vram_addr(0, (y << 1) + 1);
        load_vram(addr, row2bytes, MAP_BYTE_WIDTH);
    }
}

init_ava()
{
    spr_set(AVA_SPRITE);
    spr_x(0);
    spr_y(0);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(AVA_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_show();
}

draw_ava(int x, char y)
{
    spr_set(0);
    spr_x(x);
    spr_y(y);
    spr_show();
}

main()
{
    initialize();

    draw_ava(32, 32);
    for (;;)
    {
        vsync();
        satb_update();
    }
}