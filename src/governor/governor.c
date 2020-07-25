/*
	Governor. Handles the flow of the game.
*/

#include <huc.h>

#include "./images/images.h"
#include "cd.h"

#define FONT_VRAM 0x0800
#incbin(fontpal, "palettes/8x8.pal");

#define BACKUP_RAM_SIZE 1
#define BACKUP_RAM_NAME "\0\0SPACE AVA"

const char STEP_ORDER[] = {
    STORY_OVERLAY, 7,
    STORY_OVERLAY, 0,
    CLASSIC_OVERLAY, 0,
    STORY_OVERLAY, 1,
    CLASSIC_OVERLAY, 1,
    STORY_OVERLAY, 2,
    CLASSIC_OVERLAY, 2,
    STORY_OVERLAY, 3,
    STORY_OVERLAY, 4,
    MIRROR_OVERLAY, 0,
    MIRROR_OVERLAY, 1,
    STORY_OVERLAY, 5,
    MIRROR_OVERLAY, 2,
    MIRROR_OVERLAY, 3,
    MIRROR_OVERLAY, 4,
    STORY_OVERLAY, 6,
    CLASSIC_OVERLAY, 3};

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
    cd_loadvram(IMAGE_OVERLAY, _8X8_SECTOR_OFFSET, FONT_VRAM, _8X8_SIZE);
    load_palette(0, fontpal, 1);
    set_xres(256);
    set_screen_size(SCR_SIZE_32x32);
    scroll(0, 0, 0, 0, 223, 0xC0);
    disp_on();

    cls();
    reset_satb();
}

start_new_game()
{
    char backupmem_exists;
    int free_mem;
    char data[BACKUP_RAM_SIZE];
    governor_step = 0;

    if (!bm_check())
    {
        no_backup_ram("No backup RAM detected.");
        has_backup_ram = 0;
        return;
    }

    backupmem_exists = bm_exist(BACKUP_RAM_NAME);

    if (bm_exist(BACKUP_RAM_NAME))
    {
        load_saved_game();
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
    bm_write(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
    return;
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
    char data[BACKUP_RAM_SIZE];

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
                data[0] = 0;
                bm_format();
                bm_create(BACKUP_RAM_NAME, BACKUP_RAM_SIZE);
                bm_write(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
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

load_saved_game()
{
    char opt;
    char joyt;
    char data[BACKUP_RAM_SIZE];

    opt = 0;

    bm_read(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
    if (data[0] == 0) {
        has_backup_ram = 1;
        return;
    }

    cls();
    write_text(8, "Existing saved game found");
    for (;;)
    {
        write_text(11, opt == 0 ? "> Load save data         " : "  Load save data         ");
        write_text(13, opt == 1 ? "> Start new game         " : "  Start new game         ");
        write_text(15, opt == 2 ? "> New game without saving" : "  New game without saving");

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
            if (opt == 0)
            {
                governor_step = data[0];
                has_backup_ram = 1;
                return;
            }
            else if (opt == 1)
            {
                data[0] = 0;
                bm_write(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
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
    char state;
    current_level = STEP_ORDER[(governor_step << 1) + 1];
    cd_execoverlay(STEP_ORDER[governor_step << 1]);
}

main()
{
    char data[BACKUP_RAM_SIZE];

    initialize();
    vsync();
    if (governor_step == 255)
    {
        start_new_game();
        cls();
        write_text(11, "Loading...");
    }
    else
    {
        write_text(11, "Loading...");
        governor_step++;

        if (has_backup_ram)
        {
            data[0] = governor_step;
            bm_write(data, BACKUP_RAM_NAME, 0, BACKUP_RAM_SIZE);
        }
    }
    continue_cycle();
    for (;;)
    {
        vsync();
    }
}
