/*
	Classic style Space Ava grid-based gameplay
*/

#include <huc.h>
#include "images/images.h"

#include "classic/classic.h"
#include "classic/enemy.c"
#include "classic/object.c"

#incbin(avapal, "palettes/ava.pal");
#incbin(tilepal1, "palettes/starbase.pal");
#incbin(tilepal2, "palettes/starrot.pal");

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

// TODO: Make this a bitfield if you need to
const char TILE_SOLIDITY[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const char MUSIC_TO_CD_TRACK[] = {
    TRACK_SPACEFUL,
    TRACK_IMPOSSIBLY_BOSSY
};

char pal_rotate_step;

initialize()
{
    pal_rotate_step = 0;

    ad_trans(ADPCM_OVERLAY, EUREKA_SECTOR_OFFSET, EUREKA_SECTOR_COUNT, EUREKA_LOC);
    ad_trans(ADPCM_OVERLAY, DIE_SECTOR_OFFSET, DIE_SECTOR_COUNT, DIE_LOC);
    ad_trans(ADPCM_OVERLAY, PHOTON_SECTOR_OFFSET, PHOTON_SECTOR_COUNT, PHOTON_LOC);
    ad_trans(ADPCM_OVERLAY, CANNON_SECTOR_OFFSET, CANNON_SECTOR_COUNT, CANNON_LOC);
    ad_trans(ADPCM_OVERLAY, MINIWILHELM_SECTOR_OFFSET, MINIWILHELM_SECTOR_COUNT, WILHELM_LOC);
    cd_loadvram(IMAGE_OVERLAY, AVA_SECTOR_OFFSET, AVA_VRAM, AVA_SIZE);

    disp_off();
    reset_satb();
    satb_update();
    set_xres(256);
    set_screen_size(SCR_SIZE_64x32);
    cls();
    ad_reset();

    load_palette(16, avapal, 1);
    load_level_data(current_level);

    load_room();
}

load_level_data(char level)
{
    int vram_offset;
    char tiledata[STARBASE_SIZE + STARROT_SIZE];

    cd_loaddata(
        IMAGE_OVERLAY,
        STARBASE_SECTOR_OFFSET,
        tiledata,
        STARBASE_SIZE);
    cd_loaddata(
        IMAGE_OVERLAY,
        STARROT_SECTOR_OFFSET,
        tiledata + STARROT_SIZE,
        STARROT_SIZE);

    set_tile_data(tiledata, 32, palette_ref, 32);
    load_palette(1, tilepal1, 1);
    load_palette(2, tilepal2, 1);
    load_tile(0x2000);

    // Level data is the first sector of the level
    cd_loaddata(CLASSIC_DATA_OVERLAY, 2 * level, tiledata, 2048);
    set_map_data(tiledata, 64, 32);

    cd_loaddata(CLASSIC_DATA_OVERLAY, (2 * level) + 1, tiles, 500);

    vram_offset = AVA_VRAM + (AVA_SIZE / 2);
    populate_enemies_vram(vram_offset, tiles + 3);
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

wait_for_sync(char cycles)
{
    char i;
    for (i = 0; i < cycles; i++)
    {
        draw_objects();
        load_palette(2, tilepal2 + (pal_rotate_step << 5), 1);
        vsync();
        timer++;
        if (!(timer % 16))
            pal_rotate_step = (pal_rotate_step + 1) % 4;
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

        draw_ava(1, x, y);
        satb_update();
        wait_for_sync(1);
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

const char DEATH_FRAMES[] = {0, 0, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 8, 9, 10, 10};
kill_ava()
{
    char i;

    if (ad_stat())
    {
        ad_stop();
    }
    ad_play(DIE_LOC, DIE_SIZE, 14, 0);
    for (i = 0; i < 16; i++)
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
    cd_reset();
    if (ad_stat())
    {
        ad_stop();
    }
    ad_play(EUREKA_LOC, EUREKA_SIZE, 15, 0);

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

    init_object();
    init_enemy();
    i = 3;
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
    char joyt, track;

    initialize();
    track = MUSIC_TO_CD_TRACK[tiles[1]];
    cd_playtrk(track, track + 1, CDPLAY_REPEAT);

    for (;;)
    {
        wait_for_sync(1);
        draw_enemies(0);
        satb_update();

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
            cd_playtrk(TRACK_CHIME_2020, TRACK_CHIME_2020 + 1, CDPLAY_REPEAT);
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
