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

initialize()
{
    cd_loadvram(IMAGE_OVERLAY, AVA_FACE_SECTOR_OFFSET, FACE_VRAM, AVA_FACE_SIZE);
    cd_loadvram(IMAGE_OVERLAY, FRAMES_SECTOR_OFFSET, FRAME_VRAM, FRAMES_SIZE);
    cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
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

write_text(char x, char y, char *text)
{
    char i;
    int vaddr, parsedtext[100];
    vaddr = vram_addr(x, y);
    i = 0;
    for (;;)
    {
        if (text[i] == 0)
        {
            break;
        }
        parsedtext[i] = (text[i] << 1) + (FONT_VRAM >> 4) - 64;
        i++;
    }
    load_vram(vaddr, parsedtext, i);

    vaddr = vram_addr(x, y + 1);
    i = 0;
    for (;;)
    {
        if (text[i] == 0)
        {
            break;
        }
        parsedtext[i] = (text[i] << 1) + (FONT_VRAM >> 4) - 63;
        i++;
    }
    load_vram(vaddr, parsedtext, i);
}

#define LEVEL_GROUND 56
draw_person(char face, char x_start)
{
    char i, x, y;
    i = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 2; x++)
        {
            spr_set(i);
            spr_y(LEVEL_GROUND + (y << 4));
            spr_x((((int)(x_start + x)) << 4));
            spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
            spr_pattern(FACE_VRAM + (face * 6 * SPR_SIZE_16x16) + (i * SPR_SIZE_16x16));
            spr_pal(16);
            spr_pri(1);
            spr_show();
            i++;
        }
}

main()
{
    char joyt;

    initialize();

    draw_background();
    draw_frame();
    write_text(3, 17, "AVA: Wow! What a great day! Nothing");
    write_text(3, 19, "could go wrong today!");
    draw_person(3, 5);
    satb_update();

    vsync();
    for (;;)
    {
        vsync();

        joyt = joytrg(0);

        if (joyt & JOY_RUN)
        {
            victory = 1;
            cd_execoverlay(GOVERNOR_OVERLAY);
        }
    }
}
