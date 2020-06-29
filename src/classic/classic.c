/*
	Classic style Space Ava grid-based gameplay
*/

#include <huc.h>
#include "images/images.h"

//TODO: factor these out into a global include

#include "classic/classic.h"
#include "classic/enemy.c"
#include "classic/object.c"

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

char tiles[3000]; // Placeholder for memory

// TODO: Make this a bitfield if you need to
const char TILE_SOLIDITY[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0};

initialize()
{
    char i;

    disp_off();
    reset_satb();
    satb_update();
    cls();
    set_xres(256);
    set_screen_size(SCR_SIZE_64x32);
    ad_reset();
    reset_satb();

    cd_loadvram(IMAGE_OVERLAY, AVA_SECTOR_OFFSET, AVA_VRAM, AVA_SIZE_IN_BYTES);
    load_palette(16, avapal, 1);
    load_level_data(current_level);

    load_room();
}

load_level_data(char level)
{
    int vram_offset;
    char tiledata[STARBASE_SIZE_IN_BYTES];

    cd_loaddata(IMAGE_OVERLAY, STARBASE_SECTOR_OFFSET, tiledata, STARBASE_SIZE_IN_BYTES);

    set_tile_data(tiledata, 16, palette_ref, 16);
    load_palette(1, tilepal1, 1);
    load_tile(0x2000);

    set_font_pal(0);
    set_font_color(4, 0);
    load_default_font();

    // Each level data takes up two offsets
    cd_loaddata(4, 2*level, tiles, 3000);

    vram_offset = AVA_VRAM + AVA_SIZE_IN_BYTES;
    populate_enemies_vram(vram_offset, tiles + 2049);
}

init_ava_sprite()
{
    spr_set(0);
    spr_pal(0);
    spr_pri(1);
    spr_show();

    spr_set(1);
    spr_pal(0);
    spr_pri(1);
    spr_show();
}

wait_for_sync(char cycles) {
    char i;
    for (i = 0; i < cycles; i++) {
        draw_objects();
        vsync();
    }
}

draw_ava(char moving, int x, int y)
{
    char frame, frame_offset;
    char ctrl_flags = SZ_16x16;
    int dx, dy; // display x, display y
    int osx, osy;
    osx = sx;
    osy = sy;

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
        scroll_enemies(osx - sx, osy - sy);
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

    spr_set(1);
    spr_x(dx);
    spr_y(dy);
    spr_pattern(0x5000 + (2 * frame * SPR_SIZE_16x16) + SPR_SIZE_16x16);
    spr_ctrl(FLIP_MAS | SIZE_MAS, ctrl_flags);

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
        wait_for_sync(1);
        timer++;

        draw_ava(1, x, y);
        satb_update();
        wait_for_sync(1);
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

const char DEATH_FRAMES[] = {8, 9, 10};
kill_ava()
{
    char i;
    for (i = 0; i < 3; i++)
    {
        spr_set(0);
        spr_pattern(0x5000 + (2 * DEATH_FRAMES[i] * SPR_SIZE_16x16));
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_set(1);
        spr_pattern(0x5000 + (2 * DEATH_FRAMES[i] * SPR_SIZE_16x16) + SPR_SIZE_16x16);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        satb_update();
        wait_for_sync(4);
    }
    spr_set(0);
    spr_hide();
    spr_set(1);
    spr_hide();

    load_room();
}

const char WIN_FRAMES[] = {0, 0, 11, 12, 13, 13, 13, 14, 14, 15};
win_ava()
{
    char i;
    for (i = 0; i < 10; i++)
    {
        spr_set(0);
        spr_pattern(0x5000 + (2 * WIN_FRAMES[i] * SPR_SIZE_16x16));
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_set(1);
        spr_pattern(0x5000 + (2 * WIN_FRAMES[i] * SPR_SIZE_16x16) + SPR_SIZE_16x16);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        satb_update();
        wait_for_sync(4);
    }
    spr_set(0);
    spr_hide();
    spr_set(1);
    spr_hide();
    satb_update();
    wait_for_sync(8);
    
    // TODO: Have this move to what's next rather than reset
    //load_room();
}

load_room()
{
    int i;
    char err, x, y, type, facing, delx, dely;

    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }

    timer = 0;
    ava_facing = DOWN;

    ava_x = tiles[2048];
    ava_y = tiles[2049];
    
    init_object();
    init_enemy();
    i = 2049;
    for (;;)
    {
        x = tiles[++i];
        if (x == 255) {
            break;
        }
        y = tiles[++i];
        type = tiles[++i];
        facing = tiles[++i];
        delx = tiles[++i];
        dely = tiles[++i];
        create_enemy(type, x, y, facing, delx, dely);
    }

    for (;;)
    {
        x = tiles[++i];
        if (x == 255) {
            break;
        }
        y = tiles[++i];
        type = tiles[++i];
        create_object(type, x, y);
    }

    set_map_data(tiles, 64, 32);
    init_ava_sprite();
    draw_ava(0, ava_x * 16, ava_y * 16);
    disp_on();
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
    char joyt;

    initialize();

    for (;;)
    {
        wait_for_sync(1);
        draw_enemies(0);
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
        if (joyt & JOY_I)
        {
            update_enemies();
        }

        if (joyt & JOY_II)
        {
            win_ava();
        }

        if (update_objects())
        {
            win_ava();
        }
    }
}
