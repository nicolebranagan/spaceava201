/*
	Governor. Handles the flow of the game.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#define FONT_VRAM 0x4000
#incbin(fontpal, "palettes/8x8.pal");

const char STEP_ORDER[] = {
    STORY_OVERLAY, 0,
    CLASSIC_OVERLAY, 0};

initialize()
{
    char i;
    cls();
    scroll(0, 0, 0, 0, 223, 0xC0);
    reset_satb();
    for(i = 0; i < 64; i++) {
        spr_set(i);
        spr_hide();
    }
    satb_update();
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    load_palette(0, fontpal, 1);

    cd_loadvram(IMAGE_OVERLAY, _8X8_SECTOR_OFFSET, 0x4000, _8X8_SIZE);
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
        parsedtext[i] = text[i] + (FONT_VRAM / 16);
        i++;
    }
    load_vram(vaddr, parsedtext, i);
}

continue_cycle()
{
    char state;
    current_level = STEP_ORDER[(governor_step << 1) + 1];
    cd_execoverlay(STEP_ORDER[governor_step << 1]);
}

main()
{
    initialize();

    write_text(11, 11, "Loading...");
    vsync();
    if (victory)
    {
        victory = 0;
        governor_step++;
    }

    continue_cycle();
    for (;;)
    {
        vsync();
    }
}
