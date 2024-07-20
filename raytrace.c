// 256色レイトレーシング by K.Tanaka

#include <math.h>
#include <stdlib.h>
#include "text_graph_library.h"

#define IDX 1
#define IDY 1
//#define DITHER //定義するとディザが働く
#define XSIZE 336
#define YSIZE 216
#define ASPECT 1.11
#define DX (XSIZE/2)
#define DY (YSIZE/2/ASPECT)
#define DZ (1000*XSIZE/640)
#define DCC 252
#define ICC1 1
#define ICC2 3
#define CLEAR 8
#define CHECK 9
#define BOUND 10
#define PI 3.14159

typedef struct {
	float x, y, z;
} vector;

struct color {
	float br;
	int c;
};

vector lr = {50, 30, 0};
vector er = {-30, -35, 0};
vector e = {2000, -2000, -3000};

struct {
	int o;
	int cl, cm;
	float r;
	float ox, oy, oz;
	float xr, yr, zr;
	float x2, y2, z2;
	float x3, y3, z3;
} pr1[] = {
	{3, 2, 0, 0, 0, 0, 600, 0, 0, 0, 200, 0, 200, -200, -800, -200},
	{3, 4, 0, 0, 0, -100, 0, 0, 0, 0, 40, 40, 400, -40, -40, -400},
	{3, 2, 0, 0, 0, 0, -600, 0, 0, 0, 200, 0, 200, -200, -400, -200},
	{2, 6, 0, 200, 0, -600, -600},
	{0},
	{3, 6, 0, 0, 0, 0, 0, 0, 0, 0, -100, -200, -400, 100, 0, 400},
	{0},
	{3, 5, 0, 0, 0, 0, 0, 0, 45, 0, 200, 0, 200, -200, -100, -200},
	{0},
	{2, 2, 1, 250, 0, -250, 0},
	{1, CLEAR, 0, 0, 0, -250, 0, 45, 45, 0},
	{0},
	{2, 5, 0, 150, 0, -150, 0},
	{0},
	{3, CHECK, 0, 0, 0, 0, 0, 0, 0, 0, -1000, 100, -1000, 1000, 0, 1000},
	{0},
	{0}
};

struct {
	float x, y, z;
	float xr, yr, zr;
} w[] = {
	{-800, 50, 50, 0, 0, 0},
	{100, 50, -150, 0, 0, 0},
	{500, 50, 600, 0, 0, 0},
	{600, 50, 0, 0, 0, 0},
	{500, 50, -700, 0, 0, 0},
	{0, 50, 0, 0, 0, 0}
};

struct {
	float p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11;
	int o, cl, cm, no;
} pr[50];

struct {
	float p0, p1, p2, p3, p4;
	int o, cl, cm;
} obj[100];
vector l;
vector es, ev1, ev2;

int sign(float x) {
	if (x > 0) return 1;
	else if (x < 0) return -1;
	return 0;
}

vector normalize(vector x) {
	vector y;
	float t;

	t = sqrt(x.x * x.x + x.y * x.y + x.z * x.z);
	y.x = x.x / t;
	y.y = x.y / t;
	y.z = x.z / t;
	return y;
}

int plain(int in, vector r, vector f, vector *pp, float *tp) {
	float t;

	t = obj[in].p0 * r.x + obj[in].p1 * r.y + obj[in].p2 * r.z;
	if (t == 0) return 0;
	t = -(obj[in].p0 * f.x + obj[in].p1 * f.y + obj[in].p2 * f.z + obj[in].p3) / t;
	if (t <= 0) return 0;
	pp->x = r.x * t + f.x;
	pp->y = r.y * t + f.y;
	pp->z = r.z * t + f.z;
	*tp = t;
	return 1;
}

int sphere(int in, vector r, vector f, vector *pp1, float *tp1, vector *pp2, float *tp2) {
	float l, m, n, k, t1;

	l = obj[in].p0 - f.x;
	m = obj[in].p1 - f.y;
	n = obj[in].p2 - f.z;
	k = r.x * l + r.y * m + r.z*n;
	t1 = k * k - l * l - m * m - n * n + obj[in].p4;
	if (t1 < 0) return 0;
	t1 = sqrt(t1);
	*tp1 = k - t1;
	if (*tp1 > 0) {
		pp1->x = r.x * (*tp1) + f.x;
		pp1->y = r.y * (*tp1) + f.y;
		pp1->z = r.z * (*tp1) + f.z;
	}
	*tp2 = k + t1;
	if (*tp2 > 0) {
		pp2->x = r.x * (*tp2) + f.x;
		pp2->y = r.y * (*tp2) + f.y;
		pp2->z = r.z * (*tp2) + f.z;
		return 1;
	}
	return 0;
}

int check(int in, vector p) {
	float l, m, n;
	int im;

	for (im = obj[in].cm % 128;; im++) {
		if (im != in) {
			switch (obj[im].o) {
				case 1:
					if (obj[im].p0 * p.x + obj[im].p1 * p.y + obj[im].p2 * p.z + obj[im].p3 > 0)
						return 0;
					break;
				case 2:
					l = p.x - obj[im].p0;
					m = p.y - obj[im].p1;
					n = p.z - obj[im].p2;
					if (l * l + m * m + n * n - obj[im].p4 > 0) return 0;
					break;
			}
		}
		if (obj[im].cm >= 128) break;
	}
	return 1;
}

int cross(vector r, vector f, vector *n) {
	float tt, t, t1;
	vector pt, pt1;
	int in, ino = 0;

	tt = 1e+37;
	for (in = 1; obj[in].o != 0; in++) {
		if (obj[in].cl == CLEAR) continue;
		switch (obj[in].o) {
			case 1:
				if (plain(in, r, f, &pt, &t)) {
					if (t < tt && check(in, pt)) {
						tt = t;
						n->x = pt.x;
						n->y = pt.y;
						n->z = pt.z;
						ino = in;
					}
				}
				break;
			case 2:
				if (sphere(in, r, f, &pt, &t, &pt1, &t1)) {
					if (obj[in].cl == BOUND) break;
					if (t > 0 && t < tt && check(in, pt)) {
						tt = t;
						n->x = pt.x;
						n->y = pt.y;
						n->z = pt.z;
						ino = in;
					} else if (t1 < tt && check(in, pt1)) {
						tt = t1;
						n->x = pt1.x;
						n->y = pt1.y;
						n->z = pt1.z;
						ino = in;
					}
				} else
					while (obj[in].cm < 128) in++;
				break;
		}
	}
	return ino;
}

int shadow(int ino, float rn, vector n) {
	vector pt, pt1;
	float t, t1;
	int in;

	for (in = 1; obj[in].o != 0; in++) {
		if (obj[in].cl == CLEAR || obj[in].cl == BOUND) continue;
		switch (obj[in].o) {
			case 1:
				if (ino == in) continue;
				if (plain(in, l, n, &pt, &t)) {
					if (check(in, pt)) return 1;
				}
				break;
			case 2:
				if (ino == in && rn <= 0) continue;
				if (sphere(in, l, n, &pt, &t, &pt1, &t1)) {
					if (ino != in) {
						if (check(in, pt)) return 1;
					}
					if (check(in, pt1)) return 1;
				} else {
					while (obj[in].cm < 128) in++;
				}
				break;
		}
	}
	return 0;
}

vector nvect(int ino, vector n, float *lnp) {
	vector vn;

	switch (obj[ino].o) {
		case 1:
			vn.x = obj[ino].p0;
			vn.y = obj[ino].p1;
			vn.z = obj[ino].p2;
			*lnp = obj[ino].p4;
			break;
		case 2:
			vn.x = n.x - obj[ino].p0;
			vn.y = n.y - obj[ino].p1;
			vn.z = n.z - obj[ino].p2;
			vn = normalize(vn);
			*lnp = l.x * vn.x + l.y * vn.y + l.z * vn.z;
			break;
	}
	return vn;
}

float hilight(vector r, float ln, float rn) {
	float ht;

	ht = -2 * rn * ln + r.x * l.x + r.y * l.y + r.z * l.z;
	if (ht < 0.8) ht = 0;
	else ht = pow(ht, 4);
	return ht;
}

struct color color1(int ino, vector n, float br) {
	struct color cl;
	int c, t;
	if ((c = obj[ino].cl) == CHECK) {
		t = (int) (floor(n.x / DCC) + floor(n.y / DCC) + floor(n.z / DCC));
		if (t % 2) c = ICC1;
		else c = ICC2;
	}
	cl.br = br;
	cl.c = c;
	return cl;
}

struct color trace(vector r) {
	int ino;
	float ln, rn, ht = 0, br = 0;
	struct color cl;
	vector n, vn;

	if ((ino = cross(r, e, &n)) == 0) {
		cl.br = 0;
		cl.c = 0;
		return cl;
	}
	vn = nvect(ino, n, &ln);
	rn = r.x * vn.x + r.y * vn.y + r.z * vn.z;
	if (sign(rn) != sign(ln) && shadow(ino, rn, n) == 0) {
//		ht = hilight(r, ln, rn) + (float)(rand()%256)/320;
		ht = hilight(r, ln, rn);
		br = fabs(ln)*.9 + .1;
	}
	if (br < .25) br = .25;
	cl = color1(ino, n, br);
	br+=ht;
	if (br +(float)(rand()%256)/256/1.5-0.33> 1.5){
		cl.br=1;
		cl.c = 7;
	}
	return cl;
}

void dot(int x, int y, struct color cl) {
	int ix, iy;

#ifdef DITHER
	float r;
#endif

	for (iy = 0; iy < IDY; iy++) {
		for (ix = 0; ix < IDX; ix++) {

#ifdef DITHER
			r = (float) rand() / RAND_MAX;
			if (cl.br == 0) g_pset(x + ix, y + iy, 0);
			else g_pset(x + ix, y + iy, (int) (cl.br * 40 + r*0.95 - 5)*7 + cl.c);
#else
			if (cl.br == 0) g_pset(x + ix, y + iy, 0);
			else g_pset(x + ix, y + iy, (int) (cl.br * 40-5) *7 + cl.c);
#endif
		}
	}
}

float xrotate(float x, float y, float th) {
	return (x * cos(th * PI / 180) - y * sin(th * PI / 180));
}

float yrotate(float x, float y, float th) {
	return (x * sin(th * PI / 180) + y * cos(th * PI / 180));
}

vector rotate(vector v, float xr, float yr, float zr) {
	vector x;
	float x1, y1, z1;

	y1 = xrotate(v.y, v.z, xr);
	z1 = yrotate(v.y, v.z, xr);
	x.z = xrotate(z1, v.x, yr);
	x1 = yrotate(z1, v.x, yr);
	x.x = xrotate(x1, y1, zr);
	x.y = yrotate(x1, y1, zr);
	return x;
}

void dataread(void) {
	int in, im = 0, io = 0;
	vector xv, xv2, xv3;

	for (in = 0;; in++) {
		switch (pr1[in].o) {
			case 0:
				im++;
				if (pr1[in + 1].o == 0) goto label1;
				continue;
			case 1:
				pr[io].p0 = pr1[in].ox;
				pr[io].p1 = pr1[in].oy;
				pr[io].p2 = pr1[in].oz;
				xv.x = 0;
				xv.y = -1;
				xv.z = 0;
				xv = rotate(xv, pr1[in].xr, pr1[in].yr, pr1[in].zr);
				pr[io].p3 = xv.x + pr1[in].ox;
				pr[io].p4 = xv.y + pr1[in].oy;
				pr[io].p5 = xv.z + pr1[in].oz;
				break;
			case 2:
				pr[io].p0 = pr1[in].ox;
				pr[io].p1 = pr1[in].oy;
				pr[io].p2 = pr1[in].oz;
				pr[io].p3 = pr1[in].r;
				break;
			case 3:
				xv2.x = pr1[in].x2;
				xv2.y = pr1[in].y2;
				xv2.z = pr1[in].z2;
				xv3.x = pr1[in].x3;
				xv3.y = pr1[in].y3;
				xv3.z = pr1[in].z3;
				xv = rotate(xv2, pr1[in].xr, pr1[in].yr, pr1[in].zr);
				pr[io].p0 = xv.x + pr1[in].ox;
				pr[io].p1 = xv.y + pr1[in].oy;
				pr[io].p2 = xv.z + pr1[in].oz;
				xv.x = xv3.x;
				xv.y = xv2.y;
				xv.z = xv2.z;
				xv = rotate(xv, pr1[in].xr, pr1[in].yr, pr1[in].zr);
				pr[io].p3 = xv.x + pr1[in].ox;
				pr[io].p4 = xv.y + pr1[in].oy;
				pr[io].p5 = xv.z + pr1[in].oz;
				xv.x = xv2.x;
				xv.y = xv3.y;
				xv.z = xv2.z;
				xv = rotate(xv, pr1[in].xr, pr1[in].yr, pr1[in].zr);
				pr[io].p6 = xv.x + pr1[in].ox;
				pr[io].p7 = xv.y + pr1[in].oy;
				pr[io].p8 = xv.z + pr1[in].oz;
				xv.x = xv2.x;
				xv.y = xv2.y;
				xv.z = xv3.z;
				xv = rotate(xv, pr1[in].xr, pr1[in].yr, pr1[in].zr);
				pr[io].p9 = xv.x + pr1[in].ox;
				pr[io].p10 = xv.y + pr1[in].oy;
				pr[io].p11 = xv.z + pr1[in].oz;
				break;
		}
		pr[io].cl = pr1[in].cl;
		pr[io].cm = pr1[in].cm;
		pr[io].o = pr1[in].o;
		pr[io].no = im;
		io++;
	}
label1:
	pr[io].o = 0;
}

void setbox(int in, vector v, vector x, int cl, int ict) {
	v = normalize(v);
	obj[in].p0 = v.x;
	obj[in].p1 = v.y;
	obj[in].p2 = v.z;
	obj[in].p3 = -v.x * x.x - v.y * x.y - v.z * x.z;
	obj[in].p4 = v.x * l.x + v.y * l.y + v.z * l.z;
	obj[in].cl = cl;
	obj[in].cm = ict;
	obj[in].o = 1;
}

void move(void) {
	vector v, v1, v2, v3, vv;
	int in, im, inm, ict;

	l.x = 0;
	l.y = -1;
	l.z = 0;
	l = rotate(l, lr.x, lr.y, lr.z);
	v.x = DX;
	v.y = -DY;
	v.z = DZ;
	v2 = rotate(v, er.x, er.y, er.z);
	v.x = -DX;
	v.y = DY;
	v.z = DZ;
	v3 = rotate(v, er.x, er.y, er.z);
	v.x = -DX;
	v.y = -DY;
	v.z = DZ;
	es = rotate(v, er.x, er.y, er.z);
	ev1.x = v2.x - es.x;
	ev1.y = v2.y - es.y;
	ev1.z = v2.z - es.z;
	ev2.x = v3.x - es.x;
	ev2.y = v3.y - es.y;
	ev2.z = v3.z - es.z;
	in = 1;
	ict = 1;
	for (im = 0; pr[im].o != 0; im++) {
		inm = pr[im].no;
		switch (pr[im].o) {
			case 1:
				obj[in].o = 1;
				v.x = pr[im].p3;
				v.y = pr[im].p4;
				v.z = pr[im].p5;
				v2 = rotate(v, w[inm].xr, w[inm].yr, w[inm].zr);
				v.x = pr[im].p0;
				v.y = pr[im].p1;
				v.z = pr[im].p2;
				v = rotate(v, w[inm].xr, w[inm].yr, w[inm].zr);
				obj[in].p0 = v2.x - v.x;
				obj[in].p1 = v2.y - v.y;
				obj[in].p2 = v2.z - v.z;
				obj[in].p3 = -obj[in].p0 * (v.x + w[inm].x)
						- obj[in].p1 * (v.y + w[inm].y)
						- obj[in].p2 * (v.z + w[inm].z);
				obj[in].p4 = l.x * obj[in].p0 + l.y * obj[in].p1 + l.z * obj[in].p2;
				obj[in].cl = pr[im].cl;
				obj[in].cm = ict;
				in++;
				break;
			case 2:
				obj[in].o = 2;
				v.x = pr[im].p0;
				v.y = pr[im].p1;
				v.z = pr[im].p2;
				v = rotate(v, w[inm].xr, w[inm].yr, w[inm].zr);
				obj[in].p0 = v.x + w[inm].x;
				obj[in].p1 = v.y + w[inm].y;
				obj[in].p2 = v.z + w[inm].z;
				obj[in].p3 = pr[im].p3;
				obj[in].p4 = pr[im].p3 * pr[im].p3;
				obj[in].cl = pr[im].cl;
				obj[in].cm = ict;
				in++;
				break;
			case 3:
				v.x = pr[im].p0;
				v.y = pr[im].p1;
				v.z = pr[im].p2;
				v = rotate(v, w[inm].xr, w[inm].yr, w[inm].zr);
				v.x += w[inm].x;
				v.y += w[inm].y;
				v.z += w[inm].z;
				v1.x = pr[im].p3;
				v1.y = pr[im].p4;
				v1.z = pr[im].p5;
				v1 = rotate(v1, w[inm].xr, w[inm].yr, w[inm].zr);
				v1.x += w[inm].x;
				v1.y += w[inm].y;
				v1.z += w[inm].z;
				v2.x = pr[im].p6;
				v2.y = pr[im].p7;
				v2.z = pr[im].p8;
				v2 = rotate(v2, w[inm].xr, w[inm].yr, w[inm].zr);
				v2.x += w[inm].x;
				v2.y += w[inm].y;
				v2.z += w[inm].z;
				v3.x = pr[im].p9;
				v3.y = pr[im].p10;
				v3.z = pr[im].p11;
				v3 = rotate(v3, w[inm].xr, w[inm].yr, w[inm].zr);
				v3.x += w[inm].x;
				v3.y += w[inm].y;
				v3.z += w[inm].z;
				/* bouding sphere */
				obj[in].p0 = (v1.x + v2.x + v3.x - v.x) / 2;
				obj[in].p1 = (v1.y + v2.y + v3.y - v.y) / 2;
				obj[in].p2 = (v1.z + v2.z + v3.z - v.z) / 2;
				obj[in].p4 = (v.x - obj[in].p0)*(v.x - obj[in].p0)
						+(v.y - obj[in].p1)*(v.y - obj[in].p1)
						+(v.z - obj[in].p2)*(v.z - obj[in].p2);
				obj[in].p3 = sqrt(obj[in].p4);
				obj[in].o = 2;
				obj[in].cl = BOUND;
				obj[in].cm = ict;
				in++;
				/* */
				vv.x = v.x - v1.x;
				vv.y = v.y - v1.y;
				vv.z = v.z - v1.z;
				setbox(in++, vv, v, pr[im].cl, ict);
				vv.x = v.x - v2.x;
				vv.y = v.y - v2.y;
				vv.z = v.z - v2.z;
				setbox(in++, vv, v, pr[im].cl, ict);
				vv.x = v.x - v3.x;
				vv.y = v.y - v3.y;
				vv.z = v.z - v3.z;
				setbox(in++, vv, v, pr[im].cl, ict);
				vv.x = v1.x - v.x;
				vv.y = v1.y - v.y;
				vv.z = v1.z - v.z;
				setbox(in++, vv, v1, pr[im].cl, ict);
				vv.x = v2.x - v.x;
				vv.y = v2.y - v.y;
				vv.z = v2.z - v.z;
				setbox(in++, vv, v2, pr[im].cl, ict);
				vv.x = v3.x - v.x;
				vv.y = v3.y - v.y;
				vv.z = v3.z - v.z;
				setbox(in++, vv, v3, pr[im].cl, ict);
				break;
		}
		if (pr[im].cm == 0) {
			ict = in;
			obj[in - 1].cm += 128;
		}
	}
	obj[in].o = 0;
}

void draw(void) {
	int x, y;
	float t1, t2;
	vector r;
	struct color cl;
	for (y = 0; y < YSIZE; y += IDY) {
		for (x = 0; x < XSIZE; x += IDX) {
			t1 = (float) x / XSIZE;
			t2 = (float) y / YSIZE;
			r.x = es.x + ev1.x * t1 + ev2.x*t2;
			r.y = es.y + ev1.y * t1 + ev2.y*t2;
			r.z = es.z + ev1.z * t1 + ev2.z*t2;
			r = normalize(r);
			cl = trace(r);
			dot(x, y, cl);
		}
	}
}

void raymain(void) {
	dataread();
	move();
	draw();
}
