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
#incbin(admiralpal, "palettes/admira_face.pal");
#incbin(bigtextpal, "palettes/shutcalc.pal");

#incbin(amalhqbin, "bats/amalhq-bg.bin");
#incbin(innerbat, "bats/introinner.bin");

#incbin(introtxt, "images/introtxt.chr");
#incbin(shutcalc, "images/shutcalc.chr");
#incbin(spacemap, "bats/spacemap.bin");

#incbin(bigtext1, "bats/bigtext1.bin");
#incbin(bigtext2, "bats/bigtext2.bin");
#incbin(bigtext3, "bats/bigtext3.bin");
#incbin(bigtext4, "bats/bigtext4.bin");
#incbin(bigtext5, "bats/bigtext5.bin");

#define SPR_SIZE_16x16 0x40

#define AMALGHQ_VRAM 0x1000
#define FONT_VRAM (AMALGHQ_VRAM + (INTROTXT_SIZE / 2))
#define ADMIRAL_VRAM (FONT_VRAM + (TITLEFNT_SIZE / 2))

char phase;
int timer;

#define PHASE_INIT 0
#define PHASE_POINT9 1
#define PHASE_AMALHQ 2
#define PHASE_MEDAL 3
#define PHASE_SHOCK1 4
#define PHASE_SHOCK2 5
#define PHASE_BIGTEXT1 6
#define PHASE_BIGTEXT2 7
#define PHASE_SPACEMAP 8
#define PHASE_BIGTEXT3 9
#define PHASE_BIGTEXT4 10
#define PHASE_BIGTEXT5 11

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
    cd_loadvram(IMAGE_OVERLAY, ADMIRA_FACE_SECTOR_OFFSET, ADMIRAL_VRAM, ADMIRA_FACE_SIZE);
    load_palette(0, fontpal, 1);
    load_palette(1, amalghqpal, 1);
    load_palette(16, admiralpal, 1);

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
    case 1:
    {
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, innerbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        break;
    }
    }
}

#define LEVEL_GROUND 56
draw_person(char face, char x_start)
{
    char i, j, x, y;
    i = 0;
    j = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 2; x++)
        {
            spr_set(i);
            spr_y(LEVEL_GROUND + (y << 4));
            spr_x((((int)(x_start + x)) << 4));
            spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
            spr_pattern(ADMIRAL_VRAM + (face * 6 * SPR_SIZE_16x16) + (j * SPR_SIZE_16x16));
            spr_pal(0);
            spr_pri(1);
            spr_show();
            i++;
            j++;
        }
    satb_update();
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
    int addr;
    char y;

    switch (newphase)
    {
    case PHASE_INIT:
        timer = 50;
        write_text(13, 9, "Space Year 99", DEFAULT_CHIRP);
        break;
    case PHASE_POINT9:
        timer = 150;
        write_text(26, 9, ".9", DEFAULT_CHIRP + 7);
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
        draw_background(1);
        draw_person(0, 8);
        write_text(TEXT_X, TEXT_Y, "ADMIRAL HARMONY: I'm honestly shocked", DEFAULT_CHIRP - 8);
        write_text(TEXT_X, TEXT_Y + 2, "and flattered! Thank you so much for", DEFAULT_CHIRP - 8);
        write_text(TEXT_X, TEXT_Y + 4, "the promotion, I promise I will", DEFAULT_CHIRP - 8);
        timer = 96;
        break;
    case PHASE_SHOCK1:
        cls();
        draw_background(1);
        draw_person(1, 8);
        write_text(TEXT_X, TEXT_Y, "ADMIRAL HARMONY: !!", DEFAULT_CHIRP - 8);
        cd_playtrk(TRACK_CHIME_2020, TRACK_CHIME_2020 + 1, CDPLAY_REPEAT);
        timer = 50;
        break;
    case PHASE_SHOCK2:
        load_palette(16, admiralpal + (1 << 5), 1);
        load_palette(1, amalghqpal + (1 << 5), 1);
        timer = 255;
        break;
    case PHASE_BIGTEXT1:
        load_palette(1, bigtextpal, 1);
        cls();
        load_vram(AMALGHQ_VRAM, introtxt, INTROTXT_SIZE / 2);
        for (y = 0; y < 64; y++)
        {
            spr_set(y);
            spr_hide();
        }
        satb_update();

        for (y = 0; y < (12 * 2); y++)
        {
            addr = vram_addr(10, 1 + y);
            load_vram(addr, bigtext1 + ((32 << 1) * y), 32);
        }
        timer = 400;
        break;
    case PHASE_BIGTEXT2:
        cls();
        for (y = 0; y < (12 * 2); y++)
        {
            addr = vram_addr(1, 1 + y);
            load_vram(addr, bigtext2 + ((32 << 1) * y), 32);
        }
        timer = 400;
        break;
    case PHASE_SPACEMAP:
        cls();
        load_vram(AMALGHQ_VRAM, shutcalc, SHUTCALC_SIZE / 2);
        for (y = 0; y < (8 * 2); y++)
        {
            addr = vram_addr(6, 4 + y);
            load_vram(addr, spacemap + ((32 << 1) * y), 32);
        }
        timer = 200;
        break;
    case PHASE_BIGTEXT3:
        cls();
        for (y = 0; y < (4 * 2); y++)
        {
            addr = vram_addr(8, 8 + y);
            load_vram(addr, bigtext3 + ((32 << 1) * y), 32);
        }
        timer = 64;
        break;
    case PHASE_BIGTEXT4:
        cls();
        for (y = 0; y < (5 * 2); y++)
        {
            addr = vram_addr(8, 7 + y);
            load_vram(addr, bigtext4 + ((32 << 1) * y), 32);
        }
        timer = 64;
        break;
    case PHASE_BIGTEXT5:
        cls();
        for (y = 0; y < (2 * 2); y++)
        {
            addr = vram_addr(6, 11 + y);
            load_vram(addr, bigtext5 + ((32 << 1) * y), 32);
        }
        timer = 128;
        break;
    default:
        done();
        break;
    }
    phase = newphase;
}

done()
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
            done();
        }

        timer--;

        if (timer == 0)
        {
            prepare_phase(phase + 1);
        }
    }
}
