/*
	Title Screen.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#incbin(titlepal, "palettes/titles.pal");
#incbin(logopal, "palettes/logo.pal");
#incbin(titlebat, "bats/title-bg.bin");
#incbin(buttonspal, "palettes/buttons.pal");

#define TITLE_VRAM 0x1000
#define LOGO_VRAM (TITLE_VRAM + (TITLES_SIZE / 2))
#define BUTTONS_VRAM (LOGO_VRAM + (LOGO_SIZE / 2))
#define BACKUP_RAM_NAME "\0\0SPACE AVA"

char timer;
char done;
char selectedButton;
char hasNoSavedData;

initialize()
{
    char i;
    done = 0;
    timer = 0;

    if (!bm_check() || !bm_exist(BACKUP_RAM_NAME))
    {
        hasNoSavedData = 1;
        selectedButton = 0;
    }
    else
    {
        hasNoSavedData = 0;
        selectedButton = 1;
    }

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
    cd_loadvram(IMAGE_OVERLAY, BUTTONS_SECTOR_OFFSET, BUTTONS_VRAM, BUTTONS_SIZE);
    load_vram(0, titlebat, 0x700);
    load_palette(1, titlepal, 1);
    load_palette(16, logopal, 1);
    load_palette(17, buttonspal, 1);
    load_palette(18, buttonspal, 1);
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

#define BUTTON_NEW_OFF 0
#define BUTTON_NEW_ON 1
#define BUTTON_CON_OFF 2
#define BUTTON_CON_ON 3
#define BUTTON_CON_DIS 4

draw_button(char sprdex, char button_identity, char x, char y)
{
    char i;
    for (i = 0; i < 4; i++)
    {
        spr_set(i + sprdex);
        spr_x(x + (i << 4));
        spr_y(y);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_show();
        spr_pal((button_identity == BUTTON_NEW_ON || button_identity == BUTTON_CON_ON) ? 2 : 1);
        spr_pri(1);
        spr_pattern(BUTTONS_VRAM + ((i + (((int)button_identity) << 2)) * 0x40));
    }
}

#define BUTTON_Y 128
#define BUTTON_1_X 16
#define BUTTON_2_X 176

main()
{
    unsigned char joyt;
    char i;

    initialize();
    draw_logo();
    satb_update();
    cd_playtrk(TRACK_EVEN_MORE_SPACELESS, TRACK_EVEN_MORE_SPACELESS + 1, CDPLAY_REPEAT);
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
            load_palette(18, buttonspal + (((timer >> 4) % 4) << 5), 1);

            draw_button(8, selectedButton == 0 ? BUTTON_NEW_ON : BUTTON_NEW_OFF, BUTTON_1_X, BUTTON_Y);
            draw_button(16, hasNoSavedData ? BUTTON_CON_DIS : (selectedButton == 0 ? BUTTON_CON_OFF : BUTTON_CON_ON), BUTTON_2_X, BUTTON_Y);
        }
        
        satb_update();
        vsync();

        joyt = joytrg(0);

        if (joyt & JOY_STRT || joyt & JOY_I)
        {
            cls();
            vsync();
            governor_step = selectedButton == 0 ? 254 : 255;
            cd_execoverlay(GOVERNOR_OVERLAY);
        }

        if (!hasNoSavedData && (joyt & JOY_SLCT || joyt & JOY_LEFT || joyt & JOY_RIGHT))
        {
            selectedButton = (selectedButton + 1) & 1;
        }
    }
}
