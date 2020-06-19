/*
	Classic style Space Ava grid-based gameplay
*/

#include <huc.h>

#incspr(avachr, "images/ava.png");
#incpal(avapal, "images/ava.png");

#inctile(roomtile, "images/tiles.png", 16, 1);
#incpal(tilepal1, "images/tiles.png");

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

#define SPR_SIZE_16x16 0x40

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

char ava_x;
char ava_y;
char ava_facing;
char timer;

char tiles[2048];

// TODO: Make this a bitfield if you need to
const char TILE_SOLIDITY[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};

initialize()
{
    disp_off();
    spr_set();
    set_screen_size(SCR_SIZE_64x32);
    ad_reset();
    reset_satb();

    load_vram(0x5000, avachr, SPR_SIZE_16x16 * 16);
    load_palette(16, avapal, 1);

    timer = 0;
    ava_x = 4;
    ava_y = 8;
    ava_facing = DOWN;
    draw_ava(0, ava_x * 16, ava_y * 16);

    disp_on();
}

draw_ava(char moving, int x, int y)
{
    char frame, frame_offset;
    char ctrl_flags = SZ_16x16;
    int sx, sy; // scroll x, scroll y
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
    put_hex(delx, 4, 10, 8);
    put_hex(dely, 4, 10, 9);
    put_hex(x, 4, 10, 10);
    put_hex(y, 4, 10, 11);

    for (i = 0; i < 16; i++)
    {
        if (negative)
        {
            x = x - delx;
            y = y - dely;
        }
        else
        {
            x = x + delx;
            y = y + dely;
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

    set_tile_data(roomtile, 16, palette_ref, 16);
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
    load_map(0, 0, 0, 0, 17, 15);
}

char is_solid(char x, char y)
{
    char tile;
    tile = map_get_tile(x, y);
    return TILE_SOLIDITY[tile] == 0;
}

main()
{
    char joyt;

    initialize();
    load_room();

    for (;;)
    {
        vsync();
        satb_update();
        timer++;

        joyt = joytrg(0);
        if (joyt & JOY_UP && !is_solid(ava_x, ava_y - 1))
        {
            ava_facing = UP;
            move_ava(1, 0, 1);
        }
        if (joyt & JOY_DOWN && !is_solid(ava_x, ava_y + 1))
        {
            ava_facing = DOWN;
            move_ava(0, 0, 1);
        }
        if (joyt & JOY_LEFT && !is_solid(ava_x - 1, ava_y))
        {
            ava_facing = LEFT;
            move_ava(1, 1, 0);
        }
        if (joyt & JOY_RIGHT && !is_solid(ava_x + 1, ava_y))
        {
            ava_facing = RIGHT;
            move_ava(0, 1, 0);
        }
        put_hex(map_get_tile(ava_x, ava_y), 4, 10, 11);
    }
}
