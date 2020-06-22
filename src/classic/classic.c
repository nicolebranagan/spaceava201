/*
	Classic style Space Ava grid-based gameplay
*/

#include <huc.h>
#include "images/images.h"

//TODO: factor these out into a global include

#define SPR_SIZE_16x16 0x40
#define IMAGE_OVERLAY 5

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

int sx, sy; // scroll x, scroll y
char ava_x;
char ava_y;
char ava_facing;
char timer;

#include "classic/enemy.c"

#incbin(avapal, "palettes/ava.pal");
#incbin(tilepal1, "palettes/starbase.pal");

const char palette_ref[] = {
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10,
    0x10};

char tiles[2048];

// TODO: Make this a bitfield if you need to
const char TILE_SOLIDITY[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};

initialize()
{
    set_xres(256);
    set_screen_size(SCR_SIZE_64x32);
    ad_reset();
    reset_satb();

    cd_loadvram(IMAGE_OVERLAY, AVA_SECTOR_OFFSET, 0x5000, AVA_SIZE_IN_BYTES);
    load_palette(16, avapal, 1);

    timer = 0;
    ava_x = 10;
    ava_y = 8;
    ava_facing = DOWN;
    draw_ava(0, ava_x * 16, ava_y * 16);
}

draw_ava(char moving, int x, int y)
{
    char frame, frame_offset;
    char ctrl_flags = SZ_16x16;
    int dx, dy; // display x, display y

    sx = x - 128;
    if (sx < 0)
    {
        sx = 0;
    }
    sy = y - 128;
    if (sy < 0)
    {
        sy = 0;
    }

    dx = x - sx;
    dy = y - sy;

    scroll(0, sx, sy, 0, 223, 0xC0);
    load_map(sx >> 4, sy >> 4, sx >> 4, sy >> 4, 17, 15);
    if (moving)
    {
        quick_draw_enemies();
    }

    switch (ava_facing)
    {
    case DOWN:
        frame = 0;
        break;
    case UP:
        frame = 5;
        break;
    case LEFT:
        ctrl_flags = ctrl_flags | FLIP_X;
    case RIGHT:
        frame = 3;
        break;
    }

    if (moving)
    {
        switch (ava_facing)
        {
        case DOWN:
        case UP:
            frame_offset = (timer >> 3) & 3;
            if (frame_offset == 1)
            {
                frame = frame + 1;
            }
            if (frame_offset == 3)
            {
                frame = frame + 2;
            }
            break;
        case LEFT:
        case RIGHT:
            if ((timer >> 3) & 1)
            {
                frame = frame + 1;
            }
        }
    }

    spr_set(0);
    spr_x(dx);
    spr_y(dy - 16);
    spr_pattern(0x5000 + (2 * frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, ctrl_flags);
    spr_pal(0);
    spr_pri(1);
    spr_show();

    spr_set(1);
    spr_x(dx);
    spr_y(dy);
    spr_pattern(0x5000 + (2 * frame * SPR_SIZE_16x16) + SPR_SIZE_16x16);
    spr_ctrl(FLIP_MAS | SIZE_MAS, ctrl_flags);
    spr_pal(0);
    spr_pri(1);
    spr_show();

    return;
}

move_ava(char negative, char delx, char dely)
{
    char i;
    int x, y;

    x = ava_x * 16;
    y = ava_y * 16;

    for (i = 0; i < 8; i++)
    {
        if (negative)
        {
            x = x - 2 * delx;
            y = y - 2 * dely;
        }
        else
        {
            x = x + 2 * delx;
            y = y + 2 * dely;
        }

        draw_ava(1, x, y);
        satb_update();
        vsync();
        timer++;

        draw_ava(1, x, y);
        satb_update();
        vsync();
        timer++;
    }

    if (negative)
    {
        ava_x = ava_x - delx;
        ava_y = ava_y - dely;
    }
    else
    {
        ava_x = ava_x + delx;
        ava_y = ava_y + dely;
    }

    draw_ava(0, ava_x * 16, ava_y * 16);
    satb_update();
}

load_room()
{
    int i;
    char x, y, tile, err;
    char tiledata[STARBASE_SIZE_IN_BYTES];

    cd_loaddata(IMAGE_OVERLAY, STARBASE_SECTOR_OFFSET, tiledata, STARBASE_SIZE_IN_BYTES);

    set_tile_data(tiledata, 16, palette_ref, 16);
    load_palette(1, tilepal1, 1);
    load_tile(0x2000);

    set_font_pal(0);
    set_font_color(4, 0);
    load_default_font();

    err = cd_loaddata(4, 0, tiles, 0x7ff);

    if (err)
    {
        put_hex(err, 4, 10, 5);
        put_hex(tiles, 4, 10, 6);
        return;
    }

    i = 0;
    set_map_data(tiles, 64, 32);
    load_map(sx >> 4, sy >> 4, sx >> 4, sy >> 4, 17, 15);
}

char is_solid(char x, char y)
{
    char tile, tilesolid, i;
    tile = map_get_tile(x, y);

    if (TILE_SOLIDITY[tile] == 0)
    {
        return 1;
    }

    return 0;
}

main()
{
    int vram_offset;
    char joyt;

    vram_offset = 0x5000 + AVA_SIZE_IN_BYTES;

    disp_off();
    initialize();
    load_room();
    init_enemy();
    vram_offset = create_enemy(vram_offset, TYPE_BIGMOUTH, 6, 4, DOWN, 0, 0);
    vram_offset = create_enemy(vram_offset, TYPE_BIGMOUTH, 8, 4, DOWN, 0, 0);
    disp_on();

    for (;;)
    {
        draw_enemies();
        satb_update();
        timer++;

        joyt = joytrg(0);
        if (joyt & JOY_UP && !is_solid(ava_x, ava_y - 1))
        {
            ava_facing = UP;
            move_ava(1, 0, 1);
            update_enemies();
        }
        if (joyt & JOY_DOWN && !is_solid(ava_x, ava_y + 1))
        {
            ava_facing = DOWN;
            move_ava(0, 0, 1);
            update_enemies();
        }
        if (joyt & JOY_LEFT && !is_solid(ava_x - 1, ava_y))
        {
            ava_facing = LEFT;
            move_ava(1, 1, 0);
            update_enemies();
        }
        if (joyt & JOY_RIGHT && !is_solid(ava_x + 1, ava_y))
        {
            ava_facing = RIGHT;
            move_ava(0, 1, 0);
            update_enemies();
        }
    }
}
