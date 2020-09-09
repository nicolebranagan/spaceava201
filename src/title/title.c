/*
	Title Screen.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"
#include "./adpcm/adpcm.h"

#incbin(titlepal, "palettes/titles.pal");
#incbin(logopal, "palettes/logo.pal");
#incbin(titlebat, "bats/title-bg.bin");
#incbin(buttonspal, "palettes/buttons.pal");

#define TITLE_VRAM 0x1000
#define LOGO_VRAM (TITLE_VRAM + (TITLES_SIZE / 2))
#define BUTTONS_VRAM (LOGO_VRAM + (LOGO_SIZE / 2))

#define BACKUP_RAM_NAME "\0\0SPACE AVA"
#define CONTINUE 255
#define NEW_GAME 254
#define LEVEL_SELECT 253

char timer;
char done;
char selectedButton;
char hasNoSavedData;
char unlockedLevelSelect;
char codePoint;
int longTimer;

initialize()
{
    char i;
    done = 0;
    timer = 0;
    longTimer = 0;
    unlockedLevelSelect = 0;
    codePoint = 0;

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
    ad_trans(ADPCM_OVERLAY, MINIWILHELM_SECTOR_OFFSET, MINIWILHELM_SECTOR_COUNT, 0);
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
    char i, size;
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

#define BUTTON_LS_OFF 20
#define BUTTON_LS_ON 26

draw_level_select_button(char sprdex, char button_identity, char x, char y)
{
    char i, size;
    for (i = 0; i < 6; i++)
    {
        spr_set(i + sprdex);
        spr_x(x + (i << 4));
        spr_y(y);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_show();
        spr_pal((button_identity == BUTTON_LS_ON) ? 2 : 1);
        spr_pri(1);
        spr_pattern(BUTTONS_VRAM + ((i + button_identity) * 0x40));
    }
}

// migi shita ue migi migi shita migi migi ue ue shita shita hidari migi hidari migi
const char LEVEL_SELECT_CODE[] = {
    JOY_RIGHT,
    JOY_DOWN,
    JOY_UP,
    JOY_RIGHT,
    JOY_RIGHT,
    JOY_DOWN,
    JOY_RIGHT,
    JOY_RIGHT,
    JOY_UP,
    JOY_UP,
    JOY_DOWN,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_LEFT,
    JOY_RIGHT};

unlock_level_select(char joyt)
{
    if (unlockedLevelSelect)
    {
        return;
    }

    if (joyt & LEVEL_SELECT_CODE[codePoint])
    {
        codePoint++;
    }
    else if (joyt)
    {
        codePoint = 0;
    }

    if (codePoint == 16)
    {
        unlockedLevelSelect = 1;
        selectedButton = 2;
        ad_play(0, MINIWILHELM_SIZE, 14, 0);
    }
}

#define BUTTON_Y 128
#define BUTTON_1_X 16
#define BUTTON_2_X 176

const char SELECTED_BUTTON_TO_ACTION[] = {
    NEW_GAME,
    CONTINUE,
    LEVEL_SELECT};

main()
{
    unsigned char joyt;
    char i;

    initialize();
    draw_logo();
    satb_update();
    cd_playtrk(TRACK_EVEN_MORE_SPACELESS, TRACK_EVEN_MORE_SPACELESS + 1, CDPLAY_NORMAL);
    for (;;)
    {
        longTimer++;
        if (!done && longTimer < 130)
        {
        }
        else if (!done)
        {
            timer++;
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
            timer++;
            load_palette(16, logopal + (((timer >> 4) % 4) << 5), 1);
            load_palette(18, buttonspal + (((timer >> 4) % 4) << 5), 1);

            draw_button(8, selectedButton == 0 ? BUTTON_NEW_ON : BUTTON_NEW_OFF, BUTTON_1_X, BUTTON_Y);
            draw_button(16, hasNoSavedData ? BUTTON_CON_DIS : (selectedButton == 1 ? BUTTON_CON_ON : BUTTON_CON_OFF), BUTTON_2_X, BUTTON_Y);

            if (unlockedLevelSelect)
            {
                draw_level_select_button(20, (selectedButton == 2 ? BUTTON_LS_ON : BUTTON_LS_OFF), 80, BUTTON_Y - 32);
            }
        }

        satb_update();
        vsync();

        joyt = joytrg(0);

        unlock_level_select(joyt);
        if (joyt)
        {
            longTimer = 0;
        }

        if (joyt & JOY_STRT || joyt & JOY_I)
        {
            cls();
            vsync();
            governor_step = SELECTED_BUTTON_TO_ACTION[selectedButton];
            cd_execoverlay(GOVERNOR_OVERLAY);
        }

        if (unlockedLevelSelect)
        {

            if (joyt & JOY_UP)
            {
                selectedButton = 2;
            }
            if (selectedButton == 2 && joyt & JOY_DOWN)
            {
                selectedButton = 0;
            }
        }

        if (!hasNoSavedData && (joyt & JOY_SLCT || joyt & JOY_LEFT || joyt & JOY_RIGHT))
        {
            selectedButton = (selectedButton + 1) & 1;
        }

        if (longTimer == 3600)
        {
            cd_execoverlay(LOGO_OVERLAY);
        }
    }
}
