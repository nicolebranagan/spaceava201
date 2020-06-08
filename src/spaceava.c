/*
	sgx_test.c being mangled
*/

#include <huc.h>

#incbin(pal,"pce_pal1.bin");
#incspr(chr, "train.png");
#incpal(trainpal,"train.png")

main()
{
	unsigned char joyt;
	unsigned char d_on, sd_on;
	char i,j,k,l,m,n;
	char logox = 0;
	
	disp_off();
	spr_set();
	spr_hide();
	set_screen_size(SCR_SIZE_32x32);

	init_satb();
	load_vram(0x5000,chr,0x400);
	spr_set(0);
	spr_x(logox);
	spr_y(16);
	spr_pattern(0x5000);
	spr_ctrl(FLIP_MAS|SIZE_MAS,SZ_16x16);
	spr_pal(0);
	spr_pri(1);

	disp_on();
	
	set_font_pal(4);
	set_font_color(14,0);
	load_default_font();
	
	load_palette(0, pal,16);
	load_palette(16,trainpal,1);
				
	put_string("I  = Toggle normal display", 2, 8);
	put_string("I added this string", 2, 9);
	d_on = sd_on = 1;
	for(;;)
	{
		for( j=0; j<0xff; j++)
		{
			vsync();
			satb_update();
			logox++;
			spr_x(logox);
			
			joyt = joytrg(0);
			if (joyt & JOY_I) {
				d_on = !d_on;
				if (d_on)
					disp_on();
				else
					disp_off();
			}
		}
	}		
	
	
}