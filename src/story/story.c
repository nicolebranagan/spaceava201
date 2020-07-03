/*
    Cutscenes
*/

#include <huc.h>
#include "./images/images.h"
#include "cd.h"

#incbin(fontpal, "palettes/bizcat.pal");

#define FONT_VRAM 0x4000

initialize()
{
    cls();
    scroll(0, 0, 0, 0, 223, 0xC0);
    reset_satb();
    satb_update();
    set_xres(320);
    set_screen_size(SCR_SIZE_64x32);
    load_palette(0, fontpal, 1);

    cd_loadvram(IMAGE_OVERLAY, BIZCAT_SECTOR_OFFSET, 0x4000, BIZCAT_SIZE);
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

main()
{
    initialize();
    write_text(3, 16, "AVA: Wow! What a great day! Nothing");
    write_text(3, 18, "could go wrong today!");
    vsync();
    for (;;)
    {
        vsync();
    }
}
