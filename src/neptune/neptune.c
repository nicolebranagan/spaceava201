/*
    Side-scroller style Space Ava gameplay
*/

#include <huc.h>

#include "cd.h"
#include "neptune/neptune.h"

#incbin(avapal, "palettes/avaside.pal");
#incbin(neptunepal, "palettes/neptune.pal");

char ava_x, ava_y, ava_state, step, facing_left;
char allowed_up, allowed_down, allowed_left, allowed_right;

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

    spr_set(POINTER_SPR_UP);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(POINTER_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_hide();

    spr_set(POINTER_SPR_DOWN);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(POINTER_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_hide();

    spr_set(POINTER_SPR_LEFT);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(POINTER_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_hide();

    spr_set(POINTER_SPR_RIGHT);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(POINTER_VRAM);
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

    satb_update();
}

draw_pointers()
{
    int ava_posx, ava_posy;
    ava_posx = ava_x << 4;
    ava_posy = ava_y << 4;

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

draw_ava(int x, char y)
{
    spr_set(0);
    spr_x(x);
    spr_y(y);
    spr_show();
}

is_solid(char x, char y)
{
    return y == 5;
}

ava_update(signed char delx, signed char dely)
{
    char new_x, new_y, old_state;
    int drawx, drawy;

    new_x = ava_x + delx;
    new_y = ava_y + dely;

    old_state = ava_state;
    if ((ava_state == AVA_STATE_FALLING && delx))
    {
        ava_state = AVA_STATE_FALL_NO;
    }

    spr_set(AVA_SPRITE);
    spr_ctrl(
        FLIP_MAS | SIZE_MAS,
        facing_left ? (SZ_16x16 | FLIP_X) : SZ_16x16);
    if (is_solid(new_x, new_y + 1))
    {
        ava_state = AVA_STATE_STANDING;
        spr_pattern(AVA_STANDING);
    }
    else
    {
        if (ava_state != AVA_STATE_FALL_NO)
        {
            ava_state = AVA_STATE_FALLING;
        }
        spr_pattern(AVA_JUMP);
    }

    switch (ava_state)
    {
    case AVA_STATE_STANDING:
    {
        allowed_up = !is_solid(new_x, new_y - 1);
        allowed_down = 0;
        allowed_left = !is_solid(new_x - 1, new_y);
        allowed_right = !is_solid(new_x + 1, new_y);
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
        if (old_state == AVA_STATE_STANDING && delx)
        {
            spr_pattern(AVA_WALK_FRAMES[step]);
        }
        spr_x(drawx);
        spr_y(drawy);
        drawx += delx;
        drawy += dely;
        satb_update();
        vsync();
    }

    ava_x = new_x;
    ava_y = new_y;
    draw_ava(ava_x << 4, ava_y << 4);
    draw_pointers();
    satb_update();
}

main()
{
    char joyt;

    initialize();

    allowed_up = 0;
    allowed_down = 0;
    allowed_right = 0;
    allowed_left = 0;
    facing_left = 0;
    ava_state = AVA_STATE_FALLING;
    ava_x = 2;
    ava_y = 2;
    ava_update(0, 0);

    for (;;)
    {
        vsync();

        joyt = joytrg(0);
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

        satb_update();
    }
}