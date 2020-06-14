/*
	Classic style Space Ava grid-based gameplay
*/

#include <huc.h>

#incspr(avachr, "images/ava.png");
#incpal(avapal,"images/ava.png");

#define SPR_SIZE_16x16 0x40

initialize() {
    disp_off();
	spr_set();
	spr_hide();
	set_screen_size(SCR_SIZE_64x32);
    ad_reset();
	init_satb();

    load_vram(0x5000, avachr, SPR_SIZE_16x16 * 16);
    load_palette(16, avapal, 1);

    spr_set(0);
    spr_x(16);
    spr_y(16);
    spr_pattern(0x5000);
    spr_ctrl(FLIP_MAS|SIZE_MAS,SZ_16x16);
    spr_pal(0);
	spr_pri(1);

    spr_set(1);
    spr_x(16);
    spr_y(32);
    spr_pattern(0x5000 + SPR_SIZE_16x16);
    spr_ctrl(FLIP_MAS|SIZE_MAS,SZ_16x16);
    spr_pal(0);
	spr_pri(1);

    disp_on();
}

main() {
    initialize();

    for(;;)
	{
		vsync();
		satb_update();
    }
}
