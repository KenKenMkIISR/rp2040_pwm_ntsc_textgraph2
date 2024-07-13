//�@�e�L�X�g�{�O���t�B�b�N�r�f�I�o�͗p���C�u�����@by K.Tanaka

#include "rp2040_pwm_ntsc_textgraph.h"

void g_pset(int x, int y, int c)
{
  if((unsigned int)x>=FRAME_WIDTH) return;
  if((unsigned int)y>=FRAME_HEIGHT) return;
  GVRAM[y*FRAME_WIDTH+x]=c;
}

void g_putbmpmn(int x,int y,char m,char n,const unsigned char bmp[])
// ��m*�cn�h�b�g�̃L�����N�^�[�����Wx,y�ɕ\��
// unsigned char bmp[m*n]�z��ɁA�P���ɃJ���[�ԍ�����ׂ�
// �J���[�ԍ���0�̕����͓����F�Ƃ��Ĉ���
{
	int i,j,k;
	unsigned char *vp;
	const unsigned char *p;
	unsigned short *vph;

	if(x<=-m || x>X_RES || y<=-n || y>=Y_RES) return; //��ʊO
	if(y<0){ //��ʏ㕔�ɐ؂��ꍇ
		i=0;
		p=bmp-y*m;
	}
	else{
		i=y;
		p=bmp;
	}
	for(;i<y+n;i++){
		if(i>=Y_RES) return; //��ʉ����ɐ؂��ꍇ
		if(x<0){ //��ʍ��ɐ؂��ꍇ�͎c�镔���̂ݕ`��
			j=0;
			p+=-x;
			vp=GVRAM+i*X_RES;
		}
		else{
			j=x;
			vp=GVRAM+i*X_RES+x;
		}
		for(;j<x+m;j++){
			if(j>=X_RES){ //��ʉE�ɐ؂��ꍇ
				p+=x+m-j;
				break;
			}
			if(*p!=0){ //�J���[�ԍ���0�̏ꍇ�A�����Ƃ��ď���
				*vp=*p;
			}
			p++;
			vp++;
		}
	}
}

void g_clrbmpmn(int x,int y,char m,char n)
// �cm*��n�h�b�g�̃L�����N�^�[����
// �J���[0�œh��Ԃ�
{
	int i,j,k;
	unsigned char *vp;
	unsigned short mask,*vph;

	if(x<=-m || x>=X_RES || y<=-n || y>=Y_RES) return; //��ʊO
	if(y<0){ //��ʏ㕔�ɐ؂��ꍇ
		i=0;
	}
	else{
		i=y;
	}
	for(;i<y+n;i++){
		if(i>=Y_RES) return; //��ʉ����ɐ؂��ꍇ
		if(x<0){ //��ʍ��ɐ؂��ꍇ�͎c�镔���̂ݕ`��
			j=0;
			vp=GVRAM+i*X_RES;
		}
		else{
			j=x;
			vp=GVRAM+i*X_RES+x;
		}
		for(;j<x+m;j++){
			if(j>=X_RES){ //��ʉE�ɐ؂��ꍇ
				break;
			}
			*vp++=0;
		}
	}
}

void g_gline(int x1,int y1,int x2,int y2,unsigned int c)
// (x1,y1)-(x2,y2)�ɃJ���[c�Ő�����`��
{
	int sx,sy,dx,dy,i;
	int e;

	if(x2>x1){
		dx=x2-x1;
		sx=1;
	}
	else{
		dx=x1-x2;
		sx=-1;
	}
	if(y2>y1){
		dy=y2-y1;
		sy=1;
	}
	else{
		dy=y1-y2;
		sy=-1;
	}
	if(dx>=dy){
		e=-dx;
		for(i=0;i<=dx;i++){
			g_pset(x1,y1,c);
			x1+=sx;
			e+=dy*2;
			if(e>=0){
				y1+=sy;
				e-=dx*2;
			}
		}
	}
	else{
		e=-dy;
		for(i=0;i<=dy;i++){
			g_pset(x1,y1,c);
			y1+=sy;
			e+=dx*2;
			if(e>=0){
				x1+=sx;
				e-=dy*2;
			}
		}
	}
}
void g_circle(int x0,int y0,unsigned int r,unsigned int c)
// (x0,y0)�𒆐S�ɁA���ar�A�J���[c�̉~��`��
{
	int x,y,f;
	x=r;
	y=0;
	f=-2*r+3;
	while(x>=y){
		g_pset(x0-x,y0-y,c);
		g_pset(x0-x,y0+y,c);
		g_pset(x0+x,y0-y,c);
		g_pset(x0+x,y0+y,c);
		g_pset(x0-y,y0-x,c);
		g_pset(x0-y,y0+x,c);
		g_pset(x0+y,y0-x,c);
		g_pset(x0+y,y0+x,c);
		if(f>=0){
			x--;
			f-=x*4;
		}
		y++;
		f+=y*4+2;
	}
}
void g_hline(int x1,int x2,int y,unsigned int c)
// (x1,y)-(x2,y)�̐������C�����J���[c�ō����`��
{
	int temp;
	unsigned int d,*ad;
	unsigned short dh,*adh;

	if(y<0 || y>=Y_RES) return;
	if(x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	if(x2<0 || x1>=X_RES) return;
	if(x1<0) x1=0;
	if(x2>=X_RES) x2=X_RES-1;
	while(x1&3){
		g_pset(x1++,y,c);
		if(x1>x2) return;
	}
	d=c|(c<<8)|(c<<16)|(c<<24);
	ad=(unsigned int *)(GVRAM+y*X_RES+x1);
	while(x1+3<=x2){
		*ad++=d;
		x1+=4;
	}
	while(x1<=x2) g_pset(x1++,y,c);
}

void g_boxfill(int x1,int y1,int x2,int y2,unsigned int c)
// (x1,y1),(x2,y2)��Ίp���Ƃ���J���[c�œh��ꂽ�����`��`��
{
	int temp;

	if(x1>x2){
		temp=x1;
		x1=x2;
		x2=temp;
	}
	if(x2<0 || x1>=X_RES) return;
	if(y1>y2){
		temp=y1;
		y1=y2;
		y2=temp;
	}
	if(y2<0 || y1>=Y_RES) return;
	if(y1<0) y1=0;
	if(y2>=Y_RES) y2=Y_RES-1;
	while(y1<=y2){
		g_hline(x1,x2,y1++,c);
	}
}
void g_circlefill(int x0,int y0,unsigned int r,unsigned int c)
// (x0,y0)�𒆐S�ɁA���ar�A�J���[c�œh��ꂽ�~��`��
{
	int x,y,f;
	x=r;
	y=0;
	f=-2*r+3;
	while(x>=y){
		g_hline(x0-x,x0+x,y0-y,c);
		g_hline(x0-x,x0+x,y0+y,c);
		g_hline(x0-y,x0+y,y0-x,c);
		g_hline(x0-y,x0+y,y0+x,c);
		if(f>=0){
			x--;
			f-=x*4;
		}
		y++;
		f+=y*4+2;
	}
}
void g_putfont(int x,int y,unsigned int c,int bc,unsigned char n)
//8*8�h�b�g�̃A���t�@�x�b�g�t�H���g�\��
//���W�ix,y)�A�J���[�ԍ�c
//bc:�o�b�N�O�����h�J���[�A�����̏ꍇ����
//n:�����ԍ�
{
	int i,j,k;
	unsigned char d;
	const unsigned char *p;
	unsigned int d1,mask;
	unsigned short *ad;

	p=FontData+n*8;
	for(i=0;i<8;i++){
		d=*p++;
		for(j=0;j<8;j++){
			if(d&0x80) g_pset(x+j,y+i,c);
			else if(bc>=0) g_pset(x+j,y+i,bc);
			d<<=1;
		}
	}
}

void g_printstr(int x,int y,unsigned int c,int bc,unsigned char *s){
	//���W(x,y)����J���[�ԍ�c�ŕ�����s��\���Abc:�o�b�N�O�����h�J���[
	while(*s){
		g_putfont(x,y,c,bc,*s++);
		x+=8;
	}
}
void g_printnum(int x,int y,unsigned char c,int bc,unsigned int n){
	//���W(x,y)�ɃJ���[�ԍ�c�Ő��ln��\���Abc:�o�b�N�O�����h�J���[
	unsigned int d,e;
	d=10;
	e=0;
	while(n>=d){
		e++;
		if(e==9) break;
		d*=10;
	}
	x+=e*8;
	do{
		g_putfont(x,y,c,bc,'0'+n%10);
		n/=10;
		x-=8;
	}while(n!=0);
}
void g_printnum2(int x,int y,unsigned char c,int bc,unsigned int n,unsigned char e){
	//���W(x,y)�ɃJ���[�ԍ�c�Ő��ln��\���Abc:�o�b�N�O�����h�J���[�Ae���ŕ\��
	if(e==0) return;
	x+=(e-1)*8;
	do{
		g_putfont(x,y,c,bc,'0'+n%10);
		e--;
		n/=10;
		x-=8;
	}while(e!=0 && n!=0);
	while(e!=0){
		g_putfont(x,y,c,bc,' ');
		x-=8;
		e--;
	}
}
unsigned int g_color(int x,int y){
	//���W(x,y)��VRAM��̌��݂̃p���b�g�ԍ���Ԃ��A��ʊO��0��Ԃ�
	unsigned short *ad;

	if((unsigned int)x>=(unsigned int)X_RES) return 0;
	if((unsigned int)y>=(unsigned int)Y_RES) return 0;
	return *(GVRAM+y*X_RES+x);
}

unsigned char *cursor=TVRAM;
unsigned char cursorcolor=7;

void vramscroll(void){
	unsigned char *p1,*p2,*vramend;

	vramend=TVRAM+WIDTH_X*WIDTH_Y;
	p1=TVRAM;
	p2=p1+WIDTH_X;
	while(p2<vramend){
		*(p1+ATTROFFSET)=*(p2+ATTROFFSET);
		*p1++=*p2++;
	}
	while(p1<vramend){
		*(p1+ATTROFFSET)=0;
		*p1++=0;
	}
}
void setcursor(unsigned char x,unsigned char y,unsigned char c){
	//�J�[�\�������W(x,y)�ɃJ���[�ԍ�c�ɐݒ�
	if(x>=WIDTH_X || y>=WIDTH_Y) return;
	cursor=TVRAM+y*WIDTH_X+x;
	cursorcolor=c;
}
void setcursorcolor(unsigned char c){
	//�J�[�\���ʒu���̂܂܂ŃJ���[�ԍ���c�ɐݒ�
	cursorcolor=c;
}
void printchar(unsigned char n){
	//�J�[�\���ʒu�Ƀe�L�X�g�R�[�hn��1�����\�����A�J�[�\����1�����i�߂�
	//��ʍŏI�����\�����Ă��X�N���[�������A���̕����\�����ɃX�N���[������
	if(cursor<TVRAM || cursor>TVRAM+WIDTH_X*WIDTH_Y) return;
	if(cursor==TVRAM+WIDTH_X*WIDTH_Y){
		vramscroll();
		cursor-=WIDTH_X;
	}
	if(n=='\n'){
		//���s
		cursor+=WIDTH_X-((cursor-TVRAM)%WIDTH_X);
	} else if(n==0x08){
		//BS
		if (TVRAM<cursor) cursor--;
	} else{
		*cursor=n;
		*(cursor+ATTROFFSET)=cursorcolor;
		cursor++;
	}
}
void printstr(unsigned char *s){
	//�J�[�\���ʒu�ɕ�����s��\��
	while(*s){
		printchar(*s++);
	}
}
void printnum(unsigned int n){
	//�J�[�\���ʒu�ɕ����Ȃ�����n��10�i���\��
	unsigned int d,n1;
	n1=n/10;
	d=1;
	while(n1>=d){
		d*=10;
	}
	while(d!=0){
		printchar('0'+n/d);
		n%=d;
		d/=10;
	}
}
void printnum2(unsigned int n,unsigned char e){
	//�J�[�\���ʒu�ɕ����Ȃ�����n��e����10�i���\���i�O�̋󂫌������̓X�y�[�X�Ŗ��߂�j
	unsigned int d,n1;
	if(e==0) return;
	n1=n/10;
	d=1;
	e--;
	while(e>0 && n1>=d){
		d*=10;
		e--;
	}
	if(e==0 && n1>d) n%=d*10;
	for(;e>0;e--) printchar(' ');
	while(d!=0){
		printchar('0'+n/d);
		n%=d;
		d/=10;
	}
}

void cls(void){
	//�e�L�X�g��ʂ�0�ŃN���A���A�J�[�\������ʐ擪�Ɉړ�
	clearscreen();
	cursor=TVRAM;
}
