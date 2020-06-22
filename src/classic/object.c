/*
    objects
*/

#define MAX_OBJECT_COUNT 4
#define OBJECT_SPRITE_START 2 + (MAX_ENEMY_COUNT * 2)

char object_count;

#define OBJ_PHOTON 0
#define OBJ_ANTIPHOTON 1

struct object
{
    char type, x, y, active, frame;
};

struct object objects[MAX_OBJECT_COUNT];

init_object()
{
    object_count = 0;
}

create_object(char type, char x, char y)
{
    char new_index;
    new_index = object_count;
    object_count++;

    objects[new_index].type = type;
    objects[new_index].x = x;
    objects[new_index].y = y;
    objects[new_index].active = 1;
    objects[new_index].frame = 0;
}

char draw_object(char sprite_offset, char i)
{
    char row, frame;
    int dx, dy;
    row = objects[i].type == OBJ_PHOTON ? 2 : 3;
    frame = row * 16 + objects[i].frame;

    dx = (objects[i].x * 16) - sx;
    dy = (objects[i].y * 16) - sy;

    spr_set(sprite_offset);
    spr_x(dx);
    spr_y(dy);
    spr_pattern(AVA_VRAM + (frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(16);
    spr_pri(1);
    if (objects[i].active)
    {
        spr_show();
    }
    else
    {
        spr_hide();
    }

    objects[i].frame += 1;
    if (objects[i].frame == 16)
    {
        objects[i].frame = 0;
    }

    return sprite_offset + 1;
}

char draw_objects()
{
    char i, offset;

    if (!object_count)
    {
        return;
    }

    offset = OBJECT_SPRITE_START;
    for (i = 0; i < object_count; i++)
    {
        offset = draw_object(offset, i);
    }
}
