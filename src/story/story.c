/*
    Cutscenes
*/

#include <huc.h>
#include "./adpcm/adpcm.h"
#include "./images/images.h"
#include "cd.h"
#include "./story/chirp.c"

#incbin(framepal, "palettes/frames.pal");
#incbin(framepal2, "palettes/frames2.pal");
#incbin(retropal, "palettes/retrofont.pal");

#incbin(ava_facepal, "palettes/ava_face.pal");
#incbin(cindy_facepal, "palettes/cindy_face.pal");
#incbin(nelehu_facepal, "palettes/nelehu_face.pal");
#incbin(bob_facepal, "palettes/bob_face.pal");
#incbin(goon_facepal, "palettes/goon_face.pal");
#incbin(bunny_facepal, "palettes/bunny_face.pal");
#incbin(bagbunny_facepal, "palettes/bagbun_face.pal");
#incbin(mossbau_facepal, "palettes/mossba_face.pal");
#incbin(antoni_facepal, "palettes/antoni_face.pal");
#incbin(qcindy_facepal, "palettes/qcindy_face.pal");
#incbin(badlily_facepal, "palettes/bdlily_face.pal");
#incbin(retro_facepal, "palettes/retro_face.pal");
#incbin(futava_facepal, "palettes/futava_face.pal");
#incbin(gdlily_facepal, "palettes/gdlily_face.pal");
#incbin(schcat_facepal, "palettes/schcat_face.pal");
#incbin(_1920s_facepal, "palettes/1920s_face.pal");

#incbin(nullpal, "palettes/null.pal");

#incbin(starbasepal, "palettes/stardrop.pal");
#incbin(starshippal, "palettes/starship.pal");
#incbin(sunscapepal, "palettes/sunscape.pal");
#incbin(nepdroppal, "palettes/nepdrop.pal");
#incbin(amalghqpal, "palettes/amalghq.pal");
#incbin(harshartpal, "palettes/harshart.pal");
#incbin(harshbgpal, "palettes/harshbg.pal");
#incbin(archespal, "palettes/arches.pal");

#incbin(starbasebat, "bats/starbase-bg.bin")
#incbin(chipbasebat, "bats/basechip-bg.bin")
#incbin(emptybat, "bats/empty-bg.bin")
#incbin(voidbat, "bats/void-bg.bin")
#incbin(starshipbat, "bats/starship-bg.bin")
#incbin(redalertbat, "bats/redalert-bg.bin")
#incbin(betelshipbat, "bats/betelship.bin")
#incbin(betalertbat, "bats/betalert-bg.bin")
#incbin(sunscapebat, "bats/sunscape-bg.bin")
#incbin(nepoutbat, "bats/nepout-bg.bin")
#incbin(nepshipbat, "bats/nepship-bg.bin")
#incbin(nepponbat, "bats/neppon-bg.bin")
#incbin(starcombat, "bats/starcom-bg.bin")
#incbin(earthshipbat, "bats/earthship-bg.bin")
#incbin(innerbat, "bats/introinner.bin");
#incbin(innerbat2, "bats/office2-bg.bin");
#incbin(harshartbat, "bats/harshart-bg.bin");
#incbin(harshbat, "bats/harsh-bg.bin");
#incbin(archesbat, "bats/arches-bg.bin");

#define BACKDROP_VRAM 0x1000
#define FRAME_VRAM 0x2000
#define FONT_VRAM (FRAME_VRAM + (FRAMES_SIZE / 2))
#define FACE_VRAM (FONT_VRAM + (BIZCAT_SIZE / 2))
#define SPR_SIZE_16x16 0x40

char timer;
int pointer_to_data;
char script[2048];
char has_next_command;
char in_retro;
char in_credits;

const char TRACK_MAPPING[] = {
    TRACK_BALLAD,
    TRACK_EVANSSENSE,
    TRACK_CHIME_2020,
    TRACK_LACKED_EVEN_DAISY,
    TRACK_SEVENTEEN_CROWNS,
    TRACK_STRAINING_CHIME,
    TRACK_CAROUSEL,
    TRACK_HARSH_MEMORIES,
    TRACK_CANT_DIE_BOING,
    TRACK_SPACELESS,
    TRACK_WE_HAVE_NO_BANANAS};

initialize()
{
    cd_loaddata(STORY_DATA_OVERLAY, current_level, script, 2048);
    build_cast();
    ad_reset();

    disp_off();
    cd_loadvram(IMAGE_OVERLAY, FRAMES_SECTOR_OFFSET, FRAME_VRAM, FRAMES_SIZE);
    cd_loadvram(IMAGE_OVERLAY, BIZCAT_SECTOR_OFFSET, FONT_VRAM, BIZCAT_SIZE);
    cls(FONT_VRAM / 16);
    disp_on();

    scroll(0, 0, 0, 0, 223, 0xC0);
    reset_satb();
    satb_update();
    set_xres(336);
    set_screen_size(SCR_SIZE_64x32);
    cls(FONT_VRAM / 16);
    timer = 0;
    in_retro = 0;
    in_credits = 0;
}

load_new_segment(char segment)
{
    char i;
    cls(FONT_VRAM / 16);
    for (i = 0; i < 64; i++)
    {
        spr_set(i);
        spr_hide();
    }
    satb_update();
    current_level = segment;
    cd_loaddata(STORY_DATA_OVERLAY, current_level, script, 2048);
    build_cast();

    ad_reset();
    timer = 0;
    in_retro = 0;
}

#define MAX_CAST 5
int face_vram[MAX_CAST];
char palettes[MAX_CAST];

build_cast()
{
    char current_face;
    char current_palette;
    char index;
    int vram, size;
    index = 0;
    pointer_to_data = 0;
    vram = FACE_VRAM;
    current_palette = 16;
    for (;;)
    {
        current_face = script[pointer_to_data];
        if (current_face == 255)
        {
            pointer_to_data++;
            has_next_command = 1;
            return;
        }
        switch (current_face)
        {
        case 1: // Ava
            load_palette(current_palette, ava_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, AVA_FACE_SECTOR_OFFSET, vram, AVA_FACE_SIZE);
            size = AVA_FACE_SIZE;
            break;

        case 2: // Cindy
            load_palette(current_palette, cindy_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, CINDY_FACE_SECTOR_OFFSET, vram, CINDY_FACE_SIZE);
            size = CINDY_FACE_SIZE;
            break;

        case 3: // Nelehu
            load_palette(current_palette, nelehu_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, NELEHU_FACE_SECTOR_OFFSET, vram, NELEHU_FACE_SIZE);
            size = NELEHU_FACE_SIZE;
            break;

        case 4: // Bob
            load_palette(current_palette, bob_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, BOB_FACE_SECTOR_OFFSET, vram, BOB_FACE_SIZE);
            size = BOB_FACE_SIZE;
            break;

        case 5: // Quantum Goon
            load_palette(current_palette, goon_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, GOON_FACE_SECTOR_OFFSET, vram, GOON_FACE_SIZE);
            size = GOON_FACE_SIZE;
            break;

        case 6: // Bagbunny
            load_palette(current_palette, bunny_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, BUNNY_FACE_SECTOR_OFFSET, vram, BUNNY_FACE_SIZE);
            size = BUNNY_FACE_SIZE;
            break;

        case 7: // Bag bunny
            load_palette(current_palette, bagbunny_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, BAGBUN_FACE_SECTOR_OFFSET, vram, BAGBUN_FACE_SIZE);
            size = BAGBUN_FACE_SIZE;
            break;

        case 8: // Mossbauer
            load_palette(current_palette, mossbau_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, MOSSBA_FACE_SECTOR_OFFSET, vram, MOSSBA_FACE_SIZE);
            size = MOSSBA_FACE_SIZE;
            break;

        case 9: // Antonia
            load_palette(current_palette, antoni_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, ANTONI_FACE_SECTOR_OFFSET, vram, ANTONI_FACE_SIZE);
            size = ANTONI_FACE_SIZE;
            break;

        case 10: // Quantum Cindy
            load_palette(current_palette, qcindy_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, QCINDY_FACE_SECTOR_OFFSET, vram, QCINDY_FACE_SIZE);
            size = QCINDY_FACE_SIZE;
            break;

        case 11: // Evil Lily
            load_palette(current_palette, badlily_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, BDLILY_FACE_SECTOR_OFFSET, vram, BDLILY_FACE_SIZE);
            size = BDLILY_FACE_SIZE;
            break;
        case 12: // Retro
            load_palette(current_palette, retro_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, RETRO_FACE_SECTOR_OFFSET, vram, RETRO_FACE_SIZE);
            size = RETRO_FACE_SIZE;
            break;
        case 13: // Future Ava
            load_palette(current_palette, futava_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, FUTAVA_FACE_SECTOR_OFFSET, vram, FUTAVA_FACE_SIZE);
            size = FUTAVA_FACE_SIZE;
            break;
        case 14: // Good Lily
            load_palette(current_palette, gdlily_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, GDLILY_FACE_SECTOR_OFFSET, vram, GDLILY_FACE_SIZE);
            size = GDLILY_FACE_SIZE;
            break;
        case 15: // sCHRODINGER'S cAT
            load_palette(current_palette, schcat_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, SCHCAT_FACE_SECTOR_OFFSET, vram, SCHCAT_FACE_SIZE);
            size = SCHCAT_FACE_SIZE;
            break;
        case 16: // Future Ava
            load_palette(current_palette, _1920s_facepal, 1);
            palettes[index] = current_palette;
            current_palette++;
            cd_loadvram(IMAGE_OVERLAY, _1920S_FACE_SECTOR_OFFSET, vram, _1920S_FACE_SIZE);
            size = _1920S_FACE_SIZE;
            break;
        }
        face_vram[index] = vram;
        vram = vram + (size / 2);
        index++;
        pointer_to_data++;
    }
}

#define BACKDROP_WIDTH 16 * 2
#define BACKDROP_HEIGHT 5 * 2 // In units of 8x8 tiles
#define XTOP 5
#define YLEFT 3

draw_background(char index, char load_new_gfx)
{
    char y, background;
    int addr;

    load_palette(1, nullpal, 1);

    switch (index)
    {
    case 0:
    {
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, starbasebat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starbasepal, 1);
        break;
    }
    case 1:
    {
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, chipbasebat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starbasepal, 1);
        break;
    }
    case 2:
    {
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARDROP_SECTOR_OFFSET, BACKDROP_VRAM, STARDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, emptybat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starbasepal, 1);
        break;
    }
    case 3:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, voidbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 4:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, starshipbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 5:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, redalertbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 6:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, betelshipbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 7:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, betalertbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 8:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, SUNSCAPE_SECTOR_OFFSET, BACKDROP_VRAM, SUNSCAPE_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, sunscapebat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, sunscapepal, 1);
        break;
    case 9:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, NEPDROP_SECTOR_OFFSET, BACKDROP_VRAM, NEPDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, nepoutbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, nepdroppal, 1);
        break;
    case 10:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, nepshipbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 11:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, NEPDROP_SECTOR_OFFSET, BACKDROP_VRAM, NEPDROP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, nepponbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, nepdroppal, 1);
        break;
    case 12:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, starcombat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 13:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, STARSHIP_SECTOR_OFFSET, BACKDROP_VRAM, STARSHIP_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, earthshipbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, starshippal, 1);
        break;
    case 14:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, AMALGHQ_SECTOR_OFFSET, BACKDROP_VRAM, AMALGHQ_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, innerbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, amalghqpal, 1);
        break;
    case 15:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, AMALGHQ_SECTOR_OFFSET, BACKDROP_VRAM, AMALGHQ_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, innerbat2 + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, amalghqpal, 1);
        break;
    case 16:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, HARSHART_SECTOR_OFFSET, BACKDROP_VRAM, HARSHART_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, harshartbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, harshartpal, 1);
        break;
    case 17:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, HARSHBG_SECTOR_OFFSET, BACKDROP_VRAM, HARSHBG_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, harshbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, harshbgpal, 1);
        break;
    case 18:
        if (load_new_gfx)
            cd_loadvram(IMAGE_OVERLAY, ARCHES_SECTOR_OFFSET, BACKDROP_VRAM, ARCHES_SIZE);
        for (y = 0; y < BACKDROP_HEIGHT; y++)
        {
            addr = vram_addr(XTOP, YLEFT + y);
            load_vram(addr, archesbat + ((BACKDROP_WIDTH << 1) * y), BACKDROP_WIDTH);
        }
        load_palette(1, archespal, 1);
        break;
    }
}

enter_retro()
{
    cls();
    load_palette(0, retropal, 1);
    set_xres(256);
    cd_loadvram(IMAGE_OVERLAY, RETROFONT_SECTOR_OFFSET, FONT_VRAM, RETROFONT_SIZE);
    in_retro = 1;
}

enter_credits()
{
    cd_loadvram(IMAGE_OVERLAY, TITLEFNT_SECTOR_OFFSET, FONT_VRAM, TITLEFNT_SIZE);
    cd_loadvram(IMAGE_OVERLAY, FRAMES2_SECTOR_OFFSET, FRAME_VRAM, FRAMES2_SIZE);
    draw_frame(0);
    in_credits = 1;
}

#define CLAP_SFX 199
thunderclap()
{
    char i, j;

    load_palette(1, harshbgpal, 1);
    for (i = 0; i < 8; i++)
    {
        chirp(CLAP_SFX);
        vsync();
    }
    for (j = 0; j < 7; j++)
    {
        load_palette(1, harshbgpal + (1 << 5), 1);
        for (i = 0; i < 16; i++)
        {
            chirp(CLAP_SFX);
            vsync();
        }
        chirp(0);
        load_palette(1, harshbgpal, 1);
        for (i = 0; i < 8; i++)
        {
            vsync();
        }
    }
    load_palette(1, harshbgpal + (2 << 5), 1);
    for (i = 0; i < 8; i++)
    {
        chirp(CLAP_SFX);
        vsync();
    }
    chirp(0);
}

#define FRAME_START ((FRAME_VRAM) >> 4)
draw_frame(char frame)
{
    char x, y;
    int addr, frame_start;
    int data[BACKDROP_WIDTH + 4];

    if (in_credits)
    {
        load_palette(0, framepal2 + (frame << 5), 1);
    }
    else
    {
        load_palette(0, framepal + (frame << 5), 1);
    }
    frame_start = FRAME_START + (frame << 5);

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 1;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 8;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 9;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 4;
            continue;
        }
        data[x] = frame_start + 5;
    }
    addr = vram_addr(XTOP - 2, YLEFT - 2);
    load_vram(addr, data, BACKDROP_WIDTH + 4);

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start + 2;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 3;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 10;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 11;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 6;
            continue;
        }
        data[x] = frame_start + 7;
    }
    addr = vram_addr(XTOP - 2, YLEFT - 1);
    load_vram(addr, data, BACKDROP_WIDTH + 4);

    for (y = YLEFT; y < (YLEFT + BACKDROP_HEIGHT); y++)
    {
        data[0] = frame_start + (y % 2 ? 12 : 14);
        data[1] = frame_start + (y % 2 ? 13 : 15);
        addr = vram_addr(XTOP - 2, y);
        load_vram(addr, data, 2);

        data[0] = frame_start + (y % 2 ? 16 : 18);
        data[1] = frame_start + (y % 2 ? 17 : 19);
        addr = vram_addr(XTOP + BACKDROP_WIDTH, y);
        load_vram(addr, data, 2);
    }

    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start + 20;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 21;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 28;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 29;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 24;
            continue;
        }
        data[x] = frame_start + 25;
    }
    addr = vram_addr(XTOP - 2, YLEFT + BACKDROP_HEIGHT);
    load_vram(addr, data, BACKDROP_WIDTH + 4);
    for (x = 0; x < (BACKDROP_WIDTH + 4); x++)
    {
        if (x == 0)
        {
            data[x] = frame_start + 22;
            continue;
        }
        if (x == 1)
        {
            data[x] = frame_start + 23;
            continue;
        }
        if (x == BACKDROP_WIDTH + 2)
        {
            data[x] = frame_start + 30;
            continue;
        }
        if (x == BACKDROP_WIDTH + 3)
        {
            data[x] = frame_start + 31;
            continue;
        }
        if (x % 2)
        {
            data[x] = frame_start + 26;
            continue;
        }
        data[x] = frame_start + 27;
    }
    addr = vram_addr(XTOP - 2, YLEFT + BACKDROP_HEIGHT + 1);
    load_vram(addr, data, BACKDROP_WIDTH + 4);
}

#define TEXT_X 3
#define TEXT_Y 17

#define BLOCK_TO_CLEAR ((32 - TEXT_Y) * 64)
clear_text()
{
    int i;
    int vaddr, zero_string[BLOCK_TO_CLEAR];
    for (i = 0; i < BLOCK_TO_CLEAR; i++)
    {
        zero_string[i] = FONT_VRAM / 16;
    }
    vaddr = vram_addr(0, TEXT_Y);
    load_vram(vaddr, zero_string, BLOCK_TO_CLEAR);
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

int write_text(char *text, char chirp_type)
{
    char x, y, i;
    int len;
    x = TEXT_X;
    y = TEXT_Y;
    i = 0;
    len = 0;
    clear_text();
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

#define LEVEL_GROUND 56
draw_person(char slot, char cast_index, char face, char x_start)
{
    char i, j, x, y;
    i = slot * 6;
    j = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 2; x++)
        {
            spr_set(i);
            spr_y(LEVEL_GROUND + (y << 4));
            spr_x((((int)(x_start + x)) << 4));
            spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
            spr_pattern(face_vram[cast_index] + (face * 6 * SPR_SIZE_16x16) + (j * SPR_SIZE_16x16));
            spr_pal(palettes[cast_index]);
            spr_pri(1);
            spr_show();
            i++;
            j++;
        }
    satb_update();
}

#define HEIGHT_BG 17
draw_person_in_bg(char slot, char cast_index, char face)
{
    char i, j, x, y;
    i = slot * 6;
    j = 0;
    for (y = 0; y < 3; y++)
        for (x = 0; x < 2; x++)
        {
            spr_set(i);
            spr_y(HEIGHT_BG + (y << 4));
            spr_x((((int)(9 + x)) << 4) + 8);
            spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
            spr_pattern(face_vram[cast_index] + (face * 6 * SPR_SIZE_16x16) + (j * SPR_SIZE_16x16));
            spr_pal(palettes[cast_index]);
            spr_pri(0);
            spr_show();
            i++;
            j++;
        }
    satb_update();
}

delete_person(char slot)
{
    char i;
    for (i = 0; i < 6; i++)
    {
        spr_set((slot * 6) + i);
        spr_hide();
    }
    satb_update();
}

int sfx_size;
load_sfx(char sfx)
{
    switch (sfx)
    {
    case 0:
        ad_trans(ADPCM_OVERLAY, KABOOM_SECTOR_OFFSET, KABOOM_SECTOR_COUNT, 0);
        sfx_size = KABOOM_SIZE;
        break;
    case 1:
        ad_trans(ADPCM_OVERLAY, LOL_SECTOR_OFFSET, LOL_SECTOR_COUNT, 0);
        sfx_size = LOL_SIZE;
        break;
    }
}

draw_block(char more)
{
    int parsed[1], vaddr;
    vaddr = vram_addr(in_retro ? 28 : 39, 25);
    parsed[0] = (timer & 32) ? ((127 << 1) + (FONT_VRAM >> 4) - (more ? 64 : 63)) : FONT_VRAM / 16;
    load_vram(vaddr, parsed, 1);
}

#define CMD_SHOW_SPRITE 1
#define CMD_SHOW_TEXT 2
#define CMD_SHOW_BACKGROUND 3
#define CMD_PLAY_MUSIC 4
#define CMD_STOP_MUSIC 5
#define CMD_SHOW_FRAME 6
#define CMD_DELETE_SPRITE 7
#define CMD_LOAD_SFX 8
#define CMD_PLAY_LOADED_SFX 9
#define CMD_LOAD_NEXT_SEGMENT 10
#define CMD_SHOW_SPRITE_BG 11
#define CMD_ENTER_RETRO 12
#define CMD_THUNDERCLAP 13
#define CMD_LOAD_NEW_SEGMENT 14
#define CMD_ENTER_CREDITS 15
#define CMD_PLAY_MUSIC_ONCE 16

perform_command()
{
    char current_command, chirp_type;
    int text_len;
loop:
    current_command = script[pointer_to_data];

    switch (current_command)
    {
    case CMD_SHOW_SPRITE:
        draw_person(
            script[pointer_to_data + 1],
            script[pointer_to_data + 2],
            script[pointer_to_data + 3],
            script[pointer_to_data + 4]);
        pointer_to_data += 5;
        goto loop;
        break;
    case CMD_SHOW_TEXT:
        chirp_type = script[pointer_to_data + 1];
        text_len = write_text(script + pointer_to_data + 2, chirp_type);
        pointer_to_data += (text_len + 2);
        break;
    case CMD_SHOW_BACKGROUND:
        draw_background(script[pointer_to_data + 1], script[pointer_to_data + 2]);
        pointer_to_data += 3;
        goto loop;
        break;
    case CMD_PLAY_MUSIC:
        chirp_type = TRACK_MAPPING[script[pointer_to_data + 1]];
        cd_playtrk(chirp_type, chirp_type + 1, CDPLAY_REPEAT);
        pointer_to_data += 2;
        goto loop;
        break;
    case CMD_STOP_MUSIC:
        cd_reset();
        pointer_to_data += 1;
        goto loop;
        break;
    case CMD_SHOW_FRAME:
        draw_frame(script[pointer_to_data + 1]);
        pointer_to_data += 2;
        goto loop;
        break;
    case CMD_DELETE_SPRITE:
        delete_person(script[pointer_to_data + 1]);
        pointer_to_data += 2;
        goto loop;
        break;
    case CMD_LOAD_SFX:
        load_sfx(script[pointer_to_data + 1]);
        pointer_to_data += 2;
        goto loop;
        break;
    case CMD_PLAY_LOADED_SFX:
        ad_stop();
        ad_play(0, sfx_size, 15, 0);
        pointer_to_data += 1;
        goto loop;
        break;
    case CMD_LOAD_NEXT_SEGMENT:
        current_level++;
        cd_loaddata(STORY_DATA_OVERLAY, current_level, script, 2048);
        pointer_to_data = 0;
        goto loop;
        break;
    case CMD_SHOW_SPRITE_BG:
        draw_person_in_bg(
            script[pointer_to_data + 1],
            script[pointer_to_data + 2],
            script[pointer_to_data + 3]);
        pointer_to_data += 4;
        goto loop;
        break;
    case CMD_ENTER_RETRO:
        pointer_to_data += 1;
        enter_retro();
        goto loop;
        break;
    case CMD_THUNDERCLAP:
        pointer_to_data += 1;
        thunderclap();
        goto loop;
        break;
    case CMD_LOAD_NEW_SEGMENT:
        load_new_segment(script[pointer_to_data + 1]);
        goto loop;
        break;
    case CMD_ENTER_CREDITS:
        pointer_to_data += 1;
        enter_credits();
        goto loop;
        break;
    case CMD_PLAY_MUSIC_ONCE:
        chirp_type = TRACK_MAPPING[script[pointer_to_data + 1]];
        cd_playtrk(chirp_type, chirp_type + 1, CDPLAY_NORMAL);
        pointer_to_data += 2;
        goto loop;
        break;
    }

    if (script[pointer_to_data] == 255)
    {
        has_next_command = 0;
    }
    else
    {
        has_next_command = 1;
    }
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
    cls(FONT_VRAM / 16);
    cd_execoverlay(GOVERNOR_OVERLAY);
}

main()
{
    char joyt;

    initialize();

    perform_command();
    vsync();
    for (;;)
    {
        vsync();
        timer++;
        draw_block(has_next_command);

        joyt = joytrg(0);

        if (!in_credits)
        {
            if (joyt & JOY_RUN)
            {
                done();
            }

            if (joyt & JOY_I || joyt & JOY_II)
            {
                if (has_next_command)
                {
                    perform_command();
                }
                else
                {
                    done();
                }
            }
        }
        else
        {
            if (!has_next_command)
            {
                in_credits = 0;
                continue;
            }
            vsync(200);
            vsync(100);
            perform_command();
        }
    }
}
