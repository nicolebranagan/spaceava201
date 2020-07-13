/*
    Cutscenes
*/

#include <huc.h>
#include "./images/images.h"
#include "cd.h"
#include "./story/chirp.c"

#incbin(framepal, "palettes/frames.pal");

#incbin(ava_facepal, "palettes/ava_face.pal");
#incbin(cindy_facepal, "palettes/cindy_face.pal");
#incbin(nelehu_facepal, "palettes/nelehu_face.pal");
#incbin(bob_facepal, "palettes/bob_face.pal");
#incbin(goon_facepal, "palettes/goon_face.pal");

#incbin(starbasepal, "palettes/stardrop.pal");

#incbin(starbasebat, "bats/starbase-bg.bin")
#incbin(chipbasebat, "bats/basechip-bg.bin")
#incbin(emptybat, "bats/empty-bg.bin")

#define FONT_VRAM 0x1800
#define FACE_VRAM (FONT_VRAM + BIZCAT_SIZE)
#define SPR_SIZE_16x16 0x40

#define FRAME_VRAM 0x0800
#define BACKDROP_VRAM 0x1000

char timer;
int pointer_to_data;
char script[2048];
char has_next_command;

const char TRACK_MAPPING[] = {
    TRACK_BALLAD,
    TRACK_EVANSSENSE,
    TRACK_CHIME_2020};

initialize()
{
    cd_loaddata(STORY_DATA_OVERLAY, current_level, script, 2048);
    build_cast();
    cls(FONT_VRAM / 16);
    cd_loadvram(IMAGE_OVERLAY, FRAMES_SECTOR_OFFSET, FRAME_VRAM, FRAMES_SIZE);
    cd_loadvram(IMAGE_OVERLAY, BIZCAT_SECTOR_OFFSET, FONT_VRAM, BIZCAT_SIZE);
    scroll(0, 0, 0, 0, 223, 0xC0);
    reset_satb();
    satb_update();
    set_xres(336);
    set_screen_size(SCR_SIZE_64x32);
    cls(FONT_VRAM / 16);
    timer = 0;
}

#define MAX_CAST 5
int face_vram[MAX_CAST];
char palettes[MAX_CAST];

build_cast()
{
    char current_face;
    char current_palette;
    char index;
    int vram;
    index = 0;
    pointer_to_data = 0;
    vram = FACE_VRAM;
    current_palette = 16;
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
            load_palette(current_palette, ava_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, AVA_FACE_SECTOR_OFFSET, vram, AVA_FACE_SIZE);
            break;

        case 2: // Cindy
            load_palette(current_palette, cindy_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, CINDY_FACE_SECTOR_OFFSET, vram, CINDY_FACE_SIZE);
            break;

        case 3: // Nelehu
            load_palette(current_palette, nelehu_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, NELEHU_FACE_SECTOR_OFFSET, vram, NELEHU_FACE_SIZE);
            break;

        case 4: // Bob
            load_palette(current_palette, bob_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, BOB_FACE_SECTOR_OFFSET, vram, BOB_FACE_SIZE);
            break;

        case 5: // Quantum Goon
            load_palette(current_palette, goon_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, GOON_FACE_SECTOR_OFFSET, vram, GOON_FACE_SIZE);
            break;
        }
        face_vram[index] = vram;
        vram = vram + (AVA_FACE_SIZE / 2);
        index++;
        pointer_to_data++;
    }
}

#define BACKDROP_WIDTH 16 * 2
#define BACKDROP_HEIGHT 5 * 2 // In units of 8x8 tiles
#define XTOP 5
#define YLEFT 3

draw_background(char index)
{
    char y, background;
    int addr;

    switch (index)
    {
    case 0:
    {
        load_palette(1, starbasepal, 1);
        cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, starbasebat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        break;
    }
    case 1:
    {
        load_palette(1, starbasepal, 1);
        cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, chipbasebat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        break;
    }
    case 2:
    {
        load_palette(1, starbasepal, 1);
        cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, emptybat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        break;
    }
    }
}

#define FRAME_START ((FRAME_VRAM) >> 4)
draw_frame(char frame)
{
    char x, y, frame_start;
    int addr;
    int data[BACKDROP_WIDTH + 4];

    load_palette(0, framepal + (frame << 5), 1);
    frame_start = FRAME_START + (frame << 5);

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 1;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 8;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 9;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 4;
            continue;
        }
        data[x] = frame_start + 5;
    }
    addr = vram_addr(XTOP - 2, YLEFT - 2);
    load_vram(addr, data, BACKDROP_WIDTH + 4);

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start + 2;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 3;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 10;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 11;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 6;
            continue;
        }
        data[x] = frame_start + 7;
    }
    addr = vram_addr(XTOP - 2, YLEFT - 1);
    load_vram(addr, data, BACKDROP_WIDTH + 4);

    for (y = YLEFT; y < (YLEFT + BACKDROP_HEIGHT); y++)
    {
        data[0] = frame_start + (y % 2 ? 12 : 14);
        data[1] = frame_start + (y % 2 ? 13 : 15);
        addr = vram_addr(XTOP - 2, y);
        load_vram(addr, data, 2);

        data[0] = frame_start + (y % 2 ? 16 : 18);
        data[1] = frame_start + (y % 2 ? 17 : 19);
        addr = vram_addr(XTOP + BACKDROP_WIDTH, y);
        load_vram(addr, data, 2);
    }

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start + 20;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 21;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 28;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 29;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 24;
            continue;
        }
        data[x] = frame_start + 25;
    }
    addr = vram_addr(XTOP - 2, YLEFT + BACKDROP_HEIGHT);
    load_vram(addr, data, BACKDROP_WIDTH + 4);
    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start + 22;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 23;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 30;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 31;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 26;
            continue;
        }
        data[x] = frame_start + 27;
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
        zero_string[i] = 0x0180;
    }
    vaddr = vram_addr(0, TEXT_Y);
    load_vram(vaddr, zero_string, BLOCK_TO_CLEAR);
}

write_char(char x, char y, char character, char chirp_type)
{
    int parsed[1], vaddr;
    vaddr = vram_addr(x, y);
    parsed[0] = (character << 1) + (FONT_VRAM >> 4) - 64;
    load_vram(vaddr, parsed, 1);
    vaddr = vram_addr(x, y + 1);
    (parsed[0])++;
    load_vram(vaddr, parsed, 1);
    chirp(chirp_type);
}

int write_text(char *text, char chirp_type)
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

        write_char(x, y, text[i], chirp_type);
        x++;
        i++;
        len++;
        vsync();
        chirp(0);
    }
    return len;
}

#define LEVEL_GROUND 56
draw_person(char slot, char cast_index, char face, char x_start)
{
    char i, j, x, y;
    i = slot * 6;
    j = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 2; x++)
        {
            spr_set(i);
            spr_y(LEVEL_GROUND + (y << 4));
            spr_x((((int)(x_start + x)) << 4));
            spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
            spr_pattern(face_vram[cast_index] + (face * 6 * SPR_SIZE_16x16) + (j * SPR_SIZE_16x16));
            spr_pal(palettes[cast_index]);
            spr_pri(1);
            spr_show();
            i++;
            j++;
        }
    satb_update();
}

delete_person(char slot)
{
    char i;
    for (i = 0; i < 6; i++)
    {
        spr_set((slot * 6) + i);
        spr_hide();
    }
    satb_update();
}

draw_block(char more)
{
    int parsed[1], vaddr;
    vaddr = vram_addr(39, 25);
    parsed[0] = (timer & 32) ? ((127 << 1) + (FONT_VRAM >> 4) - (more ? 64 : 63)) : 0x0180;
    load_vram(vaddr, parsed, 1);
}

#define CMD_SHOW_SPRITE 1
#define CMD_SHOW_TEXT 2
#define CMD_SHOW_BACKGROUND 3
#define CMD_PLAY_MUSIC 4
#define CMD_STOP_MUSIC 5
#define CMD_SHOW_FRAME 6
#define CMD_DELETE_SPRITE 7

perform_command()
{
    char current_command, chirp_type;
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
        chirp_type = script[pointer_to_data + 1];
        text_len = write_text(script + pointer_to_data + 2, chirp_type);
        pointer_to_data += (text_len + 2);
        break;
    case CMD_SHOW_BACKGROUND:
        draw_background(script[pointer_to_data + 1]);
        pointer_to_data += 2;
        goto loop;
        break;
    case CMD_PLAY_MUSIC:
        chirp_type = TRACK_MAPPING[script[pointer_to_data + 1]];
        cd_playtrk(chirp_type, chirp_type + 1, CDPLAY_REPEAT);
        pointer_to_data += 2;
        goto loop;
        break;
    case CMD_STOP_MUSIC:
        cd_reset();
        pointer_to_data += 1;
        goto loop;
        break;
    case CMD_SHOW_FRAME:
        draw_frame(script[pointer_to_data + 1]);
        pointer_to_data += 2;
        goto loop;
        break;
    case CMD_DELETE_SPRITE:
        delete_person(script[pointer_to_data + 1]);
        pointer_to_data += 2;
        goto loop;
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

done()
{
    char i;
    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();
    cls(0x0180);
    cd_execoverlay(GOVERNOR_OVERLAY);
}

main()
{
    char joyt;

    initialize();

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
            done();
        }

        if (joyt & JOY_I || joyt & JOY_II)
        {
            if (has_next_command)
            {
                perform_command();
            }
            else
            {
                done();
            }
        }
    }
}
