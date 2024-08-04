#include "rp2040_pwm_ntsc_textgraph.h"
#include "text_graph_library.h"
#include <stdlib.h>


//グラフィック用メモリ
uint8_t framebuffer[X_RES*Y_RES] __attribute__ ((aligned (4)));

void raymain(void);

void main(void){
	init_composite(19); //GPIO19にビデオ出力開始
	set_videomode(VMODE_WIDEGRPH,framebuffer); //グラフィックモードに設定

	//カラーパレット設定
	set_palette(0, 0, 0, 0);
	for (int br = 1; br <= 36; br++) {
		for (int i = 1; i <= 7; i++) {
			set_palette((br - 1)*7 + i, (i&1)*br*7+3, ((i&2)>>1)*br*7+3, ((i&4)>>2)*br*7+3);
		}
	}
	g_clearscreen();
//	stop_composite(); // 映像出力を停止し高速化
	raymain(); // レイトレーシング実行
//	start_composite(); // 映像出力再開
	while(1) asm("wfi");
}