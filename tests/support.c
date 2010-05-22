#include "support.h"

#include "polymath.h"
#include "polymath_internal.h"
#include "stdio.h"
#include "libtest.h"


const char * flg_dec(enum flag_type_e e)
{
	switch(e)
	{
		case FLG_NONE: return "FLG_NONE";
		case FLG_EN: return "FLG_EN";
		case FLG_EX: return "FLG_EX";
		case FLG_EN_EX: return "FLG_EN_EX";
		case FLG_EX_EN: return "FLG_EX_EN";
		default: return "BADFLAG!!!!";
	}
}

const char * td_dec(enum trv_dir e)
{
	switch (e)
	{
		case DIR_FWD:
			return "DIR_FWD";
		case DIR_REV:
			return "DIR_REV";
		case DIR_ACFWD:
			return "DIR_ACFWD";
		case DIR_ACREV:
			return "DIR_ACREV";
		default: return "BADDIR!!!!";
	}
}
void polyDump(struct PC_vertex_ll * p)
{
	int c = 0;
	do {
		printf("$%04d$ X=%lf Y=%lf I=%d FLG=%s\n", c++, p->c.x, p->c.y, p->intersect, flg_dec(p->flag));
	} while (p = p->next);
	printf("\n");
}


struct PC_vertex_ll * polyIndex(struct PC_vertex_ll * p, int c)
{
	while (c-- && (p = p->next));
	if (!p)
		LT_THROW("ran off the end of a polygon!");
	return p;
}


int polySize(struct PC_vertex_ll * p)
{
	int size = 1;
	while ((p = p->next))
		size++;
	
	return size;
}

struct PC_vertex_ll * createRectCW(double cx, double cy, double w, double h)
{
	struct PC_vertex_ll * r = NULL, * p;
	r = PC_polyPoint(NULL, cx - w/2, cy - h/2);
	p = PC_polyPoint(r,    cx - w/2, cy + h/2);
	p = PC_polyPoint(p,    cx + w/2, cy + h/2);
	p = PC_polyPoint(p,    cx + w/2, cy - h/2);
	return r;
}

struct PC_vertex_ll * createRect(double cx, double cy, double w, double h)
{
	struct PC_vertex_ll * r = NULL, * p;
	r = PC_polyPoint(NULL, cx - w/2, cy - h/2);
	p = PC_polyPoint(r, cx + w/2, cy - h/2);
	p = PC_polyPoint(p, cx + w/2, cy + h/2);
	p = PC_polyPoint(p, cx - w/2, cy + h/2);
	return r;
}
