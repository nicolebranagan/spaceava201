/*
	sgx_test.c being mangled
*/

#include <huc.h>

#incbin(pal,"pce_pal1.bin");

main()
{
	unsigned char joyt;
	unsigned char d_on, sd_on;
	char i,j,k,l,m,n;
	
	disp_off();
	spr_set();
	spr_hide();
	load_default_font();
	set_screen_size(SCR_SIZE_32x32);
	disp_on();
	
	set_font_pal(4);
	set_font_color(14,0);
	load_default_font();
	
	load_palette(0, pal,16);
				
	put_string("I  = Toggle normal display", 2, 8);
	put_string("I added this string", 2, 9);
	d_on = sd_on = 1;
	for(;;)
	{
		for( j=0; j<0xff; j++)
		{
			vsync();
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