/*
	Governor. Handles the flow of the game.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#define FONT_VRAM 0x0800
#incbin(fontpal, "palettes/8x8.pal");
#incbin(nullpal, "palettes/null.pal");

#define BACKUP_RAM_SIZE 5
#define BACKUP_RAM_NAME "\0\0SPACE AVA"

#define CONTINUE 255
#define NEW_GAME 254
#define LEVEL_SELECT 253
#define STORY_MODE 252

const char STEP_ORDER[] = {
    STORY_OVERLAY, 0,
    CLASSIC_OVERLAY, 0,
    STORY_OVERLAY, 1,
    CLASSIC_OVERLAY, 1,
    STORY_OVERLAY, 2,
    CLASSIC_OVERLAY, 2,
    STORY_OVERLAY, 3, // Also STORY_OVERLAY 4
    MIRROR_OVERLAY, 0,
    MIRROR_OVERLAY, 1,
    STORY_OVERLAY, 5,
    MIRROR_OVERLAY, 2,
    MIRROR_OVERLAY, 3,
    MIRROR_OVERLAY, 4,
    STORY_OVERLAY, 6,
    CLASSIC_OVERLAY, 3,
    STORY_OVERLAY, 7,
    CLASSIC_OVERLAY, 4,
    STORY_OVERLAY, 8,
    CLASSIC_OVERLAY, 5,
    CLASSIC_OVERLAY, 6,
    STORY_OVERLAY, 9, // Also STORY_OVERLAY 10, 11
    NEPTUNE_OVERLAY, 7,
    NEPTUNE_OVERLAY, 8,
    STORY_OVERLAY, 12,
    NEPTUNE_OVERLAY, 9,
    NEPTUNE_OVERLAY, 10,
    NEPTUNE_OVERLAY, 11,
    STORY_OVERLAY, 13, // Also STORY_OVERLAY 14
    MIRROR_OVERLAY, 6,
    MIRROR_OVERLAY, 7,
    MIRROR_OVERLAY, 5,
    STORY_OVERLAY, 15,
    CLASSIC_OVERLAY, 12,
    CLASSIC_OVERLAY, 13,
    STORY_OVERLAY, 16,
    CLASSIC_OVERLAY, 14,
    STORY_OVERLAY, 17,
    CLASSIC_OVERLAY, 15,
    STORY_OVERLAY, 18,
    CLASSIC_OVERLAY, 16,
    STORY_OVERLAY, 19,
    CLASSIC_OVERLAY, 17,
    CLASSIC_OVERLAY, 18,
    STORY_OVERLAY, 20, // Also STORY_OVERLAY 21
    FINAL_OVERLAY, 0,
    STORY_OVERLAY, 22, // Also STORY_OVERLAY 23, 24
    POSTCREDITS_OVERLAY, 0};

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
    load_palette(0, nullpal, 1);
    cd_loadvram(IMAGE_OVERLAY, _8X8_SECTOR_OFFSET, FONT_VRAM, _8X8_SIZE);
    load_palette(0, fontpal, 1);

    reset_satb();
}

#define ASCII_ZERO 48
level_select()
{
    char joyt;
    char *leveltxt;
    char level_num;
    governor_step = 0;
    has_backup_ram = 0;
    cd_playtrk(TRACK_CHIME_2020, TRACK_CHIME_2020 + 1, CDPLAY_REPEAT);

    for (;;)
    {
        write_text(7, "Space Ava201");
        write_text(9, "Level Select");

        switch (STEP_ORDER[governor_step << 1])
        {
        case STORY_OVERLAY:
            write_text(13, "Story Section");
            break;
        case CLASSIC_OVERLAY:
            write_text(13, "Standard Gameplay");
            break;
        case MIRROR_OVERLAY:
            write_text(13, "Mirror Puzzle");
            break;
        case NEPTUNE_OVERLAY:
            write_text(13, "Neptune Gameplay");
            break;
        case FINAL_OVERLAY:
            write_text(13, "Final Boss Battle");
            break;
        default:
            write_text(13, "Unknown Level Type");
            break;
        }

        leveltxt = "Singularityxx";
        level_num = STEP_ORDER[(governor_step << 1) + 1];
        if (level_num > 9)
        {
            leveltxt[11] = ASCII_ZERO + (level_num / 10);
            leveltxt[12] = ASCII_ZERO + (level_num % 10);
        }
        else
        {
            leveltxt[11] = ASCII_ZERO + (level_num % 10);
            leveltxt[12] = 0;
        }
        write_text(14, "<                       >");
        write_text(15, leveltxt);

        vsync();

        joyt = joytrg(0);

        if (joyt & JOY_LEFT)
        {
            governor_step--;
            cls();
        }

        if (joyt & JOY_RIGHT)
        {
            governor_step++;
            cls();
        }

        if (joyt & JOY_STRT || joyt & JOY_I)
        {
            cls();
            write_text(11, "Loading...");
            vsync();
            continue_cycle();
        }
    }
}

start_new_game(char show_saved_game_warning)
{
    char backupmem_exists;
    int free_mem;
    governor_step = 0;
    steps = 0;
    deaths = 0;

    if (!bm_check())
    {
        no_backup_ram("No backup RAM detected.");
        has_backup_ram = 0;
        return;
    }

    backupmem_exists = bm_exist(BACKUP_RAM_NAME);

    if (bm_exist(BACKUP_RAM_NAME))
    {
        load_saved_game(show_saved_game_warning);
        return;
    }

    free_mem = bm_free();

    if (free_mem == -1)
    {
        format();
        return;
    }

    if (free_mem < BACKUP_RAM_SIZE)
    {
        no_backup_ram("Not enough space in backup RAM.");
        has_backup_ram = 0;
        return;
    }

    bm_create(BACKUP_RAM_NAME, BACKUP_RAM_SIZE);
    save_data();
    return;
}

save_data()
{
    char data[BACKUP_RAM_SIZE];

    data[0] = governor_step;
    data[1] = ((steps & 0xFF00) >> 8);
    data[2] = (steps & 0x00FF);
    data[3] = (deaths & 0xFF00) >> 8;
    data[4] = deaths & 0x00FF;
    bm_write(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
}

no_backup_ram(char *message)
{
    cls();
    write_text(9, message);
    write_text(11, "No game data can be saved.");
    write_text(15, "Press any key to continue.");

    press_any_key();
}

press_any_key()
{
    char joyt;
    for (;;)
    {
        vsync();
        joyt = joytrg(0);

        if (joyt)
        {
            return;
        }
    }
}

format()
{
    char opt;
    char joyt;

    opt = 1;

    cls();
    write_text(8, "Backup RAM is not formatted.");
    write_text(10, "Format Backup RAM?");
    for (;;)
    {
        write_text(13, opt ? "> Yes   No  " : "  Yes > No  ");
        vsync();
        joyt = joytrg(0);
        if (joyt & JOY_LEFT || joyt & JOY_RIGHT)
        {
            if (opt == 0)
            {
                opt = 1;
            }
            else
            {
                opt = 0;
            }
        }
        if ((joyt & JOY_I) || (joyt & JOY_II) || (joyt & JOY_RUN))
        {
            if (opt)
            {
                bm_format();
                bm_create(BACKUP_RAM_NAME, BACKUP_RAM_SIZE);
                save_data();
                has_backup_ram = 1;
                return;
            }
            else
            {
                has_backup_ram = 0;
                return;
            }
        }
    }
}

load_saved_game(char show_saved_game_warning)
{
    char opt;
    char joyt;
    char data[BACKUP_RAM_SIZE];
    char new_gov_step;
    int new_steps;
    int new_deaths;

    opt = 0;

    bm_read(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
    governor_step = data[0];
    steps = (((int)(data[1])) << 8) + data[2];
    deaths = (((int)(data[3])) << 8) + data[4];

    if (data[0] == 0 || !show_saved_game_warning)
    {
        if (show_saved_game_warning)
        {
            steps = 0;
            deaths = 0;
        }
        has_backup_ram = 1;
        return;
    }

    cls();
    write_text(8, "Existing saved game found");
    for (;;)
    {
        write_text(11, opt == 0 ? "> Start new game         " : "  Start new game         ");
        write_text(13, opt == 1 ? "> New game without saving" : "  New game without saving");
        write_text(15, opt == 2 ? "> Load save data         " : "  Load save data         ");

        vsync();
        joyt = joytrg(0);
        if ((joyt & JOY_UP) && opt > 0)
        {
            opt--;
        }
        if ((joyt & JOY_DOWN) && opt < 2)
        {
            opt++;
        }
        if ((joyt & JOY_I) || (joyt & JOY_II) || (joyt & JOY_RUN))
        {
            if (opt == 2)
            {
                has_backup_ram = 1;
                return;
            }
            else if (opt == 0)
            {
                data[0] = 0;
                steps = 0;
                deaths = 0;
                bm_write(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
                has_backup_ram = 1;
                return;
            }
            else
            {
                data[0] = 0;
                steps = 0;
                deaths = 0;
                has_backup_ram = 0;
                return;
            }
        }
    }
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

continue_cycle()
{
loop:
    if (story_mode &&
        (STEP_ORDER[governor_step << 1] != STORY_OVERLAY &&
         STEP_ORDER[governor_step << 1] != POSTCREDITS_OVERLAY))
    {
        governor_step++;
        goto loop;
    }
    current_level = STEP_ORDER[(governor_step << 1) + 1];
    cd_execoverlay(STEP_ORDER[governor_step << 1]);
}

main()
{
    initialize();
    vsync();
    if (governor_step == LEVEL_SELECT)
    {
        story_mode = 0;
        cls();
        level_select();
    }
    else if (governor_step == NEW_GAME || governor_step == CONTINUE)
    {
        story_mode = 0;
        start_new_game(governor_step == NEW_GAME);
        cls();
        write_text(11, "Loading...");
    }
    else if (governor_step == STORY_MODE)
    {
        story_mode = 1;
        has_backup_ram = 0;
        governor_step = 0;
        cls();
        write_text(11, "Loading...");
    }
    else
    {
        governor_step++;

        if (story_mode)
        {
            write_text(11, "And then...");
        }
        else if (has_backup_ram)
        {
            write_text(11, "Saving...");
            save_data();
        }
        else
        {
            write_text(11, "Loading...");
        }
    }
    continue_cycle();
    for (;;)
    {
        vsync();
    }
}
