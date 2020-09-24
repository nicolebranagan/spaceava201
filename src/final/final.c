/*
	FINAL LEVEL
*/

#include <huc.h>
#include "images/images.h"

#include "final/classic.h"
#include "final/enemy.c"
#include "final/object.c"
#include "./sfx.c"

#incbin(avapal, "palettes/ava.pal");

#incbin(harshpal1, "palettes/harsh.pal");
#incbin(harshpal2, "palettes/harshout.pal");

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
    0x10,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20,
    0x20};

char tiles[500]; // Placeholder for memory

const char TILE_SOLIDITY[] = {
    0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};

char pal_rotate_step, ava_died;

initialize()
{
    pal_rotate_step = 0;

    ad_reset();
    ad_trans(ADPCM_OVERLAY, EUREKA2_SECTOR_OFFSET, EUREKA2_SECTOR_COUNT, EUREKA_LOC);
    ad_trans(ADPCM_OVERLAY, DIE_SECTOR_OFFSET, DIE_SECTOR_COUNT, DIE_LOC);
    ad_trans(ADPCM_OVERLAY, PHOTON_SECTOR_OFFSET, PHOTON_SECTOR_COUNT, PHOTON_LOC);
    ad_trans(ADPCM_OVERLAY, CANNON_SECTOR_OFFSET, CANNON_SECTOR_COUNT, CANNON_LOC);
    ad_trans(ADPCM_OVERLAY, TILE_SECTOR_OFFSET, TILE_SECTOR_COUNT, TILE_SND_LOC);
    cd_loadvram(IMAGE_OVERLAY, AVA_SECTOR_OFFSET, AVA_VRAM, AVA_SIZE);

    disp_off();
    reset_satb();
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    cls();

    load_palette(16, avapal, 1);
    load_palette(17, avapal + (2 << 5), 1);
    load_level_data();

    load_room();
}

load_level_data()
{
    int vram_offset;
    char tiledata[STARBASE_SIZE + STARROT_SIZE];

    // Offsets, objects, are the second sector of a level
    cd_loaddata(FINAL_DATA_OVERLAY, 1, tiles, 500);

    vram_offset = AVA_VRAM + (AVA_SIZE / 2);
    populate_enemies_vram(vram_offset, tiles + 5);

    load_map_graphics(tiles[0]);

    // Level data is the first sector of the level
    cd_loaddata(FINAL_DATA_OVERLAY, 0, tiledata, 2048);
    set_map_data(tiledata, 16, 16);
    load_map(0, 0, 0, 0, 16, 16);
}

load_map_graphics(char gfx_type)
{
    int vram_offset;
    char tiledata[0];

    set_tile_data(tiledata, 32, palette_ref, 32);
    load_tile(0x2000);

    cd_loadvram(
        IMAGE_OVERLAY,
        HARSH_SECTOR_OFFSET,
        0x2000,
        HARSH_SIZE);
    cd_loadvram(
        IMAGE_OVERLAY,
        HARSHOUT_SECTOR_OFFSET,
        0x2000 + (HARSH_SIZE / 2),
        HARSHOUT_SIZE);

    load_palette(1, harshpal1, 1);
    load_palette(2, harshpal2, 1);
}

init_ava_sprite()
{
    spr_set(TOP_HALF_START);
    spr_pal(0);
    spr_pri(1);
    spr_show();

    spr_set(BOTTOM_HALF_START);
    spr_pal(0);
    spr_pri(1);
    spr_show();

    spr_set(TOP_HALF_START + 1);
    spr_pal(1);
    spr_pri(1);
    spr_show();

    spr_set(BOTTOM_HALF_START + 1);
    spr_pal(1);
    spr_pri(1);
    spr_show();
}

wait_for_sync(char cycles)
{
    char i;
    for (i = 0; i < cycles; i++)
    {
        draw_objects();
        satb_update();
        vsync();
        load_palette(2, harshpal2 + (pal_rotate_step << 5), 1);

        timer++;
        if (!(timer % 16))
            pal_rotate_step = (pal_rotate_step + 1) % 4;
    }
}

draw_ava(char moving, int x, int y, int not_x, int not_y)
{
    char frame, frame_offset;
    char not_frame;
    char ctrl_flags = SZ_16x16;
    char not_ctrl_flags = SZ_16x16;

    switch (ava_facing)
    {
    case DOWN:
        frame = 0;
        not_frame = 5;
        break;
    case UP:
        frame = 5;
        not_frame = 0;
        break;
    case LEFT:
        ctrl_flags = ctrl_flags | FLIP_X;
        frame = 3;
        not_frame = 3;
        break;
    case RIGHT:
        not_ctrl_flags = not_ctrl_flags | FLIP_X;
        frame = 3;
        not_frame = 3;
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
                not_frame = not_frame + 1;
            }
            if (frame_offset == 3)
            {
                frame = frame + 2;
                not_frame = not_frame + 2;
            }
            break;
        case LEFT:
        case RIGHT:
            if ((timer >> 3) & 1)
            {
                frame = frame + 1;
                not_frame = not_frame + 1;
            }
        }
    }

    spr_set(TOP_HALF_START);
    spr_x(x);
    spr_y(y - 16);
    spr_pattern(0x5000 + (2 * frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, ctrl_flags);

    spr_set(BOTTOM_HALF_START);
    spr_x(x);
    spr_y(y);
    spr_pattern(0x5000 + (2 * frame * SPR_SIZE_16x16) + SPR_SIZE_16x16);
    spr_ctrl(FLIP_MAS | SIZE_MAS, ctrl_flags);

    spr_set(TOP_HALF_START + 1);
    spr_x(not_x);
    spr_y(not_y - 16);
    spr_pattern(0x5000 + (2 * not_frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, not_ctrl_flags);

    spr_set(BOTTOM_HALF_START + 1);
    spr_x(not_x);
    spr_y(not_y);
    spr_pattern(0x5000 + (2 * not_frame * SPR_SIZE_16x16) + SPR_SIZE_16x16);
    spr_ctrl(FLIP_MAS | SIZE_MAS, not_ctrl_flags);

    return;
}

move_ava(signed char delx, signed char dely)
{
    char i;
    int x, y, nx, ny;

    x = ava_x * 16;
    y = ava_y * 16;
    nx = not_ava_x * 16;
    ny = not_ava_y * 16;

    for (i = 0; i < 8; i++)
    {
        x = x + delx + delx;
        y = y + dely + dely;
        nx = nx - delx - delx;
        ny = ny - dely - dely;

        draw_ava(1, x, y, nx, ny);
        wait_for_sync(1);

        draw_ava(1, x, y, nx, ny);
        wait_for_sync(1);
    }

    ava_x = ava_x + delx;
    ava_y = ava_y + dely;
    not_ava_x = not_ava_x - delx;
    not_ava_y = not_ava_y - dely;

    draw_ava(0, ava_x * 16, ava_y * 16, not_ava_x * 16, not_ava_y * 16);
}

const char DEATH_FRAMES[] = {0, 0, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 10, 10};
kill_ava()
{
    char i;

    sfx_play(DIE_LOC, DIE_SIZE, 14);
    for (i = 0; i < 16; i++)
    {
        spr_set(TOP_HALF_START);
        spr_pattern(0x5000 + (2 * DEATH_FRAMES[i] * SPR_SIZE_16x16));
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_set(BOTTOM_HALF_START);
        spr_pattern(0x5000 + (2 * DEATH_FRAMES[i] * SPR_SIZE_16x16) + SPR_SIZE_16x16);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_set(TOP_HALF_START + 1);
        spr_pattern(0x5000 + (2 * DEATH_FRAMES[i] * SPR_SIZE_16x16));
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_set(BOTTOM_HALF_START + 1);
        spr_pattern(0x5000 + (2 * DEATH_FRAMES[i] * SPR_SIZE_16x16) + SPR_SIZE_16x16);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        wait_for_sync(4);
    }
    spr_set(TOP_HALF_START);
    spr_hide();
    spr_set(BOTTOM_HALF_START);
    spr_hide();
    spr_set(TOP_HALF_START + 1);
    spr_hide();
    spr_set(BOTTOM_HALF_START + 1);
    spr_hide();

    load_room();
    ava_died = 1;
}

const char WIN_FRAMES[] = {0, 0, 11, 12, 13, 13, 13, 14, 14, 15};
win_ava()
{
    char i;
    cd_reset();
    sfx_play(EUREKA_LOC, EUREKA_SIZE, 15);

    wait_for_sync(64);

    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();
    vsync(); // Deliberately not wait_for_sync

    // Return to governor
    cd_execoverlay(GOVERNOR_OVERLAY);
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

    ava_x = tiles[2];
    ava_y = tiles[3];

    not_ava_x = tiles[4];
    not_ava_y = tiles[5];

    init_object();
    init_enemy();
    i = 5;
    for (;;)
    {
        x = tiles[++i];
        if (x == 255)
        {
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
        if (x == 255)
        {
            break;
        }
        y = tiles[++i];
        type = tiles[++i];
        create_object(type, x, y);
    }

    init_ava_sprite();
    draw_ava(0, ava_x * 16, ava_y * 16, not_ava_x * 16, not_ava_y * 16);
    disp_on();
}

char is_solid(char x, char y, char is_ava)
{
    char tile, solidity;

    if (is_ava)
    {
        if ((x == ava_x && y == ava_y) || (x == not_ava_x && y == not_ava_y))
        {
            return 1;
        }
    }

    for (i = 0; i < object_count; i++)
    {
        if (!objects[i].active || objects[i].type != OBJ_TILE)
        {
            continue;
        }
        if (objects[i].xpos == x && objects[i].ypos == y)
        {
            update_tile(i);
            return 0;
        }
    }

    if (!is_ava)
    {
        for (i = 0; i < enemy_count; i++)
        {
            if (enemies[i].active && enemies[i].x == x && enemies[i].y == y)
            {
                return 1;
            }
        }
        for (i = 0; i < object_count; i++)
        {
            if (objects[i].active && objects[i].xpos == x && objects[i].ypos == y)
            {
                return 1;
            }
        }
    }

    tile = map_get_tile(x, y);
    solidity = TILE_SOLIDITY[tile];

    if (solidity == 0)
    {
        return 1;
    }

    for (i = 0; i < object_count; i++)
    {
        if (objects[i].type == OBJ_BOX && x == objects[i].xpos && y == objects[i].ypos)
        {
            if (!is_ava)
            {
                return 1;
            }
            return move_box(i);
        }
    }

    return 0;
}

main()
{
    char joyt;

    initialize();
    cd_playtrk(TRACK_SCANDAL_NULLITY, TRACK_SCANDAL_NULLITY + 1, CDPLAY_REPEAT);

    for (;;)
    {
        wait_for_sync(1);
        draw_enemies(0);
        ava_died = 0;

        joyt = joytrg(0);
        if (
            joyt & JOY_UP &&
            !is_solid(ava_x, ava_y - 1, 1) &&
            !is_solid(not_ava_x, not_ava_y + 1, 1))
        {
            ava_facing = UP;
            move_ava(0, -1);
            if (!ava_died)
                update_enemies();
        }
        if (joyt & JOY_DOWN &&
            !is_solid(ava_x, ava_y + 1, 1) &&
            !is_solid(not_ava_x, not_ava_y - 1, 1))
        {
            ava_facing = DOWN;
            move_ava(0, 1);
            if (!ava_died)
                update_enemies();
        }
        if (joyt & JOY_LEFT &&
            !is_solid(ava_x - 1, ava_y, 1) &&
            !is_solid(not_ava_x + 1, not_ava_y, 1))
        {
            ava_facing = LEFT;
            move_ava(-1, 0);
            if (!ava_died)
                update_enemies();
        }
        if (joyt & JOY_RIGHT &&
            !is_solid(ava_x + 1, ava_y, 1) &&
            !is_solid(not_ava_x - 1, not_ava_y, 1))
        {
            ava_facing = RIGHT;
            move_ava(1, 0);
            if (!ava_died)
                update_enemies();
        }
        if (joyt & JOY_SLCT)
        {
            kill_ava();
        }
        if (joyt & JOY_RUN)
        {
            cd_pause();
            for (;;)
            {
                vsync(); // Deliberately not wait_for_sync
                joyt = joytrg(0);
                if (joyt & JOY_RUN)
                {
                    cd_unpause();
                    break;
                }
            }
        }

        update_objects();

        if (ava_y == 7 && not_ava_y == 7 &&
            (ava_x == 7 || not_ava_x == 7) && (ava_x == 8 || not_ava_x == 8))
        {
            win_ava();
        }
    }
}
