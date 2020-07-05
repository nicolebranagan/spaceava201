/*
    Cutscenes
*/

#include <huc.h>
#include "./images/images.h"
#include "cd.h"

#incbin(fontpal, "palettes/frames.pal");
#incbin(facepal, "palettes/ava_face.pal");
#incbin(backdroppal, "palettes/stardrop.pal");
#incbin(backdrop, "bats/starbase.bin");

#define FONT_VRAM 0x4000
#define FACE_VRAM (FONT_VRAM + BIZCAT_SIZE)
#define SPR_SIZE_16x16 0x40

#define FRAME_VRAM 0x2000
#define BACKDROP_VRAM 0x2800

char timer;
int pointer_to_data;
char script[2048];
char has_next_command;

initialize()
{
    cd_loaddata(STORY_DATA_OVERLAY, current_level, script, 2048);
    cd_loadvram(IMAGE_OVERLAY, FRAMES_SECTOR_OFFSET, FRAME_VRAM, FRAMES_SIZE);
    cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
    build_cast();
    cls();
    cd_loadvram(IMAGE_OVERLAY, BIZCAT_SECTOR_OFFSET, FONT_VRAM, BIZCAT_SIZE);
    scroll(0, 0, 0, 0, 223, 0xC0);
    reset_satb();
    satb_update();
    set_xres(320);
    set_screen_size(SCR_SIZE_64x32);
    load_palette(16, facepal, 1);
    load_palette(0, fontpal, 1);
    load_palette(1, backdroppal, 1);
    timer = 0;
}

#define MAX_CAST 8
int face_vram[MAX_CAST];

build_cast()
{
    char current_face;
    char index;
    int vram;
    index = 0;
    pointer_to_data = 0;
    vram = FACE_VRAM;
    for (;;)
    {
        current_face = script[pointer_to_data];
        if (current_face == 255)
        {
            pointer_to_data++;
            has_next_command = 1;
            return;
        }
        switch (current_face)
        {
        case 1: // Ava
            cd_loadvram(IMAGE_OVERLAY, AVA_FACE_SECTOR_OFFSET, vram, AVA_FACE_SIZE);
            face_vram[index] = vram;
            vram = vram + AVA_FACE_SIZE;
            break;
        }
        index++;
        pointer_to_data++;
    }
}

#define BACKDROP_WIDTH 16 * 2
#define BACKDROP_HEIGHT 5 * 2 // In units of 8x8 tiles
#define XTOP 5
#define YLEFT 3
draw_background()
{
    char y;
    int addr;

    for (y = 0; y < BACKDROP_HEIGHT; y++)
    {
        addr = vram_addr(XTOP, YLEFT + y);
        load_vram(addr, backdrop + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
    }
}

#define FRAME_START ((FRAME_VRAM) >> 4)
draw_frame()
{
    char x, y;
    int addr;
    int data[BACKDROP_WIDTH + 4];

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = FRAME_START;
            continue;
        }
        if (x == 1)
        {
            data[x] = FRAME_START + 1;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = FRAME_START + 8;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = FRAME_START + 9;
            continue;
        }
        if (x % 2)
        {
            data[x] = FRAME_START + 4;
            continue;
        }
        data[x] = FRAME_START + 5;
    }
    addr = vram_addr(XTOP - 2, YLEFT - 2);
    load_vram(addr, data, BACKDROP_WIDTH + 4);

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = FRAME_START + 2;
            continue;
        }
        if (x == 1)
        {
            data[x] = FRAME_START + 3;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = FRAME_START + 10;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = FRAME_START + 11;
            continue;
        }
        if (x % 2)
        {
            data[x] = FRAME_START + 6;
            continue;
        }
        data[x] = FRAME_START + 7;
    }
    addr = vram_addr(XTOP - 2, YLEFT - 1);
    load_vram(addr, data, BACKDROP_WIDTH + 4);

    for (y = YLEFT; y < (YLEFT + BACKDROP_HEIGHT); y++)
    {
        data[0] = FRAME_START + (y % 2 ? 12 : 14);
        data[1] = FRAME_START + (y % 2 ? 13 : 15);
        addr = vram_addr(XTOP - 2, y);
        load_vram(addr, data, 2);

        data[0] = FRAME_START + (y % 2 ? 16 : 18);
        data[1] = FRAME_START + (y % 2 ? 17 : 19);
        addr = vram_addr(XTOP + BACKDROP_WIDTH, y);
        load_vram(addr, data, 2);
    }

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = FRAME_START + 20;
            continue;
        }
        if (x == 1)
        {
            data[x] = FRAME_START + 21;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = FRAME_START + 28;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = FRAME_START + 29;
            continue;
        }
        if (x % 2)
        {
            data[x] = FRAME_START + 24;
            continue;
        }
        data[x] = FRAME_START + 25;
    }
    addr = vram_addr(XTOP - 2, YLEFT + BACKDROP_HEIGHT);
    load_vram(addr, data, BACKDROP_WIDTH + 4);
    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = FRAME_START + 22;
            continue;
        }
        if (x == 1)
        {
            data[x] = FRAME_START + 23;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = FRAME_START + 30;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = FRAME_START + 31;
            continue;
        }
        if (x % 2)
        {
            data[x] = FRAME_START + 26;
            continue;
        }
        data[x] = FRAME_START + 27;
    }
    addr = vram_addr(XTOP - 2, YLEFT + BACKDROP_HEIGHT + 1);
    load_vram(addr, data, BACKDROP_WIDTH + 4);
}

#define TEXT_X 3
#define TEXT_Y 17

#define BLOCK_TO_CLEAR ((32 - TEXT_Y) * 64)
clear_text()
{
    int i;
    int vaddr, zero_string[BLOCK_TO_CLEAR];
    for (i = 0; i < BLOCK_TO_CLEAR; i++)
    {
        zero_string[i] = 0x0080;
    }
    vaddr = vram_addr(0, TEXT_Y);
    load_vram(vaddr, zero_string, BLOCK_TO_CLEAR);
}

write_char(char x, char y, char character)
{
    int parsed[1], vaddr;
    vaddr = vram_addr(x, y);
    parsed[0] = (character << 1) + (FONT_VRAM >> 4) - 64;
    load_vram(vaddr, parsed, 1);
    vaddr = vram_addr(x, y + 1);
    (parsed[0])++;
    load_vram(vaddr, parsed, 1);
}

int write_text(char *text)
{
    char x, y, i;
    int len;
    x = TEXT_X;
    y = TEXT_Y;
    i = 0;
    len = 0;
    clear_text();
    for (;;)
    {
        if (text[i] == 0)
        {
            len++;
            break;
        }
        if (text[i] == 10)
        {
            x = TEXT_X;
            y += 2;
            i++;
            len++;
            continue;
        }

        write_char(x, y, text[i]);
        x++;
        i++;
        len++;
        vsync();
    }
    return len;
}

#define LEVEL_GROUND 56
draw_person(char slot, char cast_index, char face, char x_start)
{
    char i, j, x, y;
    i = slot*6;
    j = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 2; x++)
        {
            spr_set(i);
            spr_y(LEVEL_GROUND + (y << 4));
            spr_x((((int)(x_start + x)) << 4));
            spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
            spr_pattern(face_vram[cast_index] + (face * 6 * SPR_SIZE_16x16) + (j * SPR_SIZE_16x16));
            spr_pal(16);
            spr_pri(1);
            spr_show();
            i++;
            j++;
        }
    satb_update();
}

draw_block(char more)
{
    int parsed[1], vaddr;
    vaddr = vram_addr(39, 25);
    parsed[0] = (timer & 32) ? ((127 << 1) + (FONT_VRAM >> 4) - (more ? 64 : 63)) : 0x0080;
    load_vram(vaddr, parsed, 1);
}

#define CMD_SHOW_SPRITE 1
#define CMD_SHOW_TEXT 2

perform_command()
{
    char current_command;
    int text_len;
loop:
    current_command = script[pointer_to_data];

    switch (current_command)
    {
    case CMD_SHOW_SPRITE:
        draw_person(
            script[pointer_to_data + 1],
            script[pointer_to_data + 2],
            script[pointer_to_data + 3],
            script[pointer_to_data + 4]);
        pointer_to_data += 5;
        goto loop;
        break;
    case CMD_SHOW_TEXT:
        text_len = write_text(script + pointer_to_data + 1);
        pointer_to_data += (text_len + 1);
        break;
    }

    if (script[pointer_to_data] == 255)
    {
        has_next_command = 0;
    }
    else
    {
        has_next_command = 1;
    }
}

main()
{
    char joyt;

    initialize();

    draw_background();
    draw_frame();
    perform_command();
    vsync();
    for (;;)
    {
        vsync();
        timer++;
        draw_block(has_next_command);

        joyt = joytrg(0);

        if (joyt & JOY_RUN)
        {
            victory = 1;
            cd_execoverlay(GOVERNOR_OVERLAY);
        }

        if (joyt & JOY_I || joyt & JOY_II)
        {
            if (has_next_command)
            {
                perform_command();
            }
            else
            {
                victory = 1;
                cd_execoverlay(GOVERNOR_OVERLAY);
            }
        }
    }
}
