/*
	sgx_test.c being mangled
*/

#include <huc.h>

#incbin(pal,"pce_pal1.bin");
#incspr(chr, "train.png");
#incpal(trainpal,"train.png");
#incchr(font, "font.png")

main()
{
	unsigned char joyt;
	char logox = 0;
	char timer = 0;
	
	disp_off();
	spr_set();
	spr_hide();
	set_screen_size(SCR_SIZE_32x32);

	init_satb();
	load_vram(0x5000,chr,0x200);
	spr_set(0);
	spr_x(logox);
	spr_y(64);
	spr_pattern(0x5000);
	spr_ctrl(FLIP_MAS|SIZE_MAS,SZ_16x16);
	spr_pal(0);
	spr_pri(1);

	disp_on();
	
	set_font_pal(4);
	set_font_color(14,0);
	load_font(font, 128);
	
	load_palette(0, pal,16);
	load_palette(16,trainpal,1);
				
	put_string("I added this string", 2, 12);
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
	}		
}