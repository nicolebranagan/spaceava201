/*
    Cutscenes
*/

#include <huc.h>
#include "./images/images.h"
#include "cd.h"

#incbin(fontpal, "palettes/bizcat.pal");
#incbin(facepal, "palettes/ava_face.pal");

#define FONT_VRAM 0x4000
#define FACE_VRAM FONT_VRAM + BIZCAT_SIZE
#define SPR_SIZE_16x16 0x40

initialize()
{
    cd_loadvram(IMAGE_OVERLAY, AVA_FACE_SECTOR_OFFSET, FACE_VRAM, AVA_FACE_SIZE);

    cls();
    cd_loadvram(IMAGE_OVERLAY, BIZCAT_SECTOR_OFFSET, FONT_VRAM, BIZCAT_SIZE);
    scroll(0, 0, 0, 0, 223, 0xC0);
    reset_satb();
    satb_update();
    set_xres(320);
    set_screen_size(SCR_SIZE_64x32);
    load_palette(16, facepal, 1);
    load_palette(0, fontpal, 1);
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
        parsedtext[i] = (text[i] << 1) + (FONT_VRAM / 16) - 64;
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
        parsedtext[i] = (text[i] << 1) + (FONT_VRAM / 16) - 63;
        i++;
    }
    load_vram(vaddr, parsedtext, i);
}

#define LEVEL_GROUND 48
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
    initialize();
    write_text(3, 16, "AVA: Wow! What a great day! Nothing");
    write_text(3, 18, "could go wrong today!");
    draw_person(3, 5);
    satb_update();
    vsync();
    for (;;)
    {
        vsync();
    }
}
