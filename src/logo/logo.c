/*
	sgx_test.c being mangled
*/

#include <huc.h>

#incspr(chr, "images/train.png");
#incpal(trainpal,"images/train.png");

#incchr(logo_gfx, "images/logo.png");
#incpal(logo_pal, "images/logo.png");
#incbat(logo_bat, "images/logo.png", 0x1000);

main()
{
	unsigned char joyt;
	char logox = 0;
	char timer = 0;
	int i;
	
	disp_off();
	spr_set();
	spr_hide();
	set_screen_size(SCR_SIZE_32x32);

	ad_reset();
	init_satb();
	load_vram(0x5000,chr,0x200);
	spr_set(0);
	spr_x(logox);
	spr_y(80);
	spr_pattern(0x5000);
	spr_ctrl(FLIP_MAS|SIZE_MAS,SZ_16x16);
	spr_pal(0);
	spr_pri(1);

	load_background(logo_gfx, logo_pal, logo_bat, 32, 28);
	ad_trans(2, 0, 5, 0);
	disp_on();
	load_palette(16,trainpal,1);
				
	for(;;)
	{
		vsync();
		satb_update();
		logox++;
		spr_x(logox);
		joyt = joy(0);
		if (!(joyt & JOY_I)) {
			timer++;
			if (timer == 10) {
				spr_pattern(0x5000);
			} else if (timer == 20) {
				spr_pattern(0x5040);
				timer = 0;
			}
		}

		if (logox == 70 || logox == 150) {
			ad_play(0, 0x2370, 15, 0);
		}
	}		
}