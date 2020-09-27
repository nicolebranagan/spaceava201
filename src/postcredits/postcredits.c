/*
	Governor. Handles the flow of the game.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#define FONT_VRAM 0x0800
#define AVASIDE_VRAM (FONT_VRAM + _8X8_SIZE)
#define LILYSIDE_VRAM (AVASIDE_VRAM + AVASIDE_SIZE)

#define ASCII_ZERO 48
#define SPR_SIZE_16x16 0x40

#define BACKUP_RAM_NAME "\0\0SPACE AVA"

#incbin(fontpal, "palettes/8x8.pal");
#incbin(avasidepal, "palettes/avaside.pal");
#incbin(lilysidepal, "palettes/lilyside.pal");

char timer;

initialize()
{
    char i;
    timer = 0;

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
    //cd_loadvram(IMAGE_OVERLAY, _8X8_SECTOR_OFFSET, FONT_VRAM, _8X8_SIZE);
    cd_loadvram(IMAGE_OVERLAY, AVASIDE_SECTOR_OFFSET, AVASIDE_VRAM, AVASIDE_SIZE);
    cd_loadvram(IMAGE_OVERLAY, LILYSIDE_SECTOR_OFFSET, LILYSIDE_VRAM, LILYSIDE_SIZE);

    load_palette(0, fontpal, 1);
    load_palette(16, avasidepal, 1);
    load_palette(17, lilysidepal, 1);

    reset_satb();
}

#define LEVEL_GROUND 100
init_sprites()
{
    spr_set(0);
    spr_x(0);
    spr_y(LEVEL_GROUND);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(AVASIDE_VRAM);
    spr_pal(0);
    spr_pri(1);
    spr_show();

    spr_set(1);
    spr_x(256);
    spr_y(LEVEL_GROUND - 16);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(LILYSIDE_VRAM);
    spr_pal(1);
    spr_pri(1);
    spr_show();

    spr_set(2);
    spr_x(256);
    spr_y(LEVEL_GROUND);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pattern(LILYSIDE_VRAM + SPR_SIZE_16x16);
    spr_pal(1);
    spr_pri(1);
    spr_show();

    satb_update();
}

const char AVA_FRAMES[] = {0, 1, 0, 2};
const char LILY1_FRAMES[] = {0, 2, 0, 2};
const char LILY2_FRAMES[] = {1, 3, 1, 3};

update_sprites()
{
    spr_set(0);
    spr_x(0 + (timer >> 1));
    spr_pattern(AVASIDE_VRAM + AVA_FRAMES[((timer >> 3) & 0x03)] * SPR_SIZE_16x16);

    spr_set(1);
    spr_x(256 - (timer >> 1));
    spr_pattern(LILYSIDE_VRAM + LILY1_FRAMES[((timer >> 4) & 0x03)] * SPR_SIZE_16x16);

    spr_set(2);
    spr_x(256 - (timer >> 1));
    spr_pattern(LILYSIDE_VRAM + LILY2_FRAMES[((timer >> 3) & 0x03)] * SPR_SIZE_16x16);

    satb_update();
}

stand_forward()
{
    spr_set(0);
    spr_pattern(AVASIDE_VRAM + 31 * SPR_SIZE_16x16);

    spr_set(1);
    spr_pattern(LILYSIDE_VRAM + 4 * SPR_SIZE_16x16);

    spr_set(2);
    spr_pattern(LILYSIDE_VRAM + 5 * SPR_SIZE_16x16);

    satb_update();
}

lift_arm()
{
    spr_set(0);
    spr_pattern(AVASIDE_VRAM + 30 * SPR_SIZE_16x16);

    spr_set(1);
    spr_pattern(LILYSIDE_VRAM + 6 * SPR_SIZE_16x16);

    spr_set(2);
    spr_pattern(LILYSIDE_VRAM + 7 * SPR_SIZE_16x16);

    if (deaths == 0)
    {
        spr_set(3);
        spr_x(126);
        spr_y(LEVEL_GROUND - 16);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_pattern(LILYSIDE_VRAM + (8 * SPR_SIZE_16x16));
        spr_pal(1);
        spr_pri(1);
        spr_show();
    }

    satb_update();
}

write_text(char y, char *text)
{
    char i, x;
    int vaddr, parsedtext[100];
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

convert_to_text(char *text, int value)
{
    char digit_count;
    char digits[5];

    digits[0] = ASCII_ZERO + (value / 10000);
    digits[1] = ASCII_ZERO + ((value / 1000) % 10);
    digits[2] = ASCII_ZERO + ((value / 100) % 10);
    digits[3] = ASCII_ZERO + ((value / 10) % 10);
    digits[4] = ASCII_ZERO + ((value) % 10);

    if (digits[3] == ASCII_ZERO)
    {
        text[0] = digits[4];
        text[1] = 0;
        return;
    }

    if (digits[2] == ASCII_ZERO)
    {
        text[0] = digits[3];
        text[1] = digits[4];
        text[2] = 0;
        return;
    }

    if (digits[1] == ASCII_ZERO)
    {
        text[0] = digits[2];
        text[1] = digits[3];
        text[2] = digits[4];
        text[3] = 0;
        return;
    }

    if (digits[0] == ASCII_ZERO)
    {
        text[0] = digits[1];
        text[1] = digits[2];
        text[2] = digits[3];
        text[3] = digits[4];
        text[4] = 0;
        return;
    }

    text[0] = digits[0];
    text[1] = digits[1];
    text[2] = digits[2];
    text[3] = digits[3];
    text[4] = digits[4];
    text[5] = 0;
}

main()
{
    char *stepstxt, *deathstxt;
    char joyt;
    
    if (story_mode) {
        cd_execoverlay(LOGO_OVERLAY);
    }

    stepstxt = "Steps:       ";
    deathstxt = "Deaths:       ";
    initialize();

    write_text(4, "Ava and Lily");
    write_text(6, "Will Return Again");

    convert_to_text(stepstxt + 7, steps);
    convert_to_text(deathstxt + 8, deaths);
    write_text(19, stepstxt);
    write_text(21, deathstxt);

    init_sprites();
    satb_update();
    cd_playtrk(TRACK_SPACELESS, TRACK_SPACELESS + 1, CDPLAY_REPEAT);

    for (;;)
    {
        vsync();
        if (timer < 255)
        {
            timer++;
        }
        if (timer < 232)
        {
            update_sprites();
        }
        else if (timer == 233)
        {
            stand_forward();
        }
        else if (timer == 254)
        {
            lift_arm();
        }

        joyt = joytrg(0);

        if (joyt & JOY_RUN)
        {
            if (has_backup_ram)
            {
                bm_delete(BACKUP_RAM_NAME);
            }
            cd_execoverlay(LOGO_OVERLAY);
        }
    }
}