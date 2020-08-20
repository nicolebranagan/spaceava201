/*
	Enemy logic
*/

#include "classic/classic.h"
#include "images/images.h"
#incbin(bigmouthpal, "palettes/bigmouth.pal");
#incbin(eyewalkpal, "palettes/eyewalk.pal");
#incbin(goonbosspal, "palettes/goonboss.pal");
#incbin(mossbosspal, "palettes/mossboss.pal");
#incbin(goonguypal, "palettes/goonguy.pal");

#define MAX_ENEMY_COUNT 16

// Enemy types
#define ENEMY_TYPE_COUNT 6
#define TYPE_BIGMOUTH 0
#define TYPE_BALL 1
#define TYPE_EYEWALK 2
#define TYPE_BOSS1 3
#define TYPE_BOSS2 4
#define TYPE_GOONGUY 5

// Sound effects
#define ENEMY_NO_SOUND 0
#define ENEMY_CANNON 1
#define ENEMY_MINIWILHELM 2

// Other
#define DIR_FRAME_OVERRIDE 255

const char PALETTE_BY_TYPE[] = {17, 18, 19, 20, 21, 22};

int enemy_vram[ENEMY_TYPE_COUNT];
char enemy_palette[ENEMY_TYPE_COUNT];

char enemy_count;
char ava_dead;
char boss1_index;

struct enemy
{
    char type, x, y, timer, active, facing, frame;
    signed char delx, dely;
};

struct enemy enemies[MAX_ENEMY_COUNT];

int populate_enemies_vram(int vram_offset, char *enemy_data)
{
    int i;
    char x, y, type, facing, delx, dely;
    for (i = 0; i < ENEMY_TYPE_COUNT; i++)
    {
        enemy_vram[i] = 0;
    }

    i = 0;
    for (;;)
    {
        x = enemy_data[++i];
        if (x == 255)
        {
            break;
        }
        y = enemy_data[++i];
        type = enemy_data[++i];
        facing = enemy_data[++i];
        delx = enemy_data[++i];
        dely = enemy_data[++i];
        vram_offset = populate_enemy_vram(vram_offset, type);
    }

    return vram_offset;
}

int populate_enemy_vram(int vram_offset, char type)
{
    switch (type)
    {
    case TYPE_BIGMOUTH:
    case TYPE_BALL:
        if (!enemy_vram[TYPE_BIGMOUTH])
        {
            cd_loadvram(IMAGE_OVERLAY, BIGMOUTH_SECTOR_OFFSET, vram_offset, BIGMOUTH_SIZE);
            enemy_vram[TYPE_BIGMOUTH] = vram_offset;
            // The ball is on the Bigmouth graphics file
            enemy_vram[TYPE_BALL] = vram_offset + 12 * SPR_SIZE_16x16;
            vram_offset += BIGMOUTH_SIZE / 2;
        }
        break;
    case TYPE_EYEWALK:
        if (!enemy_vram[TYPE_EYEWALK])
        {
            cd_loadvram(IMAGE_OVERLAY, EYEWALK_SECTOR_OFFSET, vram_offset, EYEWALK_SIZE);
            enemy_vram[TYPE_EYEWALK] = vram_offset;
            vram_offset += EYEWALK_SIZE / 2;
        }
        break;
    case TYPE_BOSS1:
        if (!enemy_vram[TYPE_BOSS1])
        {
            cd_loadvram(IMAGE_OVERLAY, GOONBOSS_SECTOR_OFFSET, vram_offset, GOONBOSS_SIZE);
            enemy_vram[TYPE_BOSS1] = vram_offset;
            vram_offset += GOONBOSS_SIZE / 2;
        }
        break;
    case TYPE_BOSS2:
        if (!enemy_vram[TYPE_BOSS2])
        {
            cd_loadvram(IMAGE_OVERLAY, MOSSBOSS_SECTOR_OFFSET, vram_offset, MOSSBOSS_SIZE);
            enemy_vram[TYPE_BOSS2] = vram_offset;
            vram_offset += MOSSBOSS_SIZE / 2;
        }
        break;
    case TYPE_GOONGUY:
        if (!enemy_vram[TYPE_GOONGUY])
        {
            cd_loadvram(IMAGE_OVERLAY, GOONGUY_SECTOR_OFFSET, vram_offset, GOONGUY_SIZE);
            enemy_vram[TYPE_GOONGUY] = vram_offset;
            vram_offset += GOONGUY_SIZE / 2;
        }
        break;
    }

    return vram_offset;
}

init_enemy()
{
    char i;
    enemy_count = 0;
    boss1_index = 0;
    in_boss2_mode = 0;
    load_palette(17, bigmouthpal, 2);
    load_palette(19, eyewalkpal, 1);
    load_palette(20, goonbosspal, 1);
    load_palette(21, mossbosspal, 1);
    load_palette(22, goonguypal, 1);
}

create_enemy(char type, char x, char y, char facing, char delx, char dely)
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
    enemies[new_index].delx = delx;
    enemies[new_index].dely = dely;
    enemies[new_index].timer = 0;
    enemies[new_index].active = 1;
    enemies[new_index].facing = facing;
    enemies[new_index].frame = 0;

    if (type == TYPE_BOSS1)
    {
        boss1_index = new_index;
    }

    if (type == TYPE_BOSS2)
    {
        in_boss2_mode = 1;
        spr_set(BOSS_2_EXTRA);
        spr_pri(1);
        spr_pal(21);
        spr_ctrl(FLIP_MAS | SIZE_MAS, SZ_16x16);
        spr_pattern(enemy_vram[TYPE_BOSS2] + (SPR_SIZE_16x16 << 3));
        spr_show();
    }
}

// Returns a new sprite offset
char draw_enemy(char sprite_offset, char enemyIndex, int x, int y, char moving)
{
    char frame;
    char ctrl_flags = SZ_16x16;
    int dx, dy; // display x, display y

    frame = enemies[enemyIndex].frame;
    dx = x - sx;
    dy = y - sy;

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
        case DIR_FRAME_OVERRIDE:
            break;
        }

        if (moving && timer >> 3)
        {
            frame += 2;
        }
    }

    spr_set(TOP_HALF_START + sprite_offset);
    spr_x(dx);
    spr_y(dy - 16);
    spr_pattern(enemy_vram[enemies[enemyIndex].type] + (frame * SPR_SIZE_16x16));
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

    spr_set(BOTTOM_HALF_START + sprite_offset);
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

    return sprite_offset + 1;
}

#define SPRITE_START 2;

draw_boss2()
{
    spr_set(BOSS_2_EXTRA);
    spr_x((last_ava_x << 4) - sx);
    spr_y((last_ava_y << 4) - sy);
}

scroll_enemies(signed char dx, signed char dy)
{
    char i, offset;
    offset = SPRITE_START;
    for (i = 0; i < enemy_count; i++)
    {
        if (!enemies[i].active)
        {
            spr_set(TOP_HALF_START + offset);
            spr_hide();
            spr_set(BOTTOM_HALF_START + offset);
            spr_hide();
            offset = offset + 1;
            continue;
        }
        spr_set(TOP_HALF_START + offset);
        spr_x(spr_get_x() + dx);
        spr_y(spr_get_y() + dy);
        spr_set(BOTTOM_HALF_START + offset);
        spr_x(spr_get_x() + dx);
        spr_y(spr_get_y() + dy);
        offset = offset + 1;
    }
}

quick_draw_enemies()
{
    char i, offset;
    int dx, dy;

    if (!enemy_count)
    {
        return;
    }

    offset = SPRITE_START;
    for (i = 0; i < enemy_count; i++)
    {
        if (!enemies[i].active)
        {
            spr_set(TOP_HALF_START + offset);
            spr_hide();
            spr_set(BOTTOM_HALF_START + offset);
            spr_hide();
            offset = offset + 1;
            continue;
        }
        dx = (enemies[i].x * 16) - sx;
        dy = (enemies[i].y * 16) - sy;
        if (dx < 0 || dx > 256 || dy < 0 || dy > 256)
        {
            offset = offset + 2;
            continue;
        }
        spr_set(TOP_HALF_START + offset);
        spr_x(dx);
        spr_y(dy - 16);

        spr_set(BOTTOM_HALF_START + offset);
        spr_x(dx);
        spr_y(dy);

        offset = offset + 1;
    }
}

draw_enemies(char time_offset)
{
    char i, offset, j;
    int ex, ey, edx, edy;

    if (!enemy_count)
    {
        return;
    }

    offset = SPRITE_START;
    for (i = 0; i < enemy_count; i++)
    {
        ex = enemies[i].x * 16;
        ey = enemies[i].y * 16;
        edx = enemies[i].delx * time_offset;
        edy = enemies[i].dely * time_offset;
        offset = draw_enemy(
            offset,
            i,
            ex + edx,
            ey + edy,
            edx != 0 || edy != 0);
    }
    for (i = enemy_count; i < MAX_ENEMY_COUNT; i++)
    {
        spr_set(TOP_HALF_START + offset);
        spr_hide();
        spr_set(BOTTOM_HALF_START + offset);
        spr_hide();
        offset = offset + 1;
    }
    return;
}

char current_snd;

play_sound(char new_sound)
{
    if (new_sound > current_snd)
    {
        current_snd = new_sound;
    }
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
        switch (enemies[index].facing)
        {
        case UP:
            create_enemy(TYPE_BALL, enemies[index].x, enemies[index].y, enemies[index].facing, 0, -1);
            break;
        case DOWN:
            create_enemy(TYPE_BALL, enemies[index].x, enemies[index].y, enemies[index].facing, 0, 1);
            break;
        case LEFT:
            create_enemy(TYPE_BALL, enemies[index].x, enemies[index].y, enemies[index].facing, -1, 0);
            break;
        case RIGHT:
            create_enemy(TYPE_BALL, enemies[index].x, enemies[index].y, enemies[index].facing, +1, 0);
            break;
        }
        play_sound(ENEMY_CANNON);
    }

    if (enemies[index].timer > 3)
    {
        enemies[index].timer = 0;
    }
}

update_ball(char index)
{
    if (enemies[index].x == ava_x && enemies[index].y == ava_y)
    {
        kill_ava();
        return;
    }
    switch (enemies[index].facing)
    {
    case UP:
        if (!is_solid(enemies[index].x, enemies[index].y - 1, 0))
        {
            enemies[index].dely = -1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    case DOWN:
        if (!is_solid(enemies[index].x, enemies[index].y + 1, 0))
        {
            enemies[index].dely = 1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    case LEFT:
        if (!is_solid(enemies[index].x - 1, enemies[index].y, 0))
        {
            enemies[index].delx = -1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    case RIGHT:
        if (!is_solid(enemies[index].x + 1, enemies[index].y, 0))
        {
            enemies[index].delx = 1;
        }
        else
        {
            enemies[index].active = 0;
        }
        break;
    }
    if (
        (enemies[index].x + enemies[index].delx) == ava_x &&
        (enemies[index].y + enemies[index].dely) == ava_y)
    {
        ava_dead = 1;
        return;
    }
}

update_eyewalk(char index, char pause_on_turn)
{
    if (enemies[index].x == ava_x && enemies[index].y == ava_y)
    {
        kill_ava();
    }

eyewalk_loop:
    switch (enemies[index].facing)
    {
    case UP:
        if (!is_solid(enemies[index].x, enemies[index].y - 1, 0))
        {
            enemies[index].dely = -1;
        }
        else
        {
            enemies[index].facing = RIGHT;
            if (!pause_on_turn)
            {
                goto eyewalk_loop;
            }
        }
        break;
    case DOWN:
        if (!is_solid(enemies[index].x, enemies[index].y + 1, 0))
        {
            enemies[index].dely = 1;
        }
        else
        {
            enemies[index].facing = LEFT;
            if (!pause_on_turn)
            {
                goto eyewalk_loop;
            }
        }
        break;
    case LEFT:
        if (!is_solid(enemies[index].x - 1, enemies[index].y, 0))
        {
            enemies[index].delx = -1;
        }
        else
        {
            enemies[index].facing = UP;
            if (!pause_on_turn)
            {
                goto eyewalk_loop;
            }
        }
        break;
    case RIGHT:
        if (!is_solid(enemies[index].x + 1, enemies[index].y, 0))
        {
            enemies[index].delx = 1;
        }
        else
        {
            enemies[index].facing = DOWN;
            if (!pause_on_turn)
            {
                goto eyewalk_loop;
            }
        }
        break;
    }
    if (
        (enemies[index].x + enemies[index].delx) == ava_x &&
        (enemies[index].y + enemies[index].dely) == ava_y)
    {
        ava_dead = 1;
        return;
    }
}

update_boss1(char index)
{
    char i, delx, dely, flipx, flipy, ideal_facing;

    if (enemies[index].facing == DIR_FRAME_OVERRIDE && enemies[index].timer >= 5)
    {
        for (i = 0; i < enemy_count; i++)
        {
            enemies[i].active = 0;
        }
        create_object(1, 26, 13);
        create_object(0, 28, 13);
        cd_playtrk(TRACK_BALLAD, TRACK_BALLAD + 1, CDPLAY_REPEAT);
        play_sound(ENEMY_MINIWILHELM);
        return;
    }

    for (i = 0; i < enemy_count; i++)
    {
        if (
            enemies[i].active &&
            enemies[i].type == TYPE_BALL &&
            enemies[i].x == enemies[index].x &&
            enemies[i].y == enemies[index].y)
        {
            enemies[i].active = 0;
            hurt_boss1(index);
            return;
        }
    }

    if (enemies[index].x == ava_x && enemies[index].y == ava_y)
    {
        kill_ava();
    }

    delx = enemies[index].x - ava_x;
    dely = enemies[index].y - ava_y;

    if (delx > 128)
    {
        flipx = 1;
        delx = 255 - delx + 1;
    }
    else
    {
        flipx = 0;
    }

    if (dely > 128)
    {
        flipy = 1;
        dely = 255 - dely + 1;
    }
    else
    {
        flipy = 0;
    }

    if (delx > dely)
    {
        if (flipx)
        {
            ideal_facing = RIGHT;
        }
        else
        {
            ideal_facing = LEFT;
        }
    }
    else
    {
        if (flipy)
        {
            ideal_facing = DOWN;
        }
        else
        {
            ideal_facing = UP;
        }
    }

    if (enemies[index].facing == DIR_FRAME_OVERRIDE)
    {
        enemies[index].facing = ideal_facing;
        enemies[index].frame = 0;
    }

    if (enemies[index].facing == ideal_facing)
    {
        switch (enemies[index].facing)
        {
        case UP:
            if (!is_solid(enemies[index].x, enemies[index].y - 1, 0))
            {
                enemies[index].dely = -1;
            }
            break;
        case DOWN:
            if (!is_solid(enemies[index].x, enemies[index].y + 1, 0))
            {
                enemies[index].dely = 1;
            }
            break;
        case LEFT:
            if (!is_solid(enemies[index].x - 1, enemies[index].y, 0))
            {
                enemies[index].delx = -1;
            }
            break;
        case RIGHT:
            if (!is_solid(enemies[index].x + 1, enemies[index].y, 0))
            {
                enemies[index].delx = 1;
            }
            break;
        }
    }
    else
    {
        enemies[index].facing = ideal_facing;
    }

    if (
        (enemies[index].x + enemies[index].delx) == ava_x &&
        (enemies[index].y + enemies[index].dely) == ava_y)
    {
        ava_dead = 1;
        return;
    }
    return;
}

update_boss2(char index)
{
    char i;

    enemies[index].timer++;
    enemies[index].frame = (((enemies[index].timer) >> 2 & 1) << 1);

    if (enemies[index].x == ava_x && enemies[index].y == ava_y)
    {
        kill_ava();
        return;
    }

    for (i = 0; i < enemy_count; i++)
    {
        if (
            enemies[i].active &&
            enemies[i].type == TYPE_BALL &&
            (enemies[i].x + enemies[i].delx) == enemies[index].x &&
            (enemies[i].y + enemies[i].dely) == enemies[index].y)
        {
            for (i = 0; i < enemy_count; i++)
            {
                enemies[i].active = 0;
            }
            create_object(1, 31, 17);
            create_object(0, 33, 17);
            cd_playtrk(TRACK_BALLAD, TRACK_BALLAD + 1, CDPLAY_REPEAT);
            play_sound(ENEMY_MINIWILHELM);
            return;
        }
    }
}

check_boss1_hurt(char index)
{
    char i;

    if (!enemies[index].active)
    {
        return;
    }

    for (i = 0; i < enemy_count; i++)
    {
        if (
            enemies[i].active &&
            enemies[i].type == TYPE_BALL &&
            (enemies[i].x + enemies[i].delx) == enemies[index].x &&
            (enemies[i].y + enemies[i].dely) == enemies[index].y)
        {
            enemies[i].active = 0;
            hurt_boss1(index);
            return;
        }
    }
}

hurt_boss1(char index)
{
    enemies[index].delx = 0;
    enemies[index].dely = 0;
    enemies[index].facing = DIR_FRAME_OVERRIDE;
    enemies[index].frame = 12;
    enemies[index].timer += 1;
    play_sound(ENEMY_MINIWILHELM);
}

update_enemies()
{
    char i, j, any_moved;

    any_moved = 0;
    ava_dead = 0;
    current_snd = 0;

    for (i = 0; i < enemy_count; i++)
    {
        if (!enemies[i].active)
        {
            continue;
        }
        enemies[i].delx = 0;
        enemies[i].dely = 0;
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
        case TYPE_EYEWALK:
        {
            update_eyewalk(i, 1);
            break;
        }
        case TYPE_BOSS1:
        {
            update_boss1(i);
            break;
        }
        case TYPE_BOSS2:
        {
            update_boss2(i);
            break;
        }
        case TYPE_GOONGUY:
        {
            update_eyewalk(i, 0);
            break;
        }
        }
        if (!any_moved && enemies[i].delx != 0 || enemies[i].dely != 0)
        {
            any_moved = 1;
        }
    }

    if (boss1_index)
    {
        check_boss1_hurt(boss1_index);
    }

    if (any_moved)
    {
        for (j = 2; j < 16; j += 2)
        {
            draw_enemies(j);
            satb_update();
            wait_for_sync(1);
        }
        for (i = 0; i < enemy_count; i++)
        {
            if (!enemies[i].active)
            {
                continue;
            }
            if (enemies[i].delx)
            {
                enemies[i].x += enemies[i].delx;
            }
            if (enemies[i].dely)
            {
                enemies[i].y += enemies[i].dely;
            }
        }
    }

    switch (current_snd)
    {
    case ENEMY_CANNON:
        ad_play(CANNON_LOC, CANNON_SIZE, 14, 0);
        break;
    case ENEMY_MINIWILHELM:
        ad_play(WILHELM_LOC, MINIWILHELM_SIZE, 14, 0);
        break;
    }

    if (ava_dead)
    {
        kill_ava();
    }
}
