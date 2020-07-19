/*
    Cutscenes
*/

#include <huc.h>

#include "./mirror/const.h"
#include "./images/images.h"
#include "cd.h"

#define SYSTEM_VRAM 0x1000
#define CURSOR_VRAM (SYSTEM_VRAM + (MIRRORSYS_SIZE / 2))
#define LASER_VRAM (CURSOR_VRAM + (CURSORS_SIZE / 2))
#define FACE_VRAM (LASER_VRAM + (LASERS_SIZE / 2))
#define FONT_VRAM (FACE_VRAM + (AVASML_SIZE / 2))

#define TOP_Y 32
#define TOP_X 32
#define GRID_WIDTH 9
#define GRID_HEIGHT 10
#define PALETTE_SIZE (GRID_HEIGHT - 1)

#define SPR_SIZE_16x16 0x40

#incbin(systempal, "palettes/mirrorsys.pal");
#incbin(cursorpal, "palettes/cursors.pal");
#incbin(laserpal, "palettes/lasers.pal");
#incbin(avasmlpal, "palettes/avasml.pal");

#incbin(systembat, "bats/mirrorsys.bin");

char x, y, timer, joytimer, holding;

char palette[PALETTE_SIZE];
char grid[GRID_WIDTH * GRID_HEIGHT];

struct photon
{
    char facing, x, y, type, active, frame;
};

char photon_count;
struct photon photons[MAX_PHOTONS];

initialize()
{
    x = 0;
    y = 0;
    timer = 0;
    joytimer = 0;
    set_xres(256);
    set_screen_size(SCR_SIZE_32x64);
    scroll(0, 0, 0, 0, 223, 0xC0);
    cls();

    cd_loadvram(IMAGE_OVERLAY, SHADE8X8_SECTOR_OFFSET, FONT_VRAM, SHADE8X8_SIZE);
    cd_loadvram(IMAGE_OVERLAY, MIRRORSYS_SECTOR_OFFSET, SYSTEM_VRAM, MIRRORSYS_SIZE);
    cd_loadvram(IMAGE_OVERLAY, CURSORS_SECTOR_OFFSET, CURSOR_VRAM, CURSORS_SIZE);
    cd_loadvram(IMAGE_OVERLAY, LASERS_SECTOR_OFFSET, LASER_VRAM, LASERS_SIZE);
    cd_loadvram(IMAGE_OVERLAY, AVASML_SECTOR_OFFSET, FACE_VRAM, AVASML_SIZE);

    load_palette(1, systempal, 1);
    load_palette(2, avasmlpal, 1);
    load_palette(16, cursorpal, 1);
    load_palette(17, laserpal, 1);
    load_vram(0, systembat, 24 * 16 * 4);
}

reset_grid()
{
    char i;
    for (i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
    {
        grid[i] = SPACE_EMPTY;
    }

    for (i = 0; i < PALETTE_SIZE; i++)
    {
        palette[i] = SPACE_EMPTY;
    }

    palette[0] = SPACE_RIGHT_LEFT_MIRROR;

    grid[GRID_WIDTH + 4] = SPACE_RIGHT_LEFT_SOLMIR;
    grid[GRID_WIDTH + 6] = SPACE_ANTIPHOTON;

    grid[GRID_WIDTH + GRID_WIDTH + GRID_WIDTH + 2] = SPACE_PHOTON;

    holding = SPACE_EMPTY;
}

draw_cursor()
{
    char draw_x;
    char i;

    if (x == GRID_WIDTH && y == (GRID_HEIGHT - 1))
    {
        spr_set(PLAYER_SPRITE_1);
        spr_y(192);
        spr_x(208);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_pattern(CURSOR_VRAM + (SPR_SIZE_16x16 * (holding ? 6 : 2)));
        spr_pal(0);
        spr_pri(1);
        spr_show();

        spr_set(PLAYER_SPRITE_2);
        spr_y(192);
        spr_x(224);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_pattern(CURSOR_VRAM + ((holding ? 7 : 3) * SPR_SIZE_16x16));
        spr_pal(0);
        spr_pri(1);
        spr_show();
        spr_set(2);
        spr_hide();
        return;
    }

    spr_set(0);

    if (x < GRID_WIDTH)
    {
        draw_x = TOP_X + (x << 4);
        i = x + (y * GRID_WIDTH);
        spr_pattern(
            (grid[i] == SPACE_RIGHT_LEFT_MIRROR || grid[i] == SPACE_LEFT_RIGHT_MIRROR) ? (CURSOR_VRAM + (SPR_SIZE_16x16 << 2))
                                                                                       : ((timer >> 4 & 1) ? CURSOR_VRAM : (CURSOR_VRAM + SPR_SIZE_16x16)));
    }
    else
    {
        draw_x = 216; // Palette
        spr_pattern(
            (palette[y] == SPACE_RIGHT_LEFT_MIRROR || grid[i] == SPACE_LEFT_RIGHT_MIRROR) ? (CURSOR_VRAM + (SPR_SIZE_16x16 << 2))
                                                                                          : ((timer >> 4 & 1) ? CURSOR_VRAM : (CURSOR_VRAM + SPR_SIZE_16x16)));
    }

    spr_y(TOP_Y + (y << 4));
    spr_x(draw_x);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(0);
    spr_pri(1);
    spr_show();
    spr_set(1);
    spr_hide();

    if (holding != SPACE_EMPTY)
    {
        spr_set(0);
        spr_pattern(CURSOR_VRAM + (5 * SPR_SIZE_16x16));
        draw_mirror(2, draw_x, TOP_Y + (y << 4), 0, holding == SPACE_LEFT_RIGHT_MIRROR);
    }
    else
    {
        spr_set(2);
        spr_hide();
    }
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

draw_mirror(char sprdex, char x, char y, char solid, char flip)
{
    spr_set(sprdex);
    spr_x(x);
    spr_y(y);
    spr_ctrl(FLIP_MAS | SIZE_MAS, flip ? (SZ_16x16 | FLIP_X) : SZ_16x16);
    spr_pattern(LASER_VRAM + (SPR_SIZE_16x16 << 4) + (solid ? SPR_SIZE_16x16 : 0));
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
            draw_beam(grid_sprite, i, LASER_VRAM + 4 * SPR_SIZE_16x16);
            break;
        }
        case SPACE_LEFT_RIGHT_MIRROR:
        {
            draw_mirror(grid_sprite, TOP_X + ((i % GRID_WIDTH) << 4), TOP_Y + ((i / GRID_WIDTH) << 4), 0, 1);
            break;
        }
        case SPACE_RIGHT_LEFT_MIRROR:
        {
            draw_mirror(grid_sprite, TOP_X + ((i % GRID_WIDTH) << 4), TOP_Y + ((i / GRID_WIDTH) << 4), 0, 0);
            break;
        }
        case SPACE_LEFT_RIGHT_SOLMIR:
        {
            draw_mirror(grid_sprite, TOP_X + ((i % GRID_WIDTH) << 4), TOP_Y + ((i / GRID_WIDTH) << 4), 1, 1);
            break;
        }
        case SPACE_RIGHT_LEFT_SOLMIR:
        {
            draw_mirror(grid_sprite, TOP_X + ((i % GRID_WIDTH) << 4), TOP_Y + ((i / GRID_WIDTH) << 4), 1, 0);
            break;
        }
        }

        grid_sprite++;
    }

    for (i = 0; i < PALETTE_SIZE; i++)
    {
        if (palette[i] == SPACE_EMPTY)
        {
            continue;
        }
        draw_mirror(grid_sprite, 216, TOP_Y + (i << 4), 0, palette[i] == SPACE_LEFT_RIGHT_MIRROR);
        grid_sprite++;
    }

    for (i = grid_sprite; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
}

#define FRAME_START ((FACE_VRAM >> 4) + 0x2000)
const char FACE_CYCLE[] = {
    0, 1, 4, 5,
    2, 3, 6, 7};

draw_lower_face(char idx)
{
    int addr, frame_start;
    int data[8];
    char i, j;

    frame_start = FRAME_START + (idx << 4);
    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < 8; i++)
        {
            data[i] = frame_start + (j << 3) + FACE_CYCLE[i];
        }
        addr = vram_addr(3, 34 + (j * 2));
        load_vram(addr, data, 4);
        addr = vram_addr(3, 34 + (j * 2) + 1);
        load_vram(addr, data + 4, 4);
    }

    scroll(0, 0, 0, 0, 223 - 56, 0xC0);
    scroll(1, 0, 256, 223 - 56, 223, 0xC0);
}

write_text(char x, char y, char *text)
{
    char i;
    int vaddr, parsedtext[100];
    i = 0;
    for (;;)
    {
        if (text[i] == 0)
        {
            break;
        }
        parsedtext[i] = text[i] + (FONT_VRAM / 16) + 0x1000;
        i++;
    }
    vaddr = vram_addr(x, y);
    load_vram(vaddr, parsedtext, i);
}

wait_for_input()
{
    char joyt;
    for (;;)
    {
        vsync();
        joyt = joytrg(0);

        if (joyt)
        {
            return;
        }
    }
}

draw_grid_frame()
{
    char i, spr_index;
    int vram_offset;
    draw_grid();

    spr_index = PHOTON_SPR_START;
    for (i = 0; i < photon_count; i++)
    {
        if (!photons[i].active)
        {
            spr_set(spr_index);
            spr_hide();
            spr_index++;
            continue;
        }
        if (photons[i].type == SPACE_ANNIHILATION)
        {
            photons[i].frame--;
            if (photons[i].frame == 21)
            {
                photons[i].active = 0;
            }
            spr_set(spr_index);
            spr_x(photons[i].x);
            spr_y(photons[i].y);
            spr_pal(1);
            spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
            spr_pri(1);
            spr_pattern(LASER_VRAM + (SPR_SIZE_16x16 * photons[i].frame));
            spr_show();
            spr_index++;
            continue;
        }
        vram_offset = (photons[i].type == SPACE_PHOTON) ? (LASER_VRAM + 8 * SPR_SIZE_16x16) : (LASER_VRAM + 12 * SPR_SIZE_16x16);
        spr_set(spr_index);
        spr_x(photons[i].x);
        spr_y(photons[i].y);
        spr_pal(1);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_pri(1);
        spr_pattern(vram_offset + (SPR_SIZE_16x16 * ((timer & 63) >> 4)));
        spr_show();
        spr_index++;
    }
    satb_update();
}

char run_grid()
{
    char i, j, count, x, y, allInactive;
    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();
    vsync();

    photon_count = 0;
    for (i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
    {
        if (grid[i] == SPACE_PHOTON || grid[i] == SPACE_ANTIPHOTON)
        {
            photons[photon_count].type = grid[i];
            photons[photon_count].facing = grid[i] == SPACE_PHOTON ? RIGHT : LEFT;
            photons[photon_count].x = TOP_X + ((i % GRID_WIDTH) << 4);
            photons[photon_count].y = TOP_Y + ((i / GRID_WIDTH) << 4);
            photons[photon_count].active = 1;
            photons[photon_count].frame = 32;
            photon_count++;
            grid[i] = SPACE_EMPTY;
        }
    }

    count = 0;

    for (;;)
    {
        timer++;
        draw_grid_frame();
        vsync();
        for (i = 0; i < photon_count; i++)
        {
            switch (photons[i].facing)
            {
            case UP:
                photons[i].y -= 1;
                break;
            case DOWN:
                photons[i].y += 1;
                break;
            case LEFT:
                photons[i].x -= 1;
                break;
            case RIGHT:
                photons[i].x += 1;
                break;
            }
        }
        count++;
        if (count == 16)
        {
            count = 0;
            for (i = 0; i < photon_count; i++)
            {
                if (!photons[i].active || photons[i].type == SPACE_ANNIHILATION)
                {
                    continue;
                }
                x = (photons[i].x - TOP_X) >> 4;
                y = (photons[i].y - TOP_Y) >> 4;

                switch (grid[x + GRID_WIDTH * y])
                {
                case SPACE_RIGHT_LEFT_SOLMIR:
                case SPACE_RIGHT_LEFT_MIRROR:
                {
                    switch (photons[i].facing)
                    {
                    case UP:
                        photons[i].facing = RIGHT;
                        break;
                    case DOWN:
                        photons[i].facing = LEFT;
                        break;
                    case LEFT:
                        photons[i].facing = DOWN;
                        break;
                    case RIGHT:
                        photons[i].facing = UP;
                        break;
                    }
                    break;
                }
                case SPACE_LEFT_RIGHT_SOLMIR:
                case SPACE_LEFT_RIGHT_MIRROR:
                {
                    switch (photons[i].facing)
                    {
                    case UP:
                        photons[i].facing = LEFT;
                        break;
                    case DOWN:
                        photons[i].facing = RIGHT;
                        break;
                    case LEFT:
                        photons[i].facing = UP;
                        break;
                    case RIGHT:
                        photons[i].facing = DOWN;
                        break;
                    }
                    break;
                }
                case SPACE_EMPTY:
                {
                    for (j = (i + 1); j < photon_count; j++)
                    {
                        if (!photons[j].active)
                        {
                            continue;
                        }
                        if (
                            (x == ((photons[j].x - TOP_X) >> 4)) &&
                            (y == ((photons[j].y - TOP_Y) >> 4)))
                        {
                            photons[j].active = 0;
                            photons[i].type = SPACE_ANNIHILATION;
                            photons[i].x = (x << 4) + TOP_X;
                            photons[i].y = (y << 4) + TOP_Y;
                            photons[i].facing = DIR_NONE;
                            break;
                        }
                    }
                    break;
                }
                }
            }

            allInactive = 1;
            for (i = 0; i < photon_count; i++)
            {
                if (photons[i].active)
                {
                    allInactive = 0;
                }

                if (photons[i].x < TOP_X ||
                    photons[i].y < TOP_Y ||
                    photons[i].x > (TOP_X + ((GRID_WIDTH - 1) * 16)) ||
                    photons[i].y > (TOP_Y + (GRID_HEIGHT * 16)))
                {
                    return 0;
                }
            }
            if (allInactive)
            {
                return 1;
            }
        }
    }
}

action()
{
    char i;

    if (x == GRID_WIDTH && y == (GRID_HEIGHT - 1))
    {
        if (holding)
        {
            draw_lower_face(1);
            spr_set(0);
            spr_hide();
            spr_set(1);
            spr_hide();
            satb_update();
            write_text(8, 35, "I should put down    ");
            write_text(8, 36, "the mirror first.    ");
            wait_for_input();
            scroll_disable(1);
            scroll(0, 0, 0, 0, 223, 0xC0);
            return;
        }
        if (run_grid())
        {
            // win
            draw_lower_face(0);
            write_text(8, 35, "Wow! This is turning ");
            write_text(8, 36, "out okay!            ");
            wait_for_input();
            cls();
            scroll(0, 0, 0, 0, 223, 0xC0);
            scroll_disable(1);
            cd_execoverlay(GOVERNOR_OVERLAY);
            return;
        }
        else
        {
            // lose
            draw_lower_face(1);
            write_text(8, 35, "I was so close!      ");
            write_text(8, 36, "Maybe try again?     ");
            wait_for_input();
            for (i = 0; i < 64; i++)
            {
                spr_set(i);
                spr_hide();
            }
            satb_update();
            scroll_disable(1);
            scroll(0, 0, 0, 0, 223, 0xC0);
            reset_grid();
            return;
        }
    }

    if (x < GRID_WIDTH)
    {
        i = grid[x + (y * GRID_WIDTH)];
    }
    else
    {
        i = palette[y];
    }

    if ((holding != SPACE_EMPTY) && i == SPACE_EMPTY)
    {
        if (x < GRID_WIDTH)
        {
            grid[x + (y * GRID_WIDTH)] = holding;
        }
        else
        {
            palette[y] = holding;
        }
        holding = SPACE_EMPTY;
        return;
    }

    if ((holding == SPACE_EMPTY) && (i == SPACE_RIGHT_LEFT_MIRROR || i == SPACE_LEFT_RIGHT_MIRROR))
    {

        holding = i;
        if (x < GRID_WIDTH)
        {
            grid[x + (y * GRID_WIDTH)] = SPACE_EMPTY;
        }
        else
        {
            palette[y] = SPACE_EMPTY;
        }
        return;
    }
}

main()
{
    char joyt;

    initialize();
    reset_grid();

    for (;;)
    {
        timer++;
        joytimer++;
        vsync();
        draw_grid();
        draw_cursor();
        satb_update();

        joyt = joytrg(0);

        if ((joyt & JOY_RUN) || (joyt & JOY_I))
        {
            action();
        }

        if (joyt)
        {
            joytimer = 0;
        }

        if (joytimer % 16 == 15)
        {
            joyt = joy(0);
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