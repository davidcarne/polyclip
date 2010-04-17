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


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef INT_ASSERT
#include <assert.h>
#else
#include "../tests/libtest.h"
#define assert LT_REQUIRE
#endif

#include "polymath.h"
#include "polymath_internal.h"

#define COMPARE_EPSILON 0.0000001
#define _FC(a,b) (fabs((a) - (b)) < COMPARE_EPSILON)
#define POINT_COMPARE(a, b) (_FC((a)->x, (b)->x) && _FC((a)->y, (b)->y))
#define RECT_BETWEEN(a, c,d)	((a->x > c->x && a->x < d->x || a->x > d->x && a->x < c->x) || \
(a->y > c->y && a->y < d->y || a->y > d->x && a->y < c->y))


struct GH_vertex_ll * __find_non_intersect(struct GH_vertex_ll * v)
{
	while (v && v->intersect)
		v = v->next;
	return v;
}
struct GH_vertex_ll * __find_intersect(struct GH_vertex_ll * vo)
{
	struct GH_vertex_ll * v = vo;
	while (v && !v->intersect)
	{
		v = v->next;
		
		if (v == vo)
			return NULL;
	}
	return v;
}
struct GH_vertex_ll * __find_intersect_nb_notnone(struct GH_vertex_ll * v)
{
	while (v && !(v->intersect && v->neighbor->flag != FLG_NONE))
		v = v->next;
	return v;
}

struct GH_vertex_ll * __find_intersect_notdone(struct GH_vertex_ll * v)
{
	while (v && (!v->intersect || v->done))
		v = v->next;
	return v;
}

struct GH_vertex_ll * __find_last(struct GH_vertex_ll * vo)
{
	struct GH_vertex_ll * v = vo;
	while (v && v->next)
	{
		v = v->next;
		
		if (v == vo)
			return NULL;
	}
	return v;
}

struct GH_vertex_ll * __find_meets_p3_criteria(struct GH_vertex_ll * vo)
{
	struct GH_vertex_ll * v = vo;
	while (v)
	{
		if (v->flag != FLG_NONE)
		{
			if (v->couple)
			{
				// If we're the second of a couple, and we're an entrance
				if (v->flag == FLG_EN && v->couple == v->prev)
					return v;
				if (v->flag == FLG_EX && v->couple == v->next)
					return v;
				
			} else {
				return v;
			}
		}
		v = v->next;
		
		if (v == vo)
			return NULL;
	}
	return v;
}

struct GH_vertex_ll * alloc_vertex()
{
	struct GH_vertex_ll * v = (struct GH_vertex_ll *)malloc(sizeof(struct GH_vertex_ll));
	
	if (!v) return NULL;
	
	v->c.x = v->c.y = INFINITY;
	v->next = v->prev = v->nextPoly = v->neighbor = NULL;
	v->intersect = false;
	v->flag = FLG_NONE;
	v->cross_change = false;
	v->couple = NULL;
	v->alpha = 0;
	v->done = false;
	return v;
}

struct GH_vertex_ll * GH_createIVertex(struct GH_point * A, struct GH_point * B, float a)
{
	assert(A != NULL);
	assert(B != NULL);
	assert(a > 0 && a < 1);

	double x = A->x * (1 - a) + B->x * a;
	double y = A->y * (1 - a) + B->y * a;
	struct GH_vertex_ll * v = alloc_vertex();
	v->alpha = a;
	v->intersect = true;
	v->c.x = x;
	v->c.y = y;
	return v;
}

void GH_linkVerticies(struct GH_vertex_ll * a, struct GH_vertex_ll * b)
{
	assert(a != NULL);
	assert(b != NULL);

	a->neighbor = b;
	b->neighbor = a;
}

// insertion point and where to insert
void GH_insertAfter(struct GH_vertex_ll * ip, struct GH_vertex_ll * ti)
{
	assert(ip != NULL);
	assert(ti != NULL);
	assert(ti->next == NULL);
	assert(ti->prev == NULL);


	ti->prev = ip;
	ti->next = ip->next;
	if (ip->next)
		ip->next->prev = ti;

	ip->next = ti;
}
void GH_unlink(struct GH_vertex_ll * v)
{
	assert(v);
	
	if (v->prev)
		v->prev->next = v->next;
	if (v->next)
		v->next->prev = v->prev;
}

bool GH_polyHasIntersectingNotDone(struct GH_vertex_ll * v)
{
	assert(v);
	
	struct GH_vertex_ll * a;
	
	FOR_VERTEX_I(v, a)
		if (!a->done)
			return true;
	END_FOR_VERTEX_I(v, a);

	return false;
}

void GH_sortedInsert(struct GH_vertex_ll * P1, struct GH_vertex_ll * P2, struct GH_vertex_ll * I)
{
	assert(P1 != NULL);	assert(P2 != NULL);	assert(I != NULL);
	assert(!P1->intersect);	assert(!P2->intersect);	assert(I->intersect);
	assert(P1->alpha == 0);	assert(P2->alpha == 0);	assert(I->alpha > 0 && I->alpha < 1);

	struct GH_vertex_ll * J = P1;
	// Need to find vertex J
	// 	such that !J->intersect || J->alpha <= I->alpha
	// 	and	!J->next || !J->next->intersect || J->alpha >= I->alpha
	
	while (	J && !((!J->intersect || J->alpha <= I->alpha) && 
		(!J->next || !J->next->intersect || J->next->alpha >= I->alpha)))
	{
		J = J->next;
		assert(J != P2);
	}
	assert(J);
	GH_insertAfter(J,I);


}

float GH_calc_WEC(struct GH_point * A, struct GH_point * B, struct GH_point * C, struct GH_point * D)
{
	assert(A != NULL); assert(B != NULL); assert(C != NULL); assert(D != NULL);
	// (A - B) dot perpendicular(C - D)
	return (float)(A->y - B->y) * (C->x - D->x) - (A->x - B->x) * (C->y - D->y);
}

int outcode(struct GH_point * p, double L, double R, double B, double T)
{
	assert(p != NULL);
	assert(L <= R); assert(B <= T);

	int outcode = 0;
	if (p->x < L)
		outcode |= OC_L;
	if (p->x > R)
		outcode |= OC_R;
	if (p->y < B)
		outcode |= OC_B;
	if (p->y > T)
		outcode |= OC_T;

	return outcode;
}

bool GH_pointCompare(struct GH_point * a, struct GH_point * b)
{
	assert(a); assert(b);
	
	return _FC(a->x, b->x) && _FC(a->y, b->y);
}

// Intersects P1->P2 with Q1->Q2
// 	P1 Open, P2 Closed [aka, P1 on Q1->Q2 = NO INTERSECT]
enum intertype_e GH_intersect(struct GH_point * P1, struct GH_point * P2, struct GH_point * Q1, struct GH_point * Q2,
	float * alphaP, float * alphaQ)
{
	assert(P1 != NULL); assert(P2 != NULL); assert(Q1 != NULL); assert(Q2 != NULL);
	assert(alphaP != NULL); assert(alphaQ != NULL);

	
	*alphaQ = -INFINITY;
	*alphaP = -INFINITY;
	
	// First do outcodes
	// LRBT
	int outcode_P1 = 0;
	int outcode_P2 = 0;
	double winL = Q1->x > Q2->x ? Q2->x : Q1->x;
	double winR = Q1->x > Q2->x ? Q1->x : Q2->x;
	double winB = Q1->y > Q2->y ? Q2->y : Q1->y;
	double winT = Q1->y > Q2->y ? Q1->y : Q2->y;

	outcode_P1 = outcode(P1, winL, winR, winB, winT);
	outcode_P2 = outcode(P2, winL, winR, winB, winT);
	
	/* If any bits set in the outcode, no intersection possible, so abort */
	if (outcode_P1 & outcode_P2)
		return INTER_NONE;

	/* Now do the window-edge-coordinate method of finding intersections */
	float WEC_P1 = GH_calc_WEC(P1, Q1, Q2, Q1);
	float WEC_P2 = GH_calc_WEC(P2, Q1, Q2, Q1);

	if (WEC_P1 * WEC_P2 <= 0)
	{
		float WEC_Q1 = GH_calc_WEC(Q1, P1, P2, P1);
		float WEC_Q2 = GH_calc_WEC(Q2, P1, P2, P1);
		if (WEC_Q1 * WEC_Q2 <= 0)
		{
			*alphaP = WEC_P1 / (WEC_P1 - WEC_P2);
			*alphaQ = WEC_Q1 / (WEC_Q1 - WEC_Q2);

			assert(isnan(*alphaP) && isnan(*alphaQ) || !isnan(*alphaP) && !isnan(*alphaQ));

			if (isnan(*alphaP) && isnan(*alphaQ))
			{
				return INTER_COINCIDE;
			}

			if (_FC(*alphaP, 0.0))
				return INTER_NONE;

			if (_FC(*alphaQ, 0.0))
				return INTER_NONE;

			if (_FC(*alphaP, 1.0))
				return INTER_TOUCH;
			
			if (_FC(*alphaQ, 1.0))
				return INTER_TOUCH;


			assert(*alphaP > 0.0 && *alphaP <= 1.0 && *alphaQ > 0.0 && *alphaP <= 1.0);

			return INTER_CROSS;
		}
	}
	return INTER_NONE;
}


bool GH_pointInPoly(struct GH_vertex_ll * poly, struct GH_point * point)
{
	assert(poly); assert(point);
	
	// TODO: optimize + fix
	struct GH_point end;
	float aa, ab;

	// TODO: end.x should be > max_x
	end.x = point->x+1000000;
	end.y = point->y;

	int c = 0;
	struct GH_vertex_ll * a, *b;
	
	// initialize to status of last line
	bool coincide_disable = GH_intersect(VERTEX_POINT(__find_last(poly)), VERTEX_POINT(poly), 
										 point, &end, &aa, &ab) == INTER_COINCIDE;
	
	FOR_VERTEX_PAIR(poly, a, b)
		enum intertype_e i = GH_intersect(VERTEX_POINT(a), VERTEX_POINT(b), point, &end, &aa, &ab);
		
		// coincide disable prevents multiple consecutive lines that are coincident with
		// the test line from counting as additional windings
		// We need to count the first coincident line, since we counted the line
		// that touched the test line to bring it into co-incidence
		if (i != INTER_NONE && ( i != INTER_COINCIDE || i == INTER_COINCIDE && !coincide_disable))
			c++;
		coincide_disable = i == INTER_COINCIDE;
	
	END_FOR_VERTEX_PAIR(poly, a, b);
	return c & 0x1; // Odd winding # means inside poly, even outside.
}

float GH_calcAlpha(struct GH_point * point, struct GH_point * start, struct GH_point * finish)
{
	assert(point);
	assert(start);
	assert(finish);

	double dx = finish->x - start->x;
	double dy = finish->y - start->y;
	assert(!(_FC(dx, 0) && _FC(dy, 0)));

	float a = 0;
	if (!_FC(dx, 0))
	{
		a = (point->x - start->x) / dx;
	} else {
		a = (point->y - start->y) / dy;
	}
	assert(a > 0);
	assert(a < 1);
	assert(!_FC(a, 0));
	assert(!_FC(a, 1));

	return a;
}
void GH_insertLinkClonedVertex(struct GH_vertex_ll * toclone, struct GH_vertex_ll * before, struct GH_vertex_ll * after)
{
	assert(toclone);
	assert(before);
	assert(after);
	assert(!toclone->neighbor);
	assert(before != after);
	assert(toclone != before);
	assert(toclone != after);

	struct GH_vertex_ll * i1 = alloc_vertex();

	i1->alpha = GH_calcAlpha(VERTEX_POINT(toclone), VERTEX_POINT(before), VERTEX_POINT(after));


	i1->intersect = true;
	i1->c.x = toclone->c.x;
	i1->c.y = toclone->c.y;

	// NOTE: we do NOT mark the point is an intersect yet, as that would munge up
	// processing [as its a corner, and therefore must be considered!]
	// toclone->intersect = true;
	GH_linkVerticies(i1, toclone);
	GH_sortedInsert(before, after, i1);
}

/*
 * Quoted from Greiner-Hormann '98
 *
 * Search for all intersection points by testing whether each edge of
 * the subject polygon and each of the clip polygon intersect or not.
 *
 * If they do, the intersection routine will deliver two numbers between
 * 0 and 1, the alpha values, which indicate where the intersection
 * point lies relatively to the start and end points of both edges.
 *
 * With respect to these alpha values, we create new verticies and insert
 * them into the data structures of subject and clip polygons between the 
 * start and end points of the edges they intersect. If no intersection
 * points are detected we know that either the subject polygon lies entirely
 * inside the clip polygon or vice versa, or both polygons are disjoint.
 *
 * By performing even-odd rule we can easily decide which case we have
 * and simply return either the inner polygon as the clipped polygon, 
 * or nothing at all [moved outside of phase 1]
 *
 */
bool GH_phase_one(struct GH_vertex_ll * subject, struct GH_vertex_ll * clip)
{
	assert(subject != NULL);
	assert(clip != NULL);

	/*
	 * Pseudo code for phase one
	 * for each vertex Si of subject do
	 * 	for each vertex Cj of clip do
	 * 		if intersect Si, Si+1, Cj, Cj + 1, a, b
	 * 			I1 = CreateVertex(Si, Si+1, a)
	 * 			I2 = CreateVertex(Cj, Cj+1, b)
	 * 			link I1 + I2
	 * 			add I1 to subject poly
	 * 			add I2 to clip poly
	 * 		end if
	 * 	end for
	 * end for
	 */
	struct GH_vertex_ll * s0, * s1;
	struct GH_vertex_ll * c0, * c1;

	bool intersect_found = false;

	/* poly insertion list pointers */
	struct GH_vertex_ll * sI = NULL, * cI = NULL;
	
	// Iterate across both polygons
	FOR_VERTEX_NI_PAIR(subject, s0, s1)
		FOR_VERTEX_NI_PAIR(clip, c0, c1)
			float a, b;
			enum intertype_e inter_type = GH_intersect(
					VERTEX_POINT(s0), VERTEX_POINT(s1), 
					VERTEX_POINT(c0), VERTEX_POINT(c1),
					&a, &b);

			if (inter_type != INTER_NONE)
			{

				// If the lines are simply crossing
				if (inter_type == INTER_CROSS)
				{
					struct GH_vertex_ll * i1 = GH_createIVertex(
						VERTEX_POINT(s0), VERTEX_POINT(s1),
						a);
					struct GH_vertex_ll * i2 = GH_createIVertex(
						VERTEX_POINT(c0), VERTEX_POINT(c1),
						b);
				
					GH_linkVerticies(i1, i2);

					GH_sortedInsert(s0, s1, i1);
					GH_sortedInsert(c0, c1, i2);
				} else if (inter_type == INTER_TOUCH) {
					assert(!_FC(a, 0.0));
					assert(!_FC(b, 0.0));
#ifdef PHASE1_VERBOSE
					printf("Touch %4.2f %4.2f -> %4.2f %4.2f with %4.2f %4.2f -> %4.2f %4.2f: %4.2f %4.2f\n",
						   VERTEX_POINT(s0)->x, VERTEX_POINT(s0)->y,
						   VERTEX_POINT(s1)->x, VERTEX_POINT(s1)->y,
						   VERTEX_POINT(c0)->x, VERTEX_POINT(c0)->y,
						   VERTEX_POINT(c1)->x, VERTEX_POINT(c1)->y,
						   a,
						   b);
#endif					
					// If we're touching in the middle, then we need to add a vertex
					if ((a < 1.0) && !(b < 1.0) || (b<1.0) && !(a < 1.0)) {
						// The edge that is being touched gets an intersect vertex
						if (a < 1.0)
							GH_insertLinkClonedVertex(c1, s0, s1);
						else
							GH_insertLinkClonedVertex(s1, c0, c1);
					} else {
#ifdef PHASE1_VERBOSE
						printf("\tLinked two verticies [%f %f]\n",a,b);
#endif
						// We're touching at the ends [both a == 1 && b == 1]
						GH_linkVerticies(s1, c1);
					}

				} else if (inter_type == INTER_COINCIDE) {
					int bits = GH_lineCoincideBits(
						VERTEX_POINT(s0), VERTEX_POINT(s1),
						VERTEX_POINT(c0), VERTEX_POINT(c1));
					
					/* A on CD is already generated by touch comparison
					 *if (bits & A_ONCD)
					 *	GH_insertLinkClonedVertex(s0, c0, c1);*/
					if (bits & B_ONCD)
						GH_insertLinkClonedVertex(s1, c0, c1);
					/* C on AB is already generated by touch comparison
					 * if (bits & C_ONAB)
						GH_insertLinkClonedVertex(c0, s0, s1);*/
					if (bits & D_ONAB)
						GH_insertLinkClonedVertex(c1, s0, s1);
					
					if (bits & A_IS_C)
						GH_linkVerticies(s0, c0);
					if (bits & A_IS_D)
						GH_linkVerticies(s0, c1);
					if (bits & B_IS_C)
						GH_linkVerticies(s1, c0);
					if (bits & B_IS_D)
						GH_linkVerticies(s1, c1);

				}
				intersect_found = true;
			}
		END_FOR_VERTEX_NI_PAIR(clip, c0, c1);
	END_FOR_VERTEX_NI_PAIR(subject, s0, s1);

	struct GH_vertex_ll * i;
	FOR_VERTEX(clip, i)
		if (i->neighbor)
			i->intersect = true;
	END_FOR_VERTEX(clip, i);
	FOR_VERTEX(subject, i)
		if (i->neighbor)
			i->intersect = true;
	END_FOR_VERTEX(subject, i);

	return intersect_found;
}

// Check if line is on the other polygon
bool GH_is_on(struct GH_vertex_ll * a, struct GH_vertex_ll * b)
{
	if (!a->intersect || !b->intersect)
		return false;

	assert(a->neighbor);
	assert(b->neighbor);

	struct GH_vertex_ll * n_a = a->neighbor;
	struct GH_vertex_ll * n_b = b->neighbor;

	// wraparound case
	if (n_a->next == NULL && n_b->prev == NULL ||
		n_a->prev == NULL && n_b->next == NULL)
		return true;
	
	// standard case
	if (n_a->next == n_b || n_b->next == n_a)
		return true;

	return false;
}


struct GH_point GH_midPoint(struct GH_point * a, struct GH_point * b)
{
	assert(a);
	assert(b);
	struct GH_point mid;
	mid.x = (a->x + b->x) / 2.0;
	mid.y = (a->y + b->y) / 2.0;
	return mid;
}
/*
 * Calculates edge status according to Kim-Kim 2006 4.1
 */
enum edge_status_t GHKK_edgeStatus(struct GH_vertex_ll * a, struct GH_vertex_ll * b, struct GH_vertex_ll * other)
{
	assert(a);
	assert(b);
	assert(other);

	if (GH_is_on(a, b))
		return edge_on;

	struct GH_point mid = GH_midPoint(VERTEX_POINT(a),VERTEX_POINT(b));
	return GH_pointInPoly(other, &mid) ? edge_in : edge_out;
	
}

enum flag_type_e GHKK_calcVertexFlag(enum edge_status_t gamma_p, enum edge_status_t gamma_n)
{
	switch (gamma_p)
	{
		case edge_in:
			switch (gamma_n)
			{
				case edge_in:
					return FLG_EX_EN;
				case edge_on:
				case edge_out:
					return FLG_EX;
			}
			assert(false);

		case edge_on:
			switch (gamma_n)
			{
				case edge_in:
					return FLG_EN;
				case edge_on:
					return FLG_NONE;
				case edge_out:
					return FLG_EX;
			}
			assert(false);

		case edge_out:
			switch (gamma_n)
			{
				case edge_in:
				case edge_on:
					return FLG_EN;
				case edge_out:
					return FLG_EN_EX;
			}
			assert(false);
	}
	assert(false);
}

void GHKK_phase_two_firstpoly(struct GH_vertex_ll * p, struct GH_vertex_ll * other, enum GH_op_t op)
{
	assert(p); assert(other);
	
	enum edge_status_t gamma_p, gamma_n;

	struct GH_vertex_ll * Cip, * Ci, * Cin;
	FOR_VERTEX_I_CENTRI(p, Cip, Ci, Cin)
		gamma_p = GHKK_edgeStatus(Cip, Ci, other);
		gamma_n = GHKK_edgeStatus(Ci, Cin, other);
		Ci->flag = GHKK_calcVertexFlag(gamma_p, gamma_n);
#ifdef PHASE2_VERBOSE
		printf("(%4.2f %4.2f)->(%4.2f %4.2f)->(%4.2f %4.2f): %d %d %d\n", 
			   Cip->c.x, Cip->c.y,
			   Ci->c.x, Ci->c.y, 
			   Cin->c.x, Cin->c.y,
			   gamma_p, gamma_n, Ci->flag);
#endif
	END_FOR_VERTEX_I_CENTRI(p, Cip, Ci, Cin);
}

enum flag_type_e GHKK_invertFlag(enum flag_type_e e)
{
	switch (e)
	{	
		case FLG_NONE:
			return FLG_NONE;
		case FLG_EN:
			return FLG_EX;
		case FLG_EX:
			return FLG_EN;
		case FLG_EN_EX:
			return FLG_EX_EN;
		case FLG_EX_EN:
			return FLG_EN_EX;
		default:
			assert(false);
	}
}
void GHKK_phase_two_secondpoly(struct GH_vertex_ll * p, struct GH_vertex_ll * other, enum GH_op_t op)
{
	assert(p); assert(other);
	
	struct GH_vertex_ll * Ci = __find_intersect_nb_notnone(p);
	assert(Ci);
	
	struct GH_vertex_ll * Cip = Ci->prev;
	if (!Cip)
		Cip = __find_last(Ci);
	struct GH_vertex_ll * Cin = Ci->next;
	enum edge_status_t gamma_p, gamma_n;

	assert(Ci); assert(Cip); assert(Cin); assert(Ci->neighbor); assert(Ci->intersect);

	gamma_p = GHKK_edgeStatus(Cip, Ci, other);
	gamma_n = GHKK_edgeStatus(Ci, Cin, other);
	Ci->flag = GHKK_calcVertexFlag(gamma_p, gamma_n);

	enum flag_type_e Ciflag_inv = GHKK_invertFlag(Ci->flag);
#ifdef PHASE2_VERBOSE
	printf("gamma_p = %x, gamma_n = %x, Ci->flag = %x, CiInv = %x, Ci->neighbor->flag = %x\n", gamma_p, gamma_n, Ci->flag, Ciflag_inv, Ci->neighbor->flag);
#endif
	assert(Ci->flag == Ci->neighbor->flag || Ciflag_inv == Ci->neighbor->flag);

	bool flipflags = Ciflag_inv == Ci->neighbor->flag;

	struct GH_vertex_ll * i;
	FOR_VERTEX_I(p, i)
		if (flipflags)
			i->flag = GHKK_invertFlag(i->neighbor->flag);
		else
			i->flag = i->neighbor->flag;
	END_FOR_VERTEX_I(p, i);

}

// We don't consider FLAG_NONE points intersections after this point
void GHKK_phase_two_clearNoFlags(struct GH_vertex_ll * p1)
{
	assert(p1);
	
	struct GH_vertex_ll * Ci;
	
	FOR_VERTEX(p1, Ci); 
	{
		if (Ci->flag == FLG_NONE && Ci->intersect)
		{
			// sanity checks
			assert(Ci->neighbor);
			assert(Ci->neighbor->flag == FLG_NONE);
			
			Ci->intersect = false;
			Ci->neighbor->intersect = false;
			Ci->neighbor->neighbor = NULL;
			Ci->neighbor = NULL;
		}
	}
	END_FOR_VERTEX(p1, Ci);
}

void GHKK_phase_two_buildcouples(struct GH_vertex_ll * p1)
{
	assert(p1);
	
	struct GH_vertex_ll * a, * b;
	
	FOR_VERTEX_PAIR(p1, a, b);
	if (a->flag == b->flag && 
		(a->flag == FLG_EN || a->flag == FLG_EX))
	{
		assert(a->neighbor->flag == b->neighbor->flag && (a->next == b || b->next == a));
		
		a->couple = b;
		b->couple = a;
		a->neighbor->couple = b->neighbor;
		b->neighbor->couple = a->neighbor;
	}
	
	END_FOR_VERTEX_PAIR(p1, a, b);
}

void GH_phase_two(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, enum GH_op_t op)
{
	assert(p1); assert(p2);
	
	GHKK_phase_two_firstpoly(p2, p1, op);
	GHKK_phase_two_secondpoly(p1, p2, op);
	GHKK_phase_two_clearNoFlags(p2);
	GHKK_phase_two_buildcouples(p2);
}

void GH_state_transition_union(bool direction, struct GH_vertex_ll * v, struct GH_vertex_ll ** outvert, bool * outdir)
{
	assert(v);
	assert(outvert);
	
	// If this is not an intersection vertex, simply emit it.
	if (!v->intersect)
	{
		*outvert = direction ? v->next : v->prev;
		*outdir = direction;
		return;
	}
	
	// Otherwise, we need to transition through the intersection
	switch (v->flag)
	{
		case FLG_NONE:
			*outvert = direction ? v->next : v->prev;
			*outdir = direction;
			return;
		case FLG_EN:			
			if (v->neighbor->flag == FLG_EX)
			{
				v->flag = FLG_NONE;
				v->neighbor->flag = FLG_NONE;
				v->intersect = false;
				v->neighbor->intersect = false;
				*outvert = direction ? v->neighbor->next : v->neighbor->prev;
				*outdir = direction;
			} else if (v->neighbor->flag == FLG_EN)
			{
				v->flag = FLG_NONE;
				v->neighbor->flag = FLG_NONE;
				v->intersect = false;
				v->neighbor->intersect = false;
				*outvert = !direction ? v->neighbor->next : v->neighbor->prev;
				*outdir = !direction;
			} else {
				assert(false);
			}
			return;
			
		case FLG_EX:
			
			if (v->neighbor->flag == FLG_EX)
			{
				v->flag = FLG_NONE;
				v->neighbor->flag = FLG_NONE;
				v->intersect = false;
				v->neighbor->intersect = false;
				*outvert = !direction ? v->neighbor->next : v->neighbor->prev;
				*outdir = !direction;
			} else if (v->neighbor->flag == FLG_EN)
			{
				v->flag = FLG_NONE;
				v->neighbor->flag = FLG_NONE;
				*outvert = direction ? v->neighbor->next : v->neighbor->prev;
				*outdir = direction;
				v->intersect = false;
				v->neighbor->intersect = false;
			} else {
				assert(false);
			}
			return;
			
		default:
			assert(false); // Flag we can't handle yet encountered
	}
}

// Prepare for phase 3 by closing the ring on the polygon
void GHKK_phase_3_prep(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2)
{
	assert(p1); assert(p2);
	assert(!p1->prev); assert(!p2->prev);
	
	// close the loop
	struct GH_vertex_ll * l = __find_last(p1);
	p1->prev = l;
	l->next = p1;
	
	l = __find_last(p2);
	l->next = p2;
	p2->prev = l;
}

bool GHKK_phase_3_fp(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, struct GH_vertex_ll ** outpoly)
{
	struct GH_vertex_ll * l;
	assert(p1); assert(p2); assert(outpoly);
	
	bool direction = 1;
	l = __find_meets_p3_criteria(p1);
	
	if (!l)
		return false;
	
	struct GH_vertex_ll * v = l;
	
	struct GH_vertex_ll * newpoly = NULL;
	struct GH_vertex_ll * nextout = NULL;
	do {
		//printf("%4.2f %4.2f\n", v->c.x, v->c.y);
		nextout = GH_polyPoint(nextout, v->c.x, v->c.y);
		if (!newpoly)
			newpoly = nextout;

		bool newdir;
		struct GH_vertex_ll * newvert;
		GH_state_transition_union(direction,v, &newvert, &newdir);
		v = newvert;
		direction = newdir;
		
	} while (v != l);
	
	*outpoly = newpoly;
	return true;
}

/* Writes a new polygon at the current poly ptr + moves the pointer to the new location */

#define GH_NEWPOLYGON() { current_poly_start = output_polygon_pos; \
	if (*output_polygon_pos) \
	output_vertex_pos = output_polygon_pos = &((*output_polygon_pos)->nextPoly);}

/* allocate a new vertex, store its location where the vertex_output_ptr points to + advance the
 * vertex output ptr to the ->next field */ 
void GH_newVertex(struct GH_vertex_ll * current_vertex, struct GH_vertex_ll *** vertex_output_ptr)
{
	**vertex_output_ptr = alloc_vertex();
	(**vertex_output_ptr)->c = current_vertex->c;
	*vertex_output_ptr = &((**vertex_output_ptr)->next);
	//printf("Create point %f %f [%p] %d\n", current_vertex->c.x, current_vertex->c.y, current_vertex, current_vertex->done);
}

int GH_polySize(struct GH_vertex_ll * a)
{
	assert(a);
	int c = 1;
	while (a = a->next)
	{
		c++;
	}
	return c;
}

int GH_lineCoincideBits(struct GH_point * a, struct GH_point * b, struct GH_point * c, struct GH_point * d)
{
	int bits = 0;
	assert(a); assert(b); assert(c); assert(d);
	assert(!POINT_COMPARE(a,b));
	assert(!POINT_COMPARE(c,d));

	if (RECT_BETWEEN(a,c,d))
		bits |= A_ONCD;
	if (RECT_BETWEEN(b,c,d))
		bits |= B_ONCD;
	if (RECT_BETWEEN(c,a,b))
		bits |= C_ONAB;
	if (RECT_BETWEEN(d,a,b))
		bits |= D_ONAB;


	if (POINT_COMPARE(a,c))
		bits |= A_IS_C;
	if (POINT_COMPARE(b,c))
		bits |= B_IS_C;
	if (POINT_COMPARE(a,d))
		bits |= A_IS_D;
	if (POINT_COMPARE(b,d))
		bits |= B_IS_D;
	
	return bits;
}

/*
 *  For clip - p1 = subject, p2 = clip
 *
 *  p1 / p2 will be destroyed!
 */
struct GH_vertex_ll * GH_polygon_boolean(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, enum GH_op_t op)
{
	if (!GH_phase_one(p1, p2))
		return NULL;

	//GH_phase_two(p1, p2, op);
	//return GH_phase_three(p1, p2);
	return NULL;
}


void GH_free_polygons(struct GH_vertex_ll * polys)
{
	struct GH_vertex_ll * cur_vertex;
	while (polys)
	{
		struct GH_vertex_ll * nextpoly = polys->nextPoly;
		cur_vertex = polys;

		while (cur_vertex)
		{
			struct GH_vertex_ll * tofree = cur_vertex;
			cur_vertex = cur_vertex->next;
			free(cur_vertex);
		}
		polys = nextpoly;
	}
}

/* Polygon accessor functions */
struct GH_vertex_ll * GH_polyPoint(struct GH_vertex_ll * v, double x, double y)
{
	struct GH_vertex_ll * nv = alloc_vertex();
	nv->c.x = x;
	nv->c.y = y;

	if (v)
		GH_insertAfter(v, nv);
	
	return nv;
}

struct GH_vertex_ll * GH_getPolyPoint(struct GH_vertex_ll *v, double * x, double * y)
{
	assert(v);
	assert(x);
	assert(y);

	*x = v->c.x;
	*y = v->c.y;

	return v->next;
}

