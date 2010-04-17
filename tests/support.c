#include "support.h"

#include "polymath.h"
#include "polymath_internal.h"
#include "stdio.h"
#include "libtest.h"

void polyDump(struct GH_vertex_ll * p)
{
	int c = 0;
	do {
		printf("$%04d$ X=%lf Y=%lf I=%d FLG=%d\n", c++, p->c.x, p->c.y, p->intersect, p->flag);
	} while (p = p->next);
	printf("\n");
}


struct GH_vertex_ll * polyIndex(struct GH_vertex_ll * p, int c)
{
	while (c-- && (p = p->next));
	if (!p)
		LT_THROW("ran off the end of a polygon!");
	return p;
}


int polySize(struct GH_vertex_ll * p)
{
	int size = 1;
	while ((p = p->next))
		size++;
	
	return size;
}

struct GH_vertex_ll * createRectCW(double cx, double cy, double w, double h)
{
	struct GH_vertex_ll * r = NULL, * p;
	r = GH_polyPoint(NULL, cx - w/2, cy - h/2);
	p = GH_polyPoint(r,    cx - w/2, cy + h/2);
	p = GH_polyPoint(p,    cx + w/2, cy + h/2);
	p = GH_polyPoint(p,    cx + w/2, cy - h/2);
	return r;
}

struct GH_vertex_ll * createRect(double cx, double cy, double w, double h)
{
	struct GH_vertex_ll * r = NULL, * p;
	r = GH_polyPoint(NULL, cx - w/2, cy - h/2);
	p = GH_polyPoint(r, cx + w/2, cy - h/2);
	p = GH_polyPoint(p, cx + w/2, cy + h/2);
	p = GH_polyPoint(p, cx - w/2, cy + h/2);
	return r;
}
