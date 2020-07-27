/*
	Intro animation
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"
#include "./adpcm/adpcm.h"
#include "./story/chirp.c"

#incbin(fontpal, "palettes/titlefnt.pal");
#incbin(amalghqpal, "palettes/amalghq.pal");
#incbin(amalhqbin, "bats/amalhq-bg.bin");

#define AMALGHQ_VRAM 0x1000
#define FONT_VRAM (AMALGHQ_VRAM + (AMALGHQ_SIZE / 2))

char phase;
char timer;

#define PHASE_INIT 0
#define PHASE_AMALHQ 1
#define PHASE_MEDAL 2

initialize()
{
    char i;
    timer = 255;

    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();
    set_screen_size(SCR_SIZE_64x32);
    scroll(0, 0, 0, 0, 223, 0xC0);
    set_xres(336);
    cls();
    cd_loadvram(IMAGE_OVERLAY, TITLEFNT_SECTOR_OFFSET, FONT_VRAM, TITLEFNT_SIZE);
    cd_loadvram(IMAGE_OVERLAY, AMALGHQ_SECTOR_OFFSET, AMALGHQ_VRAM, AMALGHQ_SIZE);
    load_palette(0, fontpal, 1);
    load_palette(1, amalghqpal, 1);

    prepare_phase(PHASE_INIT);
}

#define BACKDROP_WIDTH 16 * 2
#define BACKDROP_HEIGHT 5 * 2 // In units of 8x8 tiles
#define XTOP 5
#define YLEFT 3

draw_background(char index)
{
    char y, background;
    int addr;

    switch (index)
    {
    case 0:
    {
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, amalhqbin + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        break;
    }
    }
}

write_char(char x, char y, char character, char chirp_type)
{
    int parsed[1], vaddr;
    vaddr = vram_addr(x, y);
    parsed[0] = (character << 1) + (FONT_VRAM >> 4) - 64;
    load_vram(vaddr, parsed, 1);
    vaddr = vram_addr(x, y + 1);
    (parsed[0])++;
    load_vram(vaddr, parsed, 1);
    chirp(chirp_type);
}

#define TEXT_X 3
#define TEXT_Y 17

int write_text(char x, char y, char *text, char chirp_type)
{
    char i;
    int len;
    i = 0;
    len = 0;
    for (;;)
    {
        if (text[i] == 0)
        {
            len++;
            break;
        }
        if (text[i] == 10)
        {
            x = TEXT_X;
            y += 2;
            i++;
            len++;
            continue;
        }

        write_char(x, y, text[i], chirp_type);
        x++;
        i++;
        len++;
        vsync();
        chirp(0);
    }
    return len;
}

#define DEFAULT_CHIRP 76

prepare_phase(char newphase)
{
    switch (newphase)
    {
    case PHASE_INIT:
        timer = 200;
        write_text(13, 9, "Space Year 99", DEFAULT_CHIRP);
        vsync(25);
        write_text(26, 9, ".9", DEFAULT_CHIRP);
        break;
    case PHASE_AMALHQ:
        cls();
        draw_background(0);
        write_text(9, TEXT_Y, "New Braintree, MA, Earth", DEFAULT_CHIRP);
        write_text(12, TEXT_Y + 2, "Headquarters of the", DEFAULT_CHIRP);
        write_text(10, TEXT_Y + 4, "Amalgamation of Worlds", DEFAULT_CHIRP);
        timer = 254;
        break;
    case PHASE_MEDAL:
        cls();
        write_text(13, 9, "Placeholder", DEFAULT_CHIRP);
        timer = 255;
    }
    phase = newphase;
}

main()
{
    char joyt;
    initialize();

    for (;;)
    {
        vsync();

        joyt = joytrg(0);

        if (joyt & JOY_RUN)
        {
            char i;
            for (i = 0; i < 64; i++)
            {
                spr_set(i);
                spr_hide();
            }
            satb_update();
            cls();
            cd_execoverlay(TITLE_OVERLAY);
        }

        if (timer < 255)
        {
            timer--;
        }

        if (timer == 0)
        {
            prepare_phase(phase + 1);
        }
    }
}
