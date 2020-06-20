/*
	Enemy logic
*/

#include "images/images.h"
#incbin(bigmouthpal, "palettes/bigmouth.pal");

#define MAX_ENEMY_COUNT 8

// Enemy types
#define ENEMY_TYPE_COUNT 1
#define TYPE_BIGMOUTH 0

int enemy_vram[ENEMY_TYPE_COUNT];
char enemy_palette[ENEMY_TYPE_COUNT];

char enemy_count;

struct enemy
{
    char type, x, y, timer, active;
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
int create_enemy(int vram_offset, char type, char x, char y)
{
    if (enemy_count == MAX_ENEMY_COUNT)
    {
        return 0;
    }

    enemies[enemy_count].type = type;
    if (type == TYPE_BIGMOUTH)
    {
        enemies[enemy_count].x = x;
        enemies[enemy_count].y = y;
        enemies[enemy_count].active = 1;
        enemies[enemy_count].timer = 0;

        if (!enemy_vram[type])
        {
            cd_loadvram(IMAGE_OVERLAY, BIGMOUTH_SECTOR_OFFSET, vram_offset, BIGMOUTH_SIZE_IN_BYTES);
            enemy_vram[type] = vram_offset;
            vram_offset += BIGMOUTH_SIZE_IN_BYTES;
        }
    }

    enemy_count++;
    return vram_offset;
}

// Returns a new sprite offset
char draw_enemy(char sprite_offset, char enemyIndex)
{
    char frame;
    char ctrl_flags = SZ_16x16;
    int dx, dy; // display x, display y

    frame = 0;
    dx = (enemies[enemyIndex].x * 16) - sx;
    dy = (enemies[enemyIndex].y * 16) - sy;

    spr_set(sprite_offset);
    spr_x(dx);
    spr_y(dy - 16);
    spr_pattern(enemy_vram[enemies[enemyIndex].type] + (2 * frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(1 + enemies[enemyIndex].type);
    spr_pri(1);
    spr_show();

    spr_set(sprite_offset + 1);
    spr_x(dx);
    spr_y(dy);
    spr_pattern(enemy_vram[enemies[enemyIndex].type] + SPR_SIZE_16x16 + (2 * frame * SPR_SIZE_16x16));
    spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
    spr_pal(1 + enemies[enemyIndex].type);
    spr_pri(1);
    spr_show();

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
    return;
}
