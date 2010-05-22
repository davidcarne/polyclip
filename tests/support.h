#ifndef _SUPPORT_H_
#define _SUPPORT_H_

#include "polymath.h"
#include "polymath_internal.h"
#define COMPARE_EPSILON 0.000001
#define _FC(a,b) (fabs((a) - (b)) < COMPARE_EPSILON)
#define VERTEX_COMPARE_TO(a, xx, yy) (_FC((a)->c.x, (xx)) && _FC((a)->c.y, (yy)))
#define _I polyIndex


void polyDump(struct PC_vertex_ll * p);
struct PC_vertex_ll * polyIndex(struct PC_vertex_ll * p, int c);
int polySize(struct PC_vertex_ll * p);
struct PC_vertex_ll * createRectCW(double cx, double cy, double w, double h);
struct PC_vertex_ll * createRect(double cx, double cy, double w, double h);
const char * td_dec(enum trv_dir e);
const char * flag_dec(enum trv_dir e);

#endif

