/*
	Title Screen.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#incbin(titlepal, "palettes/titles.pal");
#incbin(logopal, "palettes/logo.pal");
#incbin(titlebat, "bats/title-bg.bin")

#define TITLE_VRAM 0x1000
#define LOGO_VRAM (TITLE_VRAM + (TITLES_SIZE / 2))

char timer;
char done;

initialize()
{
    char i;
    done = 0;
    timer = 0;

    // Clear global state
    current_level = 255;
    governor_step = 255;

    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();

    disp_off();
    cd_loadvram(IMAGE_OVERLAY, TITLES_SECTOR_OFFSET, TITLE_VRAM, TITLES_SIZE);
    cd_loadvram(IMAGE_OVERLAY, LOGO_SECTOR_OFFSET, LOGO_VRAM, LOGO_SIZE);
    load_vram(0, titlebat, 0x700);
    load_palette(1, titlepal, 1);
    load_palette(16, logopal, 1);
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    scroll(0, 0, 0, 0, 223, 0xC0);
    disp_on();
}

draw_logo()
{
    char i;
    for (i = 0; i < 8; i++)
    {
        spr_set(i);
        spr_x(256 + (i << 5));
        spr_y(16);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_32x32);
        spr_show();
        spr_pal(0);
        spr_pri(1);
        spr_pattern(LOGO_VRAM + (i << 8));
    }
}

main()
{
    unsigned char joyt;
    char i;

    initialize();
    draw_logo();
    satb_update();

    for (;;)
    {
        timer++;
        if (!done)
        {
            for (i = 0; i < 8; i++)
            {
                spr_set(i);
                spr_x(256 - (timer << 2) + (i << 5));
            }
            if (timer == 64)
            {
                done = 1;
                timer = 0;
            }
        }
        else
        {
            load_palette(16, logopal + (((timer >> 4) % 4) << 5), 1);
        }
        satb_update();
        vsync();

		joyt = joytrg(0);

        if (joyt & JOY_STRT)
        {
            cls();
            vsync();
            cd_execoverlay(GOVERNOR_OVERLAY);
        }
    }
}
