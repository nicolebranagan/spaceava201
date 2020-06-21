/*
	Enemy logic
*/

#include "images/images.h"
#incbin(bigmouthpal, "palettes/bigmouth.pal");

#define MAX_ENEMY_COUNT 8

// Enemy types
#define ENEMY_TYPE_COUNT 2
#define TYPE_BIGMOUTH 0
#define TYPE_BALL 1

const char PALETTE_BY_TYPE[] = {17, 17};

int enemy_vram[ENEMY_TYPE_COUNT];
char enemy_palette[ENEMY_TYPE_COUNT];

char enemy_count;

struct enemy
{
    char type, x, y, timer, active, facing, frame;
};

struct enemy enemies[MAX_ENEMY_COUNT];

init_enemy()
{
    char i;
    enemy_count = 0;

    for (i = 0; i < ENEMY_TYPE_COUNT; i++)
    {
        enemy_vram[i] = 0;
    }

    load_palette(17, bigmouthpal, 1);
}

// Returns a new VRAM offset
int create_enemy(int vram_offset, char type, char x, char y, char facing)
{
    char i, new_index;

    new_index = MAX_ENEMY_COUNT + 1;
    for (i = 0; i < enemy_count; i++)
    {
        if (!enemies[i].active)
        {
            new_index = i;
            break;
        }
    }

    if (new_index == MAX_ENEMY_COUNT + 1)
    {
        if (enemy_count == MAX_ENEMY_COUNT)
        {
            return 0;
        }
        new_index = enemy_count;
        enemy_count++;
    }

    enemies[new_index].type = type;
    enemies[new_index].x = x;
    enemies[new_index].y = y;
    enemies[new_index].timer = 0;
    enemies[new_index].active = 1;
    enemies[new_index].facing = facing;
    enemies[new_index].frame = 0;
    if (vram_offset && type == TYPE_BIGMOUTH || type == TYPE_BALL)
    {
        if (vram_offset && !enemy_vram[TYPE_BIGMOUTH])
        {
            cd_loadvram(IMAGE_OVERLAY, BIGMOUTH_SECTOR_OFFSET, vram_offset, BIGMOUTH_SIZE_IN_BYTES);
            enemy_vram[TYPE_BIGMOUTH] = vram_offset;
            enemy_vram[TYPE_BALL] = vram_offset + 12 * SPR_SIZE_16x16;
            vram_offset += BIGMOUTH_SIZE_IN_BYTES;
        }
    }

    return vram_offset;
}

// Returns a new sprite offset
char draw_enemy(char sprite_offset, char enemyIndex)
{
    char frame;
    char ctrl_flags = SZ_16x16;
    int dx, dy; // display x, display y

    frame = enemies[enemyIndex].frame;
    dx = (enemies[enemyIndex].x * 16) - sx;
    dy = (enemies[enemyIndex].y * 16) - sy;

    if (enemies[enemyIndex].type != TYPE_BALL)
    {
        switch (enemies[enemyIndex].facing)
        {
        case DOWN:
            frame += 0;
            break;
        case UP:
            frame += 8;
            break;
        case LEFT:
            ctrl_flags = ctrl_flags | FLIP_X;
        case RIGHT:
            frame += 4;
            break;
        }
    }

    spr_set(sprite_offset);
    spr_x(dx);
    spr_y(dy - 16);
    spr_pattern(enemy_vram[enemies[enemyIndex].type] + (frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, ctrl_flags);
    spr_pal(PALETTE_BY_TYPE[enemies[enemyIndex].type]);
    spr_pri(1);
    spr_show();

    spr_set(sprite_offset + 1);
    spr_x(dx);
    spr_y(dy);
    spr_pattern(enemy_vram[enemies[enemyIndex].type] + SPR_SIZE_16x16 + (frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, ctrl_flags);
    spr_pal(PALETTE_BY_TYPE[enemies[enemyIndex].type]);
    spr_pri(1);
    if (enemies[enemyIndex].active)
    {
        spr_show();
    }
    else
    {
        spr_hide();
    }

    return sprite_offset + 2;
}

#define SPRITE_START 2;

draw_enemies()
{
    char i, offset;

    offset = SPRITE_START;
    for (i = 0; i < enemy_count; i++)
    {
        offset = draw_enemy(offset, i);
    }
    for (i = enemy_count; i < MAX_ENEMY_COUNT; i++)
    {
        spr_set(offset);
        spr_hide();
        spr_set(offset + 1);
        spr_hide();
        offset = offset + 2;
    }
    return;
}

update_bigmouth(char index)
{
    enemies[index].timer++;
    if (enemies[index].timer == 2)
    {
        enemies[index].frame = 2;
    }
    else
    {
        enemies[index].frame = 0;
    }

    if (enemies[index].timer == 3)
    {
        create_enemy(0, TYPE_BALL, enemies[index].x, enemies[index].y, enemies[index].facing);
    }

    if (enemies[index].timer > 3)
    {
        enemies[index].timer = 0;
    }
}

update_ball(char index)
{
    switch (enemies[index].facing)
    {
    case UP:
        if (!is_solid(enemies[index].x, enemies[index].y - 1))
        {
            enemies[index].y -= 1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    case DOWN:
        if (!is_solid(enemies[index].x, enemies[index].y + 1))
        {
            enemies[index].y += 1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    case LEFT:
        if (!is_solid(enemies[index].x - 1, enemies[index].y))
        {
            enemies[index].x -= 1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    case RIGHT:
        if (!is_solid(enemies[index].x + 1, enemies[index].y))
        {
            enemies[index].x += 1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    }
}

update_enemies()
{
    char i;
    for (i = 0; i < enemy_count; i++)
    {
        if (!enemies[i].active) {
            continue;
        }
        switch (enemies[i].type)
        {
        case TYPE_BIGMOUTH:
        {
            update_bigmouth(i);
            break;
        }
        case TYPE_BALL:
        {
            update_ball(i);
            break;
        }
        }
    }
}
