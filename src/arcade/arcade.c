/*
	Initializes Arcade Card
*/

#include <huc.h>
#include "cd.h"
#include "./images/images.h"

#incbin(retropal, "palettes/retrofont.pal");
#incbin(harshartpal, "palettes/harshart.pal");
#incbin(harshartbat, "bats/harshart-bg.bin");
#incbin(retro_facepal, "palettes/retro_face.pal");

#define BACKDROP_WIDTH 16 * 2
#define BACKDROP_HEIGHT 5 * 2 // In units of 8x8 tiles
#define XTOP 5
#define YLEFT 3
#define BACKDROP_VRAM 0x1000
#define FONT_VRAM 0x2000
#define FACE_VRAM (FONT_VRAM + (RETROFONT_SIZE / 2))
#define SPR_SIZE_16x16 0x40

initialize()
{
    char y, background;
    int addr;
    cls();
    load_palette(0, retropal, 1);
    set_xres(256);
    cd_loadvram(IMAGE_OVERLAY, RETROFONT_SECTOR_OFFSET, FONT_VRAM, RETROFONT_SIZE);
    cd_loadvram(IMAGE_OVERLAY, HARSHART_SECTOR_OFFSET, BACKDROP_VRAM, HARSHART_SIZE);
    cd_loadvram(IMAGE_OVERLAY, RETRO_FACE_SECTOR_OFFSET, FACE_VRAM, RETRO_FACE_SIZE);

    for (y = 0; y < BACKDROP_HEIGHT; y++)
    {
        addr = vram_addr(XTOP, YLEFT + y);
        load_vram(addr, harshartbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
    }
    load_palette(1, harshartpal, 1);
    load_palette(16, retro_facepal, 1);
}

#define LEVEL_GROUND 56
draw_person(char slot, char face, char x_start)
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
            spr_pattern(FACE_VRAM + (face * 6 * SPR_SIZE_16x16) + (j * SPR_SIZE_16x16));
            spr_pal(16);
            spr_pri(1);
            spr_show();
            i++;
            j++;
        }
    satb_update();
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
        zero_string[i] = FONT_VRAM / 16;
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

main()
{
    initialize();

    draw_person(0, 5, 6);
    draw_person(1, 8, 8);
    draw_person(2, 19, 3);
    draw_person(3, 20, 11);
    write_text("AVA: Wow, Lily, you got the\nArcade Card?\nLILY: That's right! Let's \nfill it with data!"); for (;;)
    {
        vsync();
    }
}