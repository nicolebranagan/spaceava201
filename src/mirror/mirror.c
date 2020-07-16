/*
    Cutscenes
*/

#include <huc.h>

#include "./mirror/const.h"
#include "./images/images.h"
#include "cd.h"

#define SYSTEM_VRAM 0x1000
#define CURSOR_VRAM (SYSTEM_VRAM + (MIRRORSYS_SIZE / 2))
#define LASER_VRAM (SYSTEM_VRAM + (CURSOR_VRAM / 2))

#define TOP_Y 32
#define TOP_X 32
#define GRID_WIDTH 9
#define GRID_HEIGHT 10

#define SPR_SIZE_16x16 0x40

#incbin(systempal, "palettes/mirrorsys.pal");
#incbin(cursorpal, "palettes/cursors.pal");
#incbin(laserpal, "palettes/lasers.pal");

#incbin(systembat, "bats/mirrorsys.bin");

char x, y, timer;

char grid[GRID_WIDTH * GRID_HEIGHT];

initialize()
{
    char i;
    cls();
    x = 0;
    y = 0;
    timer = 0;
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    scroll(0, 0, 0, 0, 223, 0xC0);

    for (i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
    {
        grid[i] = SPACE_EMPTY;
    }

    grid[GRID_WIDTH + 4] = SPACE_RIGHT_LEFT_MIRROR;
    grid[GRID_WIDTH + 6] = SPACE_ANTIPHOTON;

    grid[GRID_WIDTH + GRID_WIDTH + GRID_WIDTH + 4] = SPACE_RIGHT_LEFT_MIRROR;
    grid[GRID_WIDTH + GRID_WIDTH + GRID_WIDTH + 2] = SPACE_PHOTON;

    cd_loadvram(IMAGE_OVERLAY, MIRRORSYS_SECTOR_OFFSET, SYSTEM_VRAM, MIRRORSYS_SIZE);
    cd_loadvram(IMAGE_OVERLAY, CURSORS_SECTOR_OFFSET, CURSOR_VRAM, CURSORS_SIZE);
    cd_loadvram(IMAGE_OVERLAY, LASERS_SECTOR_OFFSET, LASER_VRAM, LASERS_SIZE);
    load_palette(1, systempal, 1);
    load_palette(16, cursorpal, 1);
    load_palette(17, laserpal, 1);
    load_vram(0, systembat, 16 * 16 * 4);
}

draw_cursor()
{
    int draw_x;

    if (x == GRID_WIDTH && y == (GRID_HEIGHT - 1))
    {
        spr_set(PLAYER_SPRITE_1);
        spr_y(192);
        spr_x(208);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_pattern(CURSOR_VRAM + (SPR_SIZE_16x16 * 2));
        spr_pal(0);
        spr_pri(1);
        spr_show();

        spr_set(PLAYER_SPRITE_2);
        spr_y(192);
        spr_x(224);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_pattern(CURSOR_VRAM + (3 * SPR_SIZE_16x16));
        spr_pal(0);
        spr_pri(1);
        spr_show();
        return;
    }

    if (x < GRID_WIDTH)
    {
        draw_x = TOP_X + (x << 4);
    }
    else
    {
        draw_x = 216; // Palette
    }
    spr_set(0);
    spr_y(TOP_Y + (y << 4));
    spr_x(draw_x);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern((timer >> 4 & 1) ? CURSOR_VRAM : (CURSOR_VRAM + SPR_SIZE_16x16));
    spr_pal(0);
    spr_pri(1);
    spr_show();
    spr_set(1);
    spr_hide();
}

draw_beam(char sprdex, char i, int vram_offset)
{
    spr_set(sprdex);
    spr_x(TOP_X + ((i % GRID_WIDTH) << 4));
    spr_y((TOP_Y - 3) + ((i / GRID_WIDTH) << 4));
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(vram_offset + (SPR_SIZE_16x16 * ((timer & 63) >> 4)));
    spr_pal(1);
    spr_pri(1);
    spr_show();
}

draw_mirror(char sprdex, char i, char flip)
{
    spr_set(sprdex);
    spr_x(TOP_X + ((i % GRID_WIDTH) << 4));
    spr_y(TOP_Y + ((i / GRID_WIDTH) << 4));
    spr_ctrl(FLIP_MAS | SIZE_MAS, flip ? (SZ_16x16 | FLIP_X) : SZ_16x16);
    spr_pattern(LASER_VRAM + (SPR_SIZE_16x16 << 4));
    spr_pal(1);
    spr_pri(1);
    spr_show();
}

draw_grid()
{
    char i, grid_sprite;
    grid_sprite = GRID_SPRITE_START;

    for (i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
    {
        if (grid[i] == SPACE_EMPTY)
        {
            continue;
        }

        switch (grid[i])
        {
        case SPACE_PHOTON:
        {
            draw_beam(grid_sprite, i, LASER_VRAM);
            break;
        }
        case SPACE_ANTIPHOTON:
        {
            draw_beam(grid_sprite, i, LASER_VRAM + 4*SPR_SIZE_16x16);
            break;
        }
        case SPACE_LEFT_RIGHT_MIRROR:
        {
            draw_mirror(grid_sprite, i, 1);
            break;
        }
        case SPACE_RIGHT_LEFT_MIRROR:
        {
            draw_mirror(grid_sprite, i, 0);
        }
        }

        grid_sprite++;
    }
}

main()
{
    char joyt;

    initialize();
    for (;;)
    {
        timer++;
        vsync();
        draw_grid();
        draw_cursor();
        satb_update();

        joyt = joy(0);

        if (!(joyt & JOY_SLCT))
        {
            joyt = joytrg(0);
        }

        if (joyt & JOY_UP && y > 0)
        {
            y--;
        }
        if (joyt & JOY_DOWN && y < (GRID_HEIGHT - 1))
        {
            y++;
        }

        if (joyt & JOY_LEFT && x > 0)
        {
            x--;
        }
        if (joyt & JOY_RIGHT && x < (GRID_WIDTH))
        {
            x++;
        }
    }
}