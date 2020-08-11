/*
  objects, which includes enemies
*/

#define MAX_OBJECT_COUNT 24
#define OBJECT_SPRITE_START 1

#include "neptune/neptune.h"

char object_count;
char obtained_object_count;
char photon_count;
char flipper;

struct object
{
    char type, active, xpos, ypos, facing_left, frame, timer;
    signed char xdel, ydel;
    int xdraw, ydraw;
};

struct object objects[MAX_OBJECT_COUNT];

#define OBJ_PHOTON 0
#define OBJ_ANTIPHOTON 1
#define OBJ_BLOBBO 2
#define OBJ_WALKER 3
#define OBJ_ACORN 4
#define OBJ_BIGMOUTH 5
#define OBJ_BALL 6

const int BASE_VRAM[] = {
    SIDEOBJ_VRAM,
    (SIDEOBJ_VRAM + (8 * SPR_SIZE_16x16)),
    SIDENMY_VRAM,
    (SIDENMY_VRAM + (2 * SPR_SIZE_16x16)),
    (SIDENMY_VRAM + (6 * SPR_SIZE_16x16)),
    (SIDENMY_VRAM + (8 * SPR_SIZE_16x16)),
    (SIDENMY_VRAM + (10 * SPR_SIZE_16x16))};

const char BASE_PAL[] = {
    SIDEOBJ_PAL,
    SIDEOBJ_PAL,
    SIDENMY_PAL,
    SIDENMY_PAL,
    SIDENMY_PAL,
    SIDENMY_PAL,
    SIDENMY_PAL};

init_objects()
{
    object_count = 0;
    obtained_object_count = 0;
    photon_count = 0;
    flipper = 0;
}

create_object(char type, char x, char y, signed char facing)
{
    char i, new_index;
    new_index = MAX_OBJECT_COUNT + 1;
    for (i = 0; i < object_count; i++)
    {
        if (!objects[i].active)
        {
            new_index = i;
            break;
        }
    }

    if (new_index == MAX_OBJECT_COUNT + 1)
    {
        if (object_count == MAX_OBJECT_COUNT)
        {
            return;
        }
        new_index = object_count;
        object_count++;
    }

    if ((type <= OBJ_ANTIPHOTON || type == OBJ_ACORN) && facing != -1)
    {
        photon_count++;
    }

    objects[new_index].type = type;
    objects[new_index].xdraw = x * 16;
    objects[new_index].ydraw = y * 16;
    objects[new_index].xpos = x;
    objects[new_index].ypos = y;
    objects[new_index].xdel = 0;
    objects[new_index].ydel = 0;
    objects[new_index].frame = 0;
    objects[new_index].active = 1;
    objects[new_index].timer = 0;
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

        if (objects[i].type >= OBJ_BIGMOUTH)
        {
            spr_pattern((BASE_VRAM[objects[i].type]) + (objects[i].frame * SPR_SIZE_16x16));
        }
        else if (objects[i].type <= OBJ_ANTIPHOTON)
        {
            spr_pattern((BASE_VRAM[objects[i].type]) + (((timer >> 1) & 7) * SPR_SIZE_16x16));
        }
        else
        {
            spr_pattern((BASE_VRAM[objects[i].type]) + (((objects[i].frame << 1) + ((timer >> 4) & 1)) * SPR_SIZE_16x16));
        }

        spr_x(dx);
        spr_y(dy);
    }
}

update_blobbo(char index, char dely)
{
    char target_x;
    if (ava_x == objects[i].xpos && ava_y == objects[i].ypos)
    {
        if (dely)
        {
            objects[i].active = 0;
            ad_play(PCM_CANNON, CANNON_SIZE, 14, 0);
            create_object(flipper ? OBJ_PHOTON : OBJ_ANTIPHOTON, objects[i].xpos, objects[i].ypos - 1, -1);
            flipper = flipper ? 0 : 1;
            return;
        }
        else
        {
            kill_ava();
        }
    }

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

update_walker(char index)
{
    char target_x;
    char target_y;
    if (ava_x == objects[i].xpos && ava_y == objects[i].ypos)
    {
        kill_ava();
    }

    if (objects[i].facing_left)
    {
        target_x = objects[i].xpos - 1;
        target_y = objects[i].ypos - 1;
    }
    else
    {
        target_x = objects[i].xpos + 1;
        target_y = objects[i].ypos + 1;
    }

    objects[i].frame = 0;
    if (is_ladder(objects[i].xpos, target_y))
    {
        objects[i].ydel = objects[i].facing_left ? -16 : 16;
        objects[i].ypos = target_y;
        objects[i].frame = 1;
    }
    else if (is_ladder(objects[i].xpos, objects[i].ypos) && is_empty(objects[i].xpos, target_y))
    {
        objects[i].ydel = objects[i].facing_left ? -16 : 16;
        objects[i].ypos = target_y;
        objects[i].frame = 1;
    }
    else if (!is_empty(target_x, objects[i].ypos + 1) && !is_solid(target_x, objects[i].ypos))
    {
        objects[i].xdel = objects[i].facing_left ? -16 : 16;
        objects[i].xpos = target_x;
    }
    else
    {
        objects[i].facing_left = !objects[i].facing_left;
    }
}

update_bigmouth(char index)
{
    objects[index].timer++;
    if (objects[index].timer == 3)
    {
        objects[index].frame = 1;
    }
    else
    {
        objects[index].frame = 0;
    }

    if (objects[index].timer == 4)
    {
        if (objects[index].facing_left)
        {
            create_object(OBJ_BALL, objects[index].xpos, objects[index].ypos, LEFT);
        }
        else
        {
            create_object(OBJ_BALL, objects[index].xpos, objects[index].ypos, RIGHT);
        }

        ad_play(PCM_CANNON, CANNON_SIZE, 14, 0);
        objects[index].timer = 0;
    }
}

update_ball(char index)
{
    char target_x;
    if (objects[index].xpos == ava_x && objects[index].ypos == ava_y)
    {
        kill_ava();
        return;
    }
    if (objects[index].facing_left)
    {
        target_x = objects[index].xpos - 1;
        if (objects[index].xpos == 0)
        {
            objects[index].active = 0;
        }
    }
    else
    {
        target_x = objects[index].xpos + 1;
        if (objects[index].xpos == 64)
        {
            objects[index].active = 0;
        }
    }

    if (is_empty(target_x, objects[index].ypos))
    {
        objects[index].xdel = objects[index].facing_left ? -16 : 16;
        objects[index].xpos = target_x;
    }
    else
    {
        objects[index].active = 0;
    }
}

char update_objects(char delx, char dely)
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
            update_blobbo(i, 0);
            break;
        }
        case OBJ_WALKER:
        {
            update_walker(i);
            break;
        }
        case OBJ_ACORN:
        {
            update_blobbo(i, dely);
            break;
        }
        case OBJ_BIGMOUTH:
        {
            update_bigmouth(i);
            break;
        }
        case OBJ_BALL:
        {
            update_ball(i);
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

    for (i = 0; i < object_count; i++)
    {
        if (!objects[i].active)
        {
            continue;
        }
        if (objects[i].type < OBJ_BLOBBO)
        {
            continue;
        }
        if (ava_x == objects[i].xpos && ava_y == objects[i].ypos)
        {
            kill_ava();
        }
    }

    return photon_count && (obtained_object_count == photon_count);
}
