/*
  objects, which includes enemies
*/

#define MAX_OBJECT_COUNT 16
#define OBJECT_SPRITE_START 1

#include "neptune/neptune.h"

char object_count;
char obtained_object_count;
char photon_count;

struct object
{
    char type, active, xpos, ypos, facing_left;
    signed char xdel, ydel;
    int xdraw, ydraw;
};

struct object objects[MAX_OBJECT_COUNT];

#define OBJ_PHOTON 0
#define OBJ_ANTIPHOTON 1
#define OBJ_BLOBBO 2

const int BASE_VRAM[] = {
    SIDEOBJ_VRAM,
    (SIDEOBJ_VRAM + (8 * SPR_SIZE_16x16)),
    SIDENMY_VRAM};

const char BASE_PAL[] = {
    SIDEOBJ_PAL,
    SIDEOBJ_PAL,
    SIDENMY_PAL};

init_objects()
{
    object_count = 0;
    obtained_object_count = 0;
    photon_count = 0;
}

create_object(char type, char x, char y, char facing)
{
    char new_index;
    new_index = object_count;
    object_count++;
    if (type <= OBJ_ANTIPHOTON)
    {
        photon_count++;
    }

    objects[new_index].type = type;
    objects[new_index].xdraw = x * 16;
    objects[new_index].ydraw = y * 16;
    objects[new_index].xpos = x;
    objects[new_index].ypos = y;
    objects[new_index].active = 1;
    objects[new_index].facing_left = facing == LEFT;

    spr_set(OBJECT_SPRITE_START + new_index);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(BASE_PAL[type]);
    spr_pri(1);
    spr_pattern(BASE_VRAM[type]);
    spr_show();
}

char i;
int dx, dy;
draw_objects()
{
    for (i = 0; i < object_count; i++)
    {
        spr_set(OBJECT_SPRITE_START + i);

        if (!objects[i].active)
        {
            spr_hide();
            continue;
        }

        if (objects[i].xdel != 0)
        {
            if (objects[i].xdel > 0)
            {
                objects[i].xdraw++;
                objects[i].xdel--;
            }
            else
            {
                objects[i].xdraw--;
                objects[i].xdel++;
            }
        }

        if (objects[i].ydel != 0)
        {
            if (objects[i].ydel > 0)
            {
                objects[i].ydraw++;
                objects[i].ydel--;
            }
            else
            {
                objects[i].ydraw--;
                objects[i].ydel++;
            }
        }

        dx = (objects[i].xdraw) - sx;
        dy = (objects[i].ydraw) - sy;

        if (dx < 0 || dx > 256 || dy < 0 || dy > 256)
        {
            spr_hide();
            continue;
        }
        spr_ctrl(FLIP_MAS | SIZE_MAS, objects[i].facing_left ? SZ_16x16 | FLIP_X : SZ_16x16);

        if (objects[i].type <= OBJ_ANTIPHOTON)
        {
            spr_pattern((BASE_VRAM[objects[i].type]) + (((timer >> 1) & 7) * SPR_SIZE_16x16));
        }
        else
        {
            spr_pattern((BASE_VRAM[objects[i].type]) + (((timer >> 4) & 1) * SPR_SIZE_16x16));
        }

        spr_x(dx);
        spr_y(dy);
    }
}

update_blobbo(char index)
{
    char target_x;
    if (objects[i].facing_left)
    {
        target_x = objects[i].xpos - 1;
    }
    else
    {
        target_x = objects[i].xpos + 1;
    }

    if (!is_empty(target_x, objects[i].ypos + 1) && !is_solid(target_x, objects[i].ypos))
    {
        objects[i].xdel = objects[i].facing_left ? -16 : 16;
        objects[i].xpos = target_x;
    }
    else
    {
        objects[i].facing_left = !objects[i].facing_left;
    }
}

char update_objects()
{
    if (!object_count)
    {
        return 0;
    }

    for (i = 0; i < object_count; i++)
    {
        if (!objects[i].active)
        {
            continue;
        }
        switch (objects[i].type)
        {
        case OBJ_ANTIPHOTON:
        case OBJ_PHOTON:
        {
            if (ava_x == objects[i].xpos && ava_y == objects[i].ypos)
            {
                objects[i].active = 0;
                obtained_object_count++;
                if (ad_stat())
                {
                    ad_stop();
                }
                ad_play(PCM_PHOTON, PHOTON_SIZE, 14, 0);
                spr_set(OBJECT_SPRITE_START + i);
                spr_hide();
            }
            break;
        }
        case OBJ_BLOBBO:
        {
            update_blobbo(i);
            break;
        }
        }
    }

    for (i = 0; i < object_count; i++)
    {
        if (objects[i].xdel || objects[i].ydel)
        {
            wait_for_sync(16);
            break;
        }
    }

    return photon_count && (obtained_object_count == photon_count);
}
