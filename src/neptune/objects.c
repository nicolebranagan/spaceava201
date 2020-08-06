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
    char type, active, xpos, ypos;
    signed char xdel, ydel;
    int xdraw, ydraw;
};

struct object objects[MAX_OBJECT_COUNT];

#define OBJ_PHOTON 0
#define OBJ_ANTIPHOTON 1

const int BASE_VRAM[] = {
    SIDEOBJ_VRAM,
    (SIDEOBJ_VRAM + (8 * SPR_SIZE_16x16))};

init_objects()
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

    spr_set(OBJECT_SPRITE_START + new_index);
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(SIDEOBJ_PAL);
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

        dx = (objects[i].xdraw) - sx;
        dy = (objects[i].ydraw) - sy;

        if (dx < 0 || dx > 256 || dy < 0 || dy > 256)
        {
            spr_hide();
            continue;
        }

        spr_pattern((BASE_VRAM[objects[i].type]) + (((timer >> 1) & 7) * SPR_SIZE_16x16));

        spr_x(dx);
        spr_y(dy);
    }
}
