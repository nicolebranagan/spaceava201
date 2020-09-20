/*
	CD Error. For when you absolutely, positively, use the wrong system card
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#define FONT_VRAM 0x0800
#incbin(fontpal, "palettes/8x8.pal");
#incbin(nullpal, "palettes/null.pal");

initialize()
{
    char i;

    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();

    disp_off();
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    scroll(0, 0, 0, 0, 223, 0xC0);
    cls();
    disp_on();
    cd_loadvram(IMAGE_OVERLAY, _8X8_SECTOR_OFFSET, FONT_VRAM, _8X8_SIZE);
    load_palette(0, fontpal, 1);

    reset_satb();
}

int parsedtext[100];

write_text(char y, char *text)
{
    char i, x;
    int vaddr;
    i = 0;
    for (;;)
    {
        if (text[i] == 0)
        {
            break;
        }
        parsedtext[i] = text[i] + (FONT_VRAM / 16);
        i++;
    }
    x = 16 - (i >> 1);
    vaddr = vram_addr(x, y);
    load_vram(vaddr, parsedtext, i);
}

main()
{

    initialize();
    vsync();
    cls();
    write_text(2, "In the criminal justice system");

    for (;;)
    {
        vsync();
    }
}