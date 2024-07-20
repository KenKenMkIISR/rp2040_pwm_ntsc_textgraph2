#include "rp2040_pwm_ntsc_textgraph.h"
#include "text_graph_library.h"
#include <stdlib.h>

void raymain(void);
void main(void){
    rp2040_pwm_ntsc_init();
	//カラーパレット設定
	set_palette(0, 0, 0, 0);
	for (int br = 1; br <= 36; br++) {
		for (int i = 1; i <= 7; i++) {
			set_palette((br - 1)*7 + i, (i&1)*br*7+3, ((i&2)>>1)*br*7+3, ((i&4)>>2)*br*7+3);
		}
	}
	raymain(); // レイトレーシング実行
	while(1) asm("wfi");
}