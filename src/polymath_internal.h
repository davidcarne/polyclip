/*
 *  Copyright 2009, 2010 David Carne. Released under the revised BSD license
 *
 */

#ifndef _POLYMATH_INTERNAL_H_
#define _POLYMATH_INTERNAL_H_


/*
 * You should not use this file directly from your own code. It exists purely to allow 
 * the polymath internal test code to check subfunctions of the clipper.
 * 
 * Anything in here should be considered unstable, and likely broken.
 */
#include <stdbool.h>

struct PC_point {
	double x, y;
};


enum flag_type_e {
		FLG_NONE,
		FLG_EN,
		FLG_EX,
		FLG_EN_EX,
		FLG_EX_EN,
		};

enum edge_status_t {
	edge_in,
	edge_on,
	edge_out
};

struct PC_vertex_ll {
	struct PC_point	c; //coords
	
	// Next + prev have special meaning when its an intersection queued
	//  next is next in insertion list, prev is the line segment its on
	struct PC_vertex_ll * next, * prev;

	bool	intersect;
	struct PC_vertex_ll * neighbor;

	double	alpha;
	enum flag_type_e flag;
	struct PC_vertex_ll * couple;
	bool	cross_change;
	bool 	done;

};

enum intertype_e {
	INTER_NONE,
	INTER_CROSS,
	INTER_TOUCH,
	INTER_COINCIDE,
};

enum trv_dir {
	DIR_FWD,
	DIR_REV,
	DIR_ACFWD,
	DIR_ACREV
};

bool trvIsForward(enum trv_dir a);
bool trvIsAcross(enum trv_dir a);
enum trv_dir trvReverse(enum trv_dir a);
enum trv_dir trvAcross(enum trv_dir a);


bool PC_intersection_same_way(struct PC_vertex_ll * a_p, struct PC_vertex_ll * a_n, 
                              struct PC_vertex_ll * b_p, struct PC_vertex_ll * b_n,
                              struct PC_vertex_ll * i);

// iterates over all pairs of nonintersection in a polygon
#define FOR_VERTEX_NI_PAIR(start, va, vb) \
	va = start; \
	vb = __find_non_intersect(start->next); \
	do { \
		if (vb == NULL) \
			vb = start;


#define END_FOR_VERTEX_NI_PAIR(start, va, vb) \
	va = vb; \
	vb = __find_non_intersect(va->next); \
	} while (va != start)

// iterates over all intersections with the element before and after
#define FOR_VERTEX_I_CENTRI(start, vp, vi, vn) \
	vi = __find_intersect(start); \
	do { \
		vn = vi->next; \
		if (!vn) \
			vn = start;\
		vp = vi->prev; \
		assert((vp) || (vi == start)); \
		if (vi == start) \
			vp = __find_last(start);


#define END_FOR_VERTEX_I_CENTRI(start, vp, vi, vn) \
	vi = __find_intersect(vi->next); \
	} while (vi != NULL)

// Loop over all pairs in a polygon
#define FOR_VERTEX_PAIR(start, va, vb) \
	va = start; \
	vb = start->next; \
	do { \
		if (vb == NULL) \
			vb = start;

#define END_FOR_VERTEX_PAIR(start, va, vb) \
	va = vb; \
	vb = va->next; \
	} while (va != start)

// Loop over all intersection verticies in a polygon
#define FOR_VERTEX_I(start, va) \
	va = __find_intersect(start); \
	while (va != NULL) {


#define END_FOR_VERTEX_I(start, va) \
	va = __find_intersect(va->next); \
	}

// Loop over all verticies in a polygon
#define FOR_VERTEX(start, va) \
	va = start; \
	while (va != NULL) {


#define END_FOR_VERTEX(start, va) \
	va = va->next; \
	}

#define VERTEX_POINT(v) (&((v)->c))


struct PC_vertex_ll * alloc_PC_vertex_ll();
struct PC_vertex_ll * PC_createIVertex(struct PC_point * A, struct PC_point * B, double a); 
void PC_linkVerticies(struct PC_vertex_ll * a, struct PC_vertex_ll * b); 
void PC_insertAfter(struct PC_vertex_ll * ip, struct PC_vertex_ll * ti); 
void PC_unlink(struct PC_vertex_ll * v); 
bool PC_polyHasIntersectingNotDone(struct PC_vertex_ll * v); 
void PC_sortedInsert(struct PC_vertex_ll * P1, struct PC_vertex_ll * P2, struct PC_vertex_ll * I); 
double PC_calc_WEC(struct PC_point * A, struct PC_point * B, struct PC_point * C, struct PC_point * D); 
int outcode(struct PC_point * p, double L, double R, double B, double T); 
bool PC_pointCompare(struct PC_point * a, struct PC_point * b); 
enum intertype_e PC_intersect(struct PC_point * P1, struct PC_point * P2, struct PC_point * Q1, struct PC_point * Q2,
	double * alphaP, double * alphaQ); 
	

bool PC_pointInPoly(struct PC_vertex_ll * poly, struct PC_point * point); 
double PC_calcAlpha(struct PC_point * point, struct PC_point * start, struct PC_point * finish); 
int PC_polySize(struct PC_vertex_ll * a); 

bool PC_phase_one(struct PC_vertex_ll * subject, struct PC_vertex_ll * clip); 
void PC_phase_two(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2, enum PC_op_t op);
struct PC_vertex_ll * PC_phase_three(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2);
void GHKK_phase_3_prep(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2);
bool GHKK_phase_3_fp(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2, enum PC_op_t op, struct PC_vertex_ll ** outpoly);

/*
 * This function does not indicate bits where A or B = C or D, only when there is an intersection point
 * that needs to be created
 */
int PC_lineCoincideBits(struct PC_point * a, struct PC_point * b, struct PC_point * c, struct PC_point * d);

#define OC_L (1 << 3)
#define OC_R (1 << 2)
#define OC_B (1 << 1)
#define OC_T (1 << 0)

#define A_ONCD (1<<3)
#define B_ONCD (1<<2)
#define C_ONAB (1<<1)
#define D_ONAB (1<<0)

#define A_IS_C (1<<7)
#define A_IS_D (1<<6)
#define B_IS_C (1<<5)
#define B_IS_D (1<<4)


#endif

