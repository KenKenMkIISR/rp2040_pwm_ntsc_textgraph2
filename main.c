#include "rp2040_pwm_ntsc_textgraph.h"
#include "text_graph_library.h"
#include <stdlib.h>
#include "pico/stdlib.h"

uint8_t framebuffer[X_RES*Y_RES] __attribute__ ((aligned (4)));
uint8_t fontram[256*8];

void demo(void)
{
  static int counter=0;
  static uint8_t ch=0x21;
  drawcount=0;
  counter++;
  if(counter<100){
	if(counter==1){
	  g_clearscreen();
	  cls();
	  for(int c=0;c<8;c++){
		g_boxfill(c*42, 0, c*42+41, Y_RES/2-1, 7-c);
		g_boxfill(c*42, Y_RES/2, c*42+41, Y_RES-1, 15-c);
	  }
	}
  }
  else if(counter<200){
	if(counter==100){
	  g_clearscreen();
	  cls();
	}
	setcursor(0, 0, 7);
	printnum(counter-100+1);
	g_gline((rand()&255), (rand()&255), (rand()&255)+100, (rand()&255), (rand()&15)+1);
  }
  else if(counter<300){
	if(counter==200){
	  g_clearscreen();
	  cls();
	}
	setcursor(0, 0, 7);
	printnum(counter-200+1);
	g_circlefill((rand()&255)+50, (rand()&127)+50, (rand()&63)+20, (rand()&15)+1);
  }
  else if(counter<600){
	if(counter==300){
	  printchar('\n');
	}
	if((counter&3)==0){
	  for(int i=0;i<WIDTH_X;i++){
		setcursorcolor(rand()&7);
		printchar(ch++);
		if(ch>0xf7) ch=0x21;
	  }
	}
  }
  else counter=0;
  while(drawcount<1) asm("wfi");
}

void main(void){
    init_composite(19);
	startPCG(fontram,1);
	uint8_t d=0x0f;
	for(int i=0;i<8;i++){
		fontram[0x80*8+i]=d;
		d=(d<<1)|(d>>7);
	}
	while(1){
		set_videomode(VMODE_WIDEGRPH,framebuffer);
		for(int i=0;i<600;i++) demo();
		set_videomode(VMODE_WIDETEXT,0);
		set_palette(256,255,0,0);// set background color blue
		cls();
		setcursorcolor(7);
		uint8_t c='A';
		for(int i=0;i<WIDTH_X*14;i++){
			printchar(c);
			if(c=='Z') c='a';
			else if(c=='z') c='A';
			else c++;
		}
		for(int i=0;i<WIDTH_X*13;i++){
			printchar(0x80);
		}
		drawcount=0;
		while(drawcount<60*5) asm("wfi");
		set_videomode(VMODE_MONOTEXT,0);
		cls();
		c='A';
		for(int i=0;i<WIDTH_XBW*7;i++){
			printchar(c);
			if(c=='Z') c='a';
			else if(c=='z') c='A';
			else c++;
		}
		setcursorcolor(0x80);
		for(int i=0;i<WIDTH_XBW*7;i++){
			printchar(c);
			if(c=='Z') c='a';
			else if(c=='z') c='A';
			else c++;
		}
		setcursorcolor(0);
		for(int i=0;i<WIDTH_XBW*7;i++){
			printchar(0x80);
		}
		setcursorcolor(0x80);
		for(int i=0;i<WIDTH_XBW*6;i++){
			printchar(0x80);
		}
		drawcount=0;
		while(drawcount<60*5) asm("wfi");
	}
}