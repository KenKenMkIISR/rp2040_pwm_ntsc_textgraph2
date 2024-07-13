#include "rp2040_pwm_ntsc_textgraph.h"
#include "text_graph_library.h"
#include <stdlib.h>

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
		g_boxfill(c*42, 0, c*42+41, FRAME_HEIGHT/2-1, 7-c);
		g_boxfill(c*42, FRAME_HEIGHT/2, c*42+41, FRAME_HEIGHT-1, 15-c);
	  }
	}
  }
  else if(counter<200){
	if(counter==100){
	  g_clearscreen();
	  cls();
	}
	setcursor(0, 0, 7);
	printnum(counter);
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
    rp2040_pwm_ntsc_init();
    while(1){
        demo();
    }
}