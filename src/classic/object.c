/*
    objects
*/

#include "classic/classic.h"

#define MAX_OBJECT_COUNT 4
#define OBJECT_SPRITE_START (MAX_ENEMY_COUNT)

char object_count;
char obtained_object_count;
char photon_count;

#define OBJ_PHOTON 0
#define OBJ_ANTIPHOTON 1
#define OBJ_BOX 2

struct object
{
    char type, active, xpos, ypos;
    signed char xdel, ydel;
    int xdraw, ydraw;
};

struct object objects[MAX_OBJECT_COUNT];

init_object()
{
    object_count = 0;
    obtained_object_count = 0;
    photon_count = 0;
}

create_object(char type, char x, char y)
{
    char new_index;
    new_index = object_count;
    object_count++;
    if (type < OBJ_BOX)
    {
        photon_count++;
    }

    objects[new_index].type = type;
    objects[new_index].xdraw = x * 16;
    objects[new_index].ydraw = y * 16;
    objects[new_index].xpos = x;
    objects[new_index].ypos = y;
    objects[new_index].active = 1;

    spr_set(BOTTOM_HALF_START + OBJECT_SPRITE_START + new_index);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(16);
    spr_pri(1);
    spr_pattern(AVA_VRAM + ((((int)(type + 2)) << 4) * SPR_SIZE_16x16));
    spr_show();
}

char i;
int dx, dy;

draw_objects()
{
    for (i = 0; i < object_count; i++)
    {
        spr_set(BOTTOM_HALF_START + OBJECT_SPRITE_START + i);

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

        if (objects[i].type < OBJ_BOX)
        {
            spr_pattern(AVA_VRAM + (((((int)(objects[i].type + 2)) << 4) + (timer & 15)) * SPR_SIZE_16x16));
        }

        spr_x(dx);
        spr_y(dy);
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
        if (objects[i].type < OBJ_BOX && objects[i].active && ava_x == objects[i].xpos && ava_y == objects[i].ypos)
        {
            objects[i].active = 0;
            obtained_object_count++;
            if (ad_stat())
            {
                ad_stop();
            }
            ad_play(PHOTON_LOC, PHOTON_SIZE, 14, 0);
            spr_set(BOTTOM_HALF_START + OBJECT_SPRITE_START + i);
            spr_hide();
        }
    }

    return obtained_object_count == photon_count;
}

char move_box(char index)
{
    char dx, dy, new_x, new_y;
    dx = objects[index].xpos - ava_x;
    dy = objects[index].ypos - ava_y;
    new_x = objects[index].xpos + dx;
    new_y = objects[index].ypos + dy;

    if (is_solid(new_x, new_y, 0))
    {
        return 1;
    }

    for (i = 0; i < enemy_count; i++)
    {
        if (enemies[i].active && enemies[i].x == new_x && enemies[i].y == new_y)
        {
            return 1;
        }
    }

    ad_play(SHOVE_LOC, SHOVE_SIZE, 13, 0);
    objects[index].xpos = new_x;
    objects[index].ypos = new_y;
    objects[index].xdel = dx * 16;
    objects[index].ydel = dy * 16;

    return 0;
}
