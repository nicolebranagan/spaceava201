/*
    objects
*/

#include "classic/classic.h"

#define MAX_OBJECT_COUNT 4
#define OBJECT_SPRITE_START 2 + (MAX_ENEMY_COUNT * 2)

char object_count;
char obtained_object_count;

#define OBJ_PHOTON 0
#define OBJ_ANTIPHOTON 1

struct object
{
    char type, active, xpos, ypos;
    int xdraw, ydraw;
};

struct object objects[MAX_OBJECT_COUNT];

init_object()
{
    object_count = 0;
    obtained_object_count = 0;
}

create_object(char type, char x, char y)
{
    char new_index;
    new_index = object_count;
    object_count++;

    objects[new_index].type = type;
    objects[new_index].xdraw = x*16;
    objects[new_index].ydraw = y*16;
    objects[new_index].xpos = x;
    objects[new_index].ypos = y;
    objects[new_index].active = 1;

    spr_set(OBJECT_SPRITE_START + new_index);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(16);
    spr_pri(1);
    spr_show();
}

draw_object(char i)
{
    char row, frame;
    int dx, dy;

    spr_set(OBJECT_SPRITE_START + i);

    if (!objects[i].active) {
        spr_hide();
        return;
    }

    dx = (objects[i].xdraw) - sx;
    dy = (objects[i].ydraw) - sy;

    if (dx < 0 || dx > 256 || dy < 0 || dy > 256)
    {
        spr_hide();
        return;
    }

    row = objects[i].type == OBJ_PHOTON ? 2 : 3;
    frame = (((int)row) << 4) + (timer & 15);

    spr_x(dx);
    spr_y(dy);
    spr_pattern(AVA_VRAM + (frame * SPR_SIZE_16x16));
}

char draw_objects()
{
    char i, offset;

    if (!object_count)
    {
        return;
    }

    for (i = 0; i < object_count; i++)
    {
        draw_object(i);
    }
}

char update_objects()
{
    char i;

    if (!object_count)
    {
        return 0;
    }

    for (i = 0; i < object_count; i++)
    {
        if (objects[i].active && ava_x == objects[i].xpos && ava_y == objects[i].ypos)
        {
            objects[i].active = 0;
            obtained_object_count++;
            spr_set(OBJECT_SPRITE_START + i);
            spr_hide();
            satb_update();
        }
    }

    return obtained_object_count == object_count;
}
