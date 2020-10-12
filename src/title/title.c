/*
	Title Screen.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"
#include "./adpcm/adpcm.h"
#include "./sgx.c"
#include "acd.c"

#incbin(titlepal, "palettes/titles.pal");
#incbin(logopal, "palettes/logo.pal");
#incbin(superpal, "palettes/super.pal");
#incbin(titlebat, "bats/title-bg.bin");
#incbin(buttonspal, "palettes/buttons.pal");
#incbin(superbat, "bats/superbat.bin");
#incbin(nullpal, "palettes/null.pal");

#define TITLE_VRAM 0x1000
#define LOGO_VRAM (TITLE_VRAM + (TITLES_SIZE / 2))
#define BUTTONS_VRAM (LOGO_VRAM + (LOGO_SIZE / 2))

#define BACKUP_RAM_NAME "\0\0SPACE AVA"
#define CONTINUE 255
#define NEW_GAME 254
#define LEVEL_SELECT 253
#define STORY_MODE 252

#define SGX_PAL 4

#define ADPCM_PHOTON 0x8000

char timer;
char done;
char selectedButton;
char hasNoSavedData;
char unlockedLevelSelect;
char unlockedStoryMode;
char codePoint;
char storyCodePoint;
int longTimer;
int scr_y;

char buffer[2048];

initialize()
{
    char i;

    // Clear palettes from intro
    load_palette(0, nullpal, 1);
    load_palette(1, nullpal, 1);
    load_palette(SGX_PAL, nullpal, 1);

    scr_y = 128;
    done = 0;
    timer = 0;
    longTimer = 0;
    unlockedLevelSelect = 0;
    unlockedStoryMode = 0;
    codePoint = 0;
    storyCodePoint = 0;

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
    CD_LOADVRAM(IMAGE_OVERLAY, TITLES_SECTOR_OFFSET, TITLE_VRAM, TITLES_SIZE);
    CD_LOADVRAM(IMAGE_OVERLAY, LOGO_SECTOR_OFFSET, LOGO_VRAM, LOGO_SIZE);
    CD_LOADVRAM(IMAGE_OVERLAY, BUTTONS_SECTOR_OFFSET, BUTTONS_VRAM, BUTTONS_SIZE);
    ad_trans(ADPCM_OVERLAY, MINIWILHELM_SECTOR_OFFSET, MINIWILHELM_SECTOR_COUNT, 0);
    ad_trans(ADPCM_OVERLAY, PHOTON_SECTOR_OFFSET, PHOTON_SECTOR_COUNT, ADPCM_PHOTON);
    load_vram(0, titlebat, 0x700);
    if (is_sgx())
    {
        sgx_init(SCR_SIZE_32x32);
        sgx_disable();
        disp_off();
        for (i = 0; i < SUPER_SECTOR_COUNT; i++)
        {
            cd_loaddata(IMAGE_OVERLAY, SUPER_SECTOR_OFFSET + i, buffer, 2048);
            sgx_load_vram(0x1000 + (1024 * i), buffer, 2048);
        }
        sgx_load_vram(0, superbat, 32 * 32 * 2);
    }
    load_palette(1, titlepal, 1);
    load_palette(16, logopal, 1);
    load_palette(17, buttonspal, 1);
    load_palette(18, buttonspal, 1);
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    scroll(0, 0, 0, 0, 223, 0xC0);
    disp_on();
    if (is_sgx())
    {
        load_palette(SGX_PAL, superpal, 1);
        sgx_init(SCR_SIZE_32x32);
        scroll_sgx(0, scr_y);
        set_color(0, 0x02);
    }
    else
    {
        set_color(0, 0x01);
    }
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

draw_arcade()
{
    char i;

    if (arcade_card_initialized != ACD_INITIALIZED) {
        return;
    }

    for (i = 0; i < 4; i++)
    {
        spr_set(i + 34);
        spr_x(0 + (i << 4));
        spr_y(50);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_show();
        spr_pal(1);
        spr_pri(1);
        spr_pattern(BUTTONS_VRAM + ((44 + i) * 0x40));
    }
}

move_arcade(char new_pos)
{
    char i;
    
    if (arcade_card_initialized != ACD_INITIALIZED) {
        return;
    }

    for (i = 0; i < 4; i++)
    {
        spr_set(i + 34);
        spr_x(new_pos + (i << 4));
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
#define BUTTON_SM_OFF 32
#define BUTTON_SM_ON 38

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
        spr_pal(((button_identity == BUTTON_LS_ON) || (button_identity == BUTTON_SM_ON)) ? 2 : 1);
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

const char STORY_MODE_CODE[] = {
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT};

unlock_story_mode(char joyt)
{
    if (unlockedStoryMode)
    {
        return;
    }

    if (joyt & STORY_MODE_CODE[storyCodePoint])
    {
        storyCodePoint++;
    }
    else if (joyt)
    {
        storyCodePoint = 0;
    }

    if (storyCodePoint == 4)
    {
        unlockedStoryMode = 1;
        selectedButton = 3;
        ad_play(ADPCM_PHOTON, PHOTON_SIZE, 14, 0);
    }
}

#define BUTTON_Y 128
#define BUTTON_1_X 16
#define BUTTON_2_X 176

const char SELECTED_BUTTON_TO_ACTION[] = {
    NEW_GAME,
    CONTINUE,
    LEVEL_SELECT,
    STORY_MODE};

main()
{
    unsigned char joyt;
    char i;

    initialize();
    draw_logo();
    draw_arcade();
    satb_update();
    cd_playtrk(TRACK_EVEN_MORE_SPACELESS, TRACK_EVEN_MORE_SPACELESS + 1, CDPLAY_NORMAL);
    for (;;)
    {
        longTimer++;
        if (scr_y < 264)
        {
            scr_y += 2;
            if (is_sgx())
            {
                scroll_sgx(0, scr_y);
            }
        }
        if (!done)
        {
            timer++;
            for (i = 0; i < 8; i++)
            {
                spr_set(i);
                spr_x(256 - (timer << 2) + (i << 5));
            }
            if (timer < 48)
            {
                move_arcade(timer << 2);
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
            if (unlockedStoryMode)
            {
                draw_level_select_button(26, (selectedButton == 3 ? BUTTON_SM_ON : BUTTON_SM_OFF), 88, BUTTON_Y + 32);
            }
        }

        satb_update();
        vsync();

        joyt = joytrg(0);

        unlock_level_select(joyt);
        unlock_story_mode(joyt);
        if (joyt)
        {
            longTimer = 0;
        }

        if (joyt & JOY_STRT || joyt & JOY_I)
        {
            cls();
            vsync();
            governor_step = SELECTED_BUTTON_TO_ACTION[selectedButton];
            if (is_sgx())
            {
                sgx_disable();
            }
            cd_execoverlay(GOVERNOR_OVERLAY);
        }

        if (unlockedLevelSelect)
        {
            if (selectedButton != 3 && joyt & JOY_UP)
            {
                selectedButton = 2;
                continue;
            }
            if (selectedButton == 2 && joyt & JOY_DOWN)
            {
                selectedButton = 0;
                continue;
            }
        }

        if (unlockedStoryMode)
        {
            if (selectedButton == 3 && joyt & JOY_UP)
            {
                selectedButton = 0;
                continue;
            }
            if (selectedButton != 2 && joyt & JOY_DOWN)
            {
                selectedButton = 3;
                continue;
            }
        }

        if (!hasNoSavedData && (joyt & JOY_SLCT || joyt & JOY_LEFT || joyt & JOY_RIGHT))
        {
            selectedButton = (selectedButton + 1) & 1;
            continue;
        }

        if (longTimer >= 3600)
        {
            if (is_sgx())
            {
                sgx_disable();
            }
            cd_execoverlay(LOGO_OVERLAY);
        }
    }
}
