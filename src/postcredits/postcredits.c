/*
	Governor. Handles the flow of the game.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#define FONT_VRAM 0x0800
#define ASCII_ZERO 48
#incbin(fontpal, "palettes/8x8.pal");

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

    stepstxt = "Steps:       ";
    deathstxt = "Deaths:       ";
    initialize();

    write_text(4, "Ava and Lily");
    write_text(6, "Will Return Again");

    convert_to_text(stepstxt + 7, steps);
    convert_to_text(deathstxt + 8, deaths);
    write_text(19, stepstxt);
    write_text(21, deathstxt);

    for (;;)
    {
        vsync();
    }
}