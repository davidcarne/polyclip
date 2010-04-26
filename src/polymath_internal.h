/*
 *  Portions Copyright 2006,2009 David Carne and 2007,2008 Spark Fun Electronics
 *
 *
 *  This file is part of gerberDRC.
 *
 *  gerberDRC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  gerberDRC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _POLYMATH_INTERNAL_H_
#define _POLYMATH_INTERNAL_H_

#include <stdbool.h>

struct GH_point {
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

struct GH_vertex_ll {
	struct GH_point	c; //coords
	
	// Next + prev have special meaning when its an intersection queued
	//  next is next in insertion list, prev is the line segment its on
	struct GH_vertex_ll * next, * prev;

	bool	intersect;
	struct GH_vertex_ll * neighbor;

	double	alpha;
	enum flag_type_e flag;
	struct GH_vertex_ll * couple;
	bool	cross_change;
	bool 	done;

};

enum intertype_e {
	INTER_NONE,
	INTER_CROSS,
	INTER_TOUCH,
	INTER_COINCIDE,
};

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

struct GH_vertex_ll * __find_non_intersect(struct GH_vertex_ll * v); 
struct GH_vertex_ll * __find_intersect(struct GH_vertex_ll * v); 
struct GH_vertex_ll * __find_last(struct GH_vertex_ll * v); 
struct GH_vertex_ll * __find_intersect_notdone(struct GH_vertex_ll * v);

struct GH_vertex_ll * alloc_GH_vertex_ll();
struct GH_vertex_ll * GH_createIVertex(struct GH_point * A, struct GH_point * B, double a); 
void GH_linkVerticies(struct GH_vertex_ll * a, struct GH_vertex_ll * b); 
void GH_insertAfter(struct GH_vertex_ll * ip, struct GH_vertex_ll * ti); 
void GH_unlink(struct GH_vertex_ll * v); 
bool GH_polyHasIntersectingNotDone(struct GH_vertex_ll * v); 
void GH_sortedInsert(struct GH_vertex_ll * P1, struct GH_vertex_ll * P2, struct GH_vertex_ll * I); 
double GH_calc_WEC(struct GH_point * A, struct GH_point * B, struct GH_point * C, struct GH_point * D); 
int outcode(struct GH_point * p, double L, double R, double B, double T); 
bool GH_pointCompare(struct GH_point * a, struct GH_point * b); 
enum intertype_e GH_intersect(struct GH_point * P1, struct GH_point * P2, struct GH_point * Q1, struct GH_point * Q2,
	double * alphaP, double * alphaQ); 
	

bool GH_pointInPoly(struct GH_vertex_ll * poly, struct GH_point * point); 
double GH_calcAlpha(struct GH_point * point, struct GH_point * start, struct GH_point * finish); 
int GH_polySize(struct GH_vertex_ll * a); 

bool GH_phase_one(struct GH_vertex_ll * subject, struct GH_vertex_ll * clip); 
void GH_phase_two(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, enum GH_op_t op);
struct GH_vertex_ll * GH_phase_three(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2);
void GHKK_phase_3_prep(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2);
bool GHKK_phase_3_fp(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, enum GH_op_t op, struct GH_vertex_ll ** outpoly);

/*
 * This function does not indicate bits where A or B = C or D, only when there is an intersection point
 * that needs to be created
 */
int GH_lineCoincideBits(struct GH_point * a, struct GH_point * b, struct GH_point * c, struct GH_point * d);

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

