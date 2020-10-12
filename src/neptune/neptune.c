/*
    Side-scroller style Space Ava gameplay
*/

#include <huc.h>

#include "cd.h"
#include "neptune/neptune.h"
#include "neptune/objects.c"
#include "./sfx.c"
#include "./sgx.c"
#include "acd.c"

#incbin(avapal, "palettes/avaside.pal");
#incbin(neptunepal, "palettes/neptune.pal");
#incbin(sideobjpal, "palettes/objside.pal");
#incbin(sidenmypal, "palettes/sidenmy.pal");
#incbin(nullpal, "palettes/null.pal");
#incbin(neptbg1pal, "palettes/neptbg1.pal")
#incbin(neptbg2pal, "palettes/neptbg2.pal")

#define SGX_VRAM 0x1000
#define SGX_PAL 4

#define DK_LEVEL 11

char tiles[2048];
char objdata[500];
char scroll_bg;

const char TILE_SOLIDITY[] = {
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_SOLID,
    TILE_LADDER,
    TILE_LADDER,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY,
    TILE_EMPTY};

char pal_cycle;

const char PAL_CYCLE[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    6,
    5,
    4,
    3,
    2,
    1};

char joyt;

initialize()
{
    pal_cycle = 0;
    timer = 0;
    scroll_bg = 0;
    //
    load_palette(1, nullpal, 1);
    ad_reset();
    ad_trans(ADPCM_OVERLAY, JUMP_SECTOR_OFFSET, JUMP_SECTOR_COUNT, PCM_JUMP);
    ad_trans(ADPCM_OVERLAY, DIE_SECTOR_OFFSET, DIE_SECTOR_COUNT, PCM_DIE);
    ad_trans(ADPCM_OVERLAY, PHOTON_SECTOR_OFFSET, PHOTON_SECTOR_COUNT, PCM_PHOTON);
    ad_trans(ADPCM_OVERLAY, EUREKA_SECTOR_OFFSET, EUREKA_SECTOR_COUNT, PCM_EUREKA);
    ad_trans(ADPCM_OVERLAY, CANNON_SECTOR_OFFSET, CANNON_SECTOR_COUNT, PCM_CANNON);
    ad_trans(ADPCM_OVERLAY, STOMP_SECTOR_OFFSET, STOMP_SECTOR_COUNT, PCM_STOMP);

    if (is_sgx())
    {
        CD_LOADVRAM(IMAGE_OVERLAY, NEPTSUP_SECTOR_OFFSET, NEPTUNE_VRAM, NEPTSUP_SIZE);
        for (i = 0; i < NEPTBG1_SECTOR_COUNT; i++)
        {
            CD_LOADDATA(IMAGE_OVERLAY, (current_level == DK_LEVEL ? NEPTBG2_SECTOR_OFFSET : NEPTBG1_SECTOR_OFFSET) + i, tiles, 2048);
            sgx_load_vram(SGX_VRAM + (1024 * i), tiles, 2048);
        }
        if (current_level == DK_LEVEL)
        {
            populate_sgx_vram_bg2();
        }
        else
        {
            scroll_bg = 1;
            populate_sgx_vram_bg1();
        }
    }
    else
    {
        CD_LOADVRAM(IMAGE_OVERLAY, NEPTUNE_SECTOR_OFFSET, NEPTUNE_VRAM, NEPTUNE_SIZE);
    }
    CD_LOADVRAM(IMAGE_OVERLAY, AVASIDE_SECTOR_OFFSET, AVA_VRAM, AVASIDE_SIZE);
    CD_LOADVRAM(IMAGE_OVERLAY, OBJSIDE_SECTOR_OFFSET, SIDEOBJ_VRAM, OBJSIDE_SIZE);
    CD_LOADVRAM(IMAGE_OVERLAY, SIDENMY_SECTOR_OFFSET, SIDENMY_VRAM, SIDENMY_SIZE);
    cd_loaddata(CLASSIC_DATA_OVERLAY, (2 * current_level), tiles, 2048);
    cd_loaddata(CLASSIC_DATA_OVERLAY, (2 * current_level + 1), objdata, 500);

    disp_off();
    reset_satb();
    set_xres(256);
    load_palette(0, nullpal, 1);
    set_screen_size(SCR_SIZE_128x64);
    cls();

    if (current_level == DK_LEVEL)
    {
        load_palette(SGX_PAL, neptbg2pal, 1);
    }
    else
    {
        load_palette(SGX_PAL, neptbg1pal, 1);
    }
    load_palette(16, avapal, 1);
    load_palette(SIDEOBJ_PAL, sideobjpal, 1);
    load_palette(SIDENMY_PAL, sidenmypal, 1);
    draw_map();
    init_ava();
    disp_on();

    if (is_sgx())
    {
        sgx_init(current_level == DK_LEVEL ? SCR_SIZE_32x32 : SCR_SIZE_64x32);
        scroll_sgx(0, 0);
    }
}

#define FRAME_START ((SGX_VRAM) >> 4)
#define SGX_ROWS 32
#define SGX_WIDTH 32
populate_sgx_vram_bg1()
{
    char i, j;
    int vaddr, k;
    k = FRAME_START;
    for (i = 0; i < SGX_ROWS; i++)
    {
        unsigned int data[SGX_WIDTH * 2];
        for (j = 0; j < SGX_WIDTH; j++)
        {
            //palette 4
            data[j] = 0x4000 + k;
            k++;
        }
        for (j = 0; j < SGX_WIDTH; j++)
        {
            //palette 4
            data[j + SGX_WIDTH] = 0x4000 + FRAME_START;
        }

        sgx_load_vram((i) * (SGX_WIDTH << 1), data, SGX_WIDTH << 2);
    }
}
populate_sgx_vram_bg2()
{
    char i, j;
    int vaddr, k;
    k = FRAME_START;
    for (i = 0; i < SGX_ROWS; i++)
    {
        unsigned int data[SGX_WIDTH];
        for (j = 0; j < SGX_WIDTH; j++)
        {
            //palette 4
            data[j] = 0x4000 + k;
            k++;
        }

        sgx_load_vram((i) * (SGX_WIDTH), data, SGX_WIDTH << 1);
    }
}

load_room()
{
    allowed_up = 0;
    allowed_down = 0;
    allowed_right = 0;
    allowed_left = 0;
    facing_left = 0;
    ava_state = AVA_STATE_FALLING;

    ava_x = objdata[2];
    ava_y = objdata[3];

    sx = (ava_x * 16) - 128;
    sy = (ava_y * 16) - 128;
    if (sx < 0)
    {
        sx = 0;
    }
    if (sy < 0)
    {
        sy = 0;
    }
    scroll(0, sx, sy, 0, 223, 0xC0);

    init_objects();
    {
        char i, x, y, type, facing, delx, dely;
        i = 3;
        for (;;)
        {
            x = objdata[++i];
            if (x == 255)
            {
                break;
            }
            y = objdata[++i];
            type = objdata[++i];
            facing = objdata[++i];
            delx = objdata[++i];
            dely = objdata[++i];
            create_object(type + OBJ_BLOBBO, x, y, facing);
        }

        for (;;)
        {
            x = objdata[++i];
            if (x == 255)
            {
                break;
            }
            y = objdata[++i];
            type = objdata[++i];
            create_object(type, x, y, 0);
        }
    }

    ava_update(0, 0);
    load_palette(0, neptunepal, 1);
}

#define MAP_BYTE_WIDTH (LEVEL_WIDTH_16x16 * 2)
#define TILE_START ((NEPTUNE_VRAM) >> 4)
draw_map()
{
    char x, y;
    int addr, i;
    int row1bytes[MAP_BYTE_WIDTH];
    int row2bytes[MAP_BYTE_WIDTH];

    i = 0;
    for (y = 0; y < LEVEL_HEIGHT_16x16; y++)
    {
        for (x = 0; x < LEVEL_WIDTH_16x16; x++)
        {
            row1bytes[x << 1] = TILE_START + (tiles[i] << 2) + 0;
            row1bytes[(x << 1) + 1] = TILE_START + (tiles[i] << 2) + 1;
            row2bytes[x << 1] = TILE_START + (tiles[i] << 2) + 2;
            row2bytes[(x << 1) + 1] = TILE_START + (tiles[i] << 2) + 3;
            i++;
        }
        addr = vram_addr(0, y << 1);
        load_vram(addr, row1bytes, MAP_BYTE_WIDTH);
        addr = vram_addr(0, (y << 1) + 1);
        load_vram(addr, row2bytes, MAP_BYTE_WIDTH);
    }
}

void wait_for_sync(char cycles)
{
    char i, pal;
    for (i = 0; i < cycles; i++)
    {
        pal = PAL_CYCLE[pal_cycle];
        load_palette(0, neptunepal + (pal << 5), 1);
        timer++;
        if (!(timer % 16))
        {
            pal_cycle++;
            if (pal_cycle > 13)
            {
                pal_cycle = 0;
            }
        }
        draw_objects();
        satb_update();
        vsync();
        if (scroll_bg)
        {
            scroll_sgx(sx >> 2, 0);
        }
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

    spr_set(POINTER_SPR_UP);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(POINTERU_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_hide();

    spr_set(POINTER_SPR_DOWN);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16 | FLIP_Y);
    spr_pattern(POINTERU_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_hide();

    spr_set(POINTER_SPR_LEFT);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(POINTERL_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_hide();

    spr_set(POINTER_SPR_RIGHT);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16 | FLIP_X);
    spr_pattern(POINTERL_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_hide();
}

hide_pointers()
{
    spr_set(POINTER_SPR_UP);
    spr_hide();
    spr_set(POINTER_SPR_DOWN);
    spr_hide();
    spr_set(POINTER_SPR_LEFT);
    spr_hide();
    spr_set(POINTER_SPR_RIGHT);
    spr_hide();
}

draw_pointers()
{
    int ava_posx, ava_posy;
    ava_posx = (ava_x << 4) - sx;
    ava_posy = (ava_y << 4) - sy;

    spr_set(POINTER_SPR_UP);
    if (allowed_up)
    {
        spr_x(ava_posx);
        spr_y(ava_posy - 16);
        spr_show();
    }
    else
    {
        spr_hide();
    }

    spr_set(POINTER_SPR_DOWN);
    if (allowed_down)
    {
        spr_x(ava_posx);
        spr_y(ava_posy + 16);
        spr_show();
    }
    else
    {
        spr_hide();
    }

    spr_set(POINTER_SPR_LEFT);
    if (allowed_left)
    {
        spr_x(ava_posx - 16);
        spr_y(ava_posy);
        spr_show();
    }
    else
    {
        spr_hide();
    }

    spr_set(POINTER_SPR_RIGHT);
    if (allowed_right)
    {
        spr_x(ava_posx + 16);
        spr_y(ava_posy);
        spr_show();
    }
    else
    {
        spr_hide();
    }
}

draw_ava(int x, int y)
{
    spr_set(0);
    spr_x(x - sx);
    spr_y(y - sy);
    spr_show();
}

char getTileSolidity(char x, char y)
{
    int offset;
    // Trick into doing 16-bit math
    offset = y * LEVEL_WIDTH_16x16;
    offset += x;
    return TILE_SOLIDITY[*(tiles + offset)];
}

char is_empty(char x, char y)
{
    return getTileSolidity(x, y) == TILE_EMPTY;
}

char is_solid(char x, char y)
{
    return getTileSolidity(x, y) == TILE_SOLID;
}

char is_ladder(char x, char y)
{
    return getTileSolidity(x, y) == TILE_LADDER;
}

ava_update(signed char delx, signed char dely)
{
    char new_x, new_y, old_state;
    int drawx, drawy;

    if (delx || dely) {
        steps++;
    }

    new_x = ava_x + delx;
    new_y = ava_y + dely;

    if (new_y > 24)
    {
        kill_ava();
        return;
    }

    old_state = ava_state;
    if ((ava_state == AVA_STATE_FALLING && delx))
    {
        ava_state = AVA_STATE_FALL_NO;
    }

    hide_pointers();
    spr_set(AVA_SPRITE);
    spr_ctrl(
        FLIP_MAS | SIZE_MAS,
        facing_left ? (SZ_16x16 | FLIP_X) : SZ_16x16);
    spr_pattern(AVA_STANDING);

    if (is_ladder(new_x, new_y))
    {
        ava_state = AVA_STATE_ON_LADDER;
    }
    else if (is_solid(new_x, new_y + 1) || is_ladder(new_x, new_y + 1))
    {
        ava_state = AVA_STATE_STANDING;
    }
    else
    {
        if (ava_state != AVA_STATE_FALL_NO)
        {
            ava_state = AVA_STATE_FALLING;
        }
        spr_pattern(AVA_JUMP);
        if (dely < 0)
        {
            sfx_play(PCM_JUMP, JUMP_SIZE, 14);
        }
    }

    if (dely && ((ava_state == AVA_STATE_ON_LADDER) || (old_state == AVA_STATE_ON_LADDER)))
    {
        spr_pattern(AVA_LADDER);
    }

    switch (ava_state)
    {
    case AVA_STATE_STANDING:
    {
        allowed_up = is_empty(new_x, new_y - 1) || is_ladder(new_x, new_y - 1);
        allowed_down = is_ladder(new_x, new_y + 1);
        allowed_left = !is_solid(new_x - 1, new_y);
        allowed_right = !is_solid(new_x + 1, new_y);
        break;
    }
    case AVA_STATE_ON_LADDER:
    {
        allowed_up = is_empty(new_x, new_y - 1) || is_ladder(new_x, new_y - 1);
        allowed_down = is_ladder(new_x, new_y + 1);
        allowed_left = is_empty(new_x - 1, new_y);
        allowed_right = is_empty(new_x + 1, new_y);
        break;
    }
    case AVA_STATE_FALLING:
    {
        allowed_up = 0;
        allowed_down = 1;
        allowed_left = !is_solid(new_x - 1, new_y);
        allowed_right = !is_solid(new_x + 1, new_y);
        break;
    }
    case AVA_STATE_FALL_NO:
    {
        allowed_up = 0;
        allowed_down = 1;
        allowed_left = 0;
        allowed_right = 0;
    }
    default:
        break;
    }

    drawx = ava_x << 4;
    drawy = ava_y << 4;
    for (step = 0; step < 16; step++)
    {

        drawx += delx;
        drawy += dely;
        if (drawx > 128)
        {
            sx = drawx - 128;
        }
        else
        {
            sx = 0;
        }
        if (drawy > 128)
        {
            sy = drawy - 128;
        }
        else
        {
            sy = 0;
        }
        scroll(0, sx, sy, 0, 223, 0xC0);
        draw_ava(drawx, drawy);

        if (dely && ((ava_state == AVA_STATE_ON_LADDER) || (old_state == AVA_STATE_ON_LADDER)))
        {
            spr_ctrl(
                FLIP_MAS | SIZE_MAS,
                (step > 4 && step < 12) ? (SZ_16x16 | FLIP_X) : SZ_16x16);
        }
        else if ((old_state == AVA_STATE_STANDING || old_state == AVA_STATE_ON_LADDER) && delx)
        {
            spr_pattern(AVA_WALK_FRAMES[step]);
        }
        wait_for_sync(1);
    }

    ava_x = new_x;
    ava_y = new_y;
    switch (ava_state)
    {
    case AVA_STATE_ON_LADDER:
        spr_pattern(AVA_LADDER);
        break;
    case AVA_STATE_STANDING:
        spr_pattern(AVA_STANDING);
        break;
    case AVA_STATE_FALLING:
    case AVA_STATE_FALL_NO:
        spr_pattern(AVA_JUMP);
        break;
    }
    draw_ava(ava_x << 4, ava_y << 4);

    if ((delx || dely) && update_objects(delx, dely))
    {
        win_ava();
    }

    draw_pointers();
    wait_for_sync(1);
}

const char WIN_FRAMES[] = {0, 5, 6, 7, 8, 9, 10, 11, 12, 13};
win_ava()
{
    char i;
    cd_reset();
    hide_pointers();

    sfx_play(PCM_EUREKA, EUREKA_SIZE, 15);

    for (i = 0; i < 10; i++)
    {
        spr_set(AVA_SPRITE);
        spr_pattern(AVA_VRAM + (WIN_FRAMES[i] * SPR_SIZE_16x16));
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        wait_for_sync(4);
    }

    spr_set(AVA_SPRITE);
    spr_hide();
    wait_for_sync(8);
    load_palette(0, nullpal, 1);
    load_palette(1, nullpal, 1);

    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();
    // Return to governor
    if (is_sgx()) {
        sgx_disable();
    }
    cd_execoverlay(GOVERNOR_OVERLAY);
}

const char DEATH_FRAMES[] = {0, 0, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22};
kill_ava()
{
    char i;
    deaths++;
    sfx_play(PCM_DIE, DIE_SIZE, 14);

    for (i = 0; i < 16; i++)
    {
        spr_set(AVA_SPRITE);
        spr_pattern(AVA_VRAM + (DEATH_FRAMES[i] * SPR_SIZE_16x16));
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        wait_for_sync(4);
    }
    spr_set(AVA_SPRITE);
    for (i = 0; i < MAX_OBJECT_COUNT; i++)
    {
        spr_set(OBJECT_SPRITE_START + i);
        spr_hide();
    }
    spr_hide();

    load_room();
}

main()
{
    initialize();
    cd_playtrk(TRACK_PARASOL_GENTLY, TRACK_PARASOL_GENTLY + 1, CDPLAY_REPEAT);
    load_room();

    for (;;)
    {
        wait_for_sync(1);

        joyt = joy(0);
        if (((joyt & JOY_UP) || (joyt & JOY_I)) && allowed_up)
        {
            ava_update(0, -1);
        }
        else if ((joyt & JOY_DOWN) && allowed_down)
        {
            ava_update(0, 1);
        }
        else if ((joyt & JOY_LEFT) && allowed_left)
        {
            facing_left = 1;
            ava_update(-1, 0);
        }
        else if ((joyt & JOY_RIGHT) && allowed_right)
        {
            facing_left = 0;
            ava_update(1, 0);
        }

        if (joytrg(0) & JOY_RUN)
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
    }
}