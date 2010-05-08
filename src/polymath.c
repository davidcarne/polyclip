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

#define COMPARE_EPSILON 0.000000001
#define _FC(a,b) (fabs((a) - (b)) < COMPARE_EPSILON)
#define POINT_COMPARE(a, b) (_FC((a)->x, (b)->x) && _FC((a)->y, (b)->y))
#define RECT_BETWEEN(a, c,d)	((a->x > c->x && a->x < d->x || a->x > d->x && a->x < c->x) || \
(a->y > c->y && a->y < d->y || a->y > d->y && a->y < c->y))

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

/* Find a meaningful vertex to calculate the second polygon from.
 * If there is no meaningful second vertex
 */
struct GH_vertex_ll * __find_intersect_nb_notnone_notdbl(struct GH_vertex_ll * v)
{
	while (v && !(v->intersect && v->neighbor->flag != FLG_NONE
                    && v->neighbor->flag != FLG_EN_EX
                    && v->neighbor->flag != FLG_EX_EN))
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
	v->next = v->prev = v->neighbor = NULL;
	v->intersect = false;
	v->flag = FLG_NONE;
	v->cross_change = false;
	v->couple = NULL;
	v->alpha = 0;
	v->done = false;
	return v;
}

struct GH_vertex_ll * GH_createIVertex(struct GH_point * A, struct GH_point * B, double a)
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

double GH_calc_WEC(struct GH_point * A, struct GH_point * B, struct GH_point * C, struct GH_point * D)
{
	assert(A != NULL); assert(B != NULL); assert(C != NULL); assert(D != NULL);
	// (A - B) dot perpendicular(C - D)
	return (double)(A->y - B->y) * (C->x - D->x) - (A->x - B->x) * (C->y - D->y);
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
	//printf("Outcode %20.16f, %50.40, %20.16lf [%d] - %x\n", B, p->y, B-p->y, p->y < B, outcode);
	return outcode;
}


// Intersects P1->P2 with Q1->Q2
// 	P1 Open, P2 Closed [aka, P1 on Q1->Q2 = NO INTERSECT]
enum intertype_e GH_intersect(struct GH_point * P1, struct GH_point * P2, struct GH_point * Q1, struct GH_point * Q2,
	double * alphaP, double * alphaQ)
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
	double WEC_P1 = GH_calc_WEC(P1, Q1, Q2, Q1);
	double WEC_P2 = GH_calc_WEC(P2, Q1, Q2, Q1);

	if (WEC_P1 * WEC_P2 <= 0)
	{
		double WEC_Q1 = GH_calc_WEC(Q1, P1, P2, P1);
		double WEC_Q2 = GH_calc_WEC(Q2, P1, P2, P1);
		if (WEC_Q1 * WEC_Q2 <= 0)
		{
			*alphaP = WEC_P1 / (WEC_P1 - WEC_P2);
			*alphaQ = WEC_Q1 / (WEC_Q1 - WEC_Q2);

			assert(isnan(*alphaP) && isnan(*alphaQ) || !isnan(*alphaP) && !isnan(*alphaQ));

			if (isnan(*alphaP) && isnan(*alphaQ))
			{
				return INTER_COINCIDE;
			}

			// TODO: Revisit assumption that these will always come out "on-the-button"
			if (*alphaP == 0.0)
				return INTER_NONE;

			if (*alphaQ == 0.0)
				return INTER_NONE;

			if (*alphaP == 1.0)
				return INTER_TOUCH;
			
			if (*alphaQ == 1.0)
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
	double aa, ab;

	// TODO: end.x should be > max_x
	end.x = point->x+1000000.0;
	end.y = point->y;

	int c = 0;
	struct GH_vertex_ll * a, *b;
	
	bool check_slope = false;
	float old_slope = 0;
	
	bool first_slope_set = false;
	float first_slope;
	//printf("Checking point %f %f\n", point->x, point->y);

	FOR_VERTEX_PAIR(poly, a, b)
		enum intertype_e i = GH_intersect(VERTEX_POINT(a), VERTEX_POINT(b), point, &end, &aa, &ab);
	
		assert(!check_slope || i == INTER_NONE || i == INTER_COINCIDE);
	
		if (i != INTER_NONE && i != INTER_COINCIDE)
			c++;
		
		//printf("Intersection of %f %f -> %f %f : %d\n", a->c.x, a->c.y, b->c.x, b->c.y, i);
		if (i == INTER_NONE && !first_slope_set)
		{
			first_slope_set = true;
			first_slope = b->c.y - a->c.y;
		}
		
		if (i == INTER_NONE && check_slope)
		{
			check_slope = false;
			float new_slope = b->c.y - a->c.y;
			assert(new_slope != 0);
			assert(old_slope != 0);
			
			// different sides of the horiz test line, so another winding
			if (new_slope * old_slope < 0)
				c++;
				
		}
		else if (i == INTER_TOUCH && !check_slope)
		{
			check_slope = true;
			old_slope = b->c.y - a->c.y;
		}
	
	END_FOR_VERTEX_PAIR(poly, a, b);

	if (check_slope)
	{
		assert(first_slope_set);
		if (first_slope * old_slope < 0)
			c++;
	}
	
	return c & 0x1; // Odd winding # means inside poly, even outside.
}

double GH_calcAlpha(struct GH_point * point, struct GH_point * start, struct GH_point * finish)
{
	assert(point);
	assert(start);
	assert(finish);

	double dx = finish->x - start->x;
	double dy = finish->y - start->y;
	assert(!(_FC(dx, 0) && _FC(dy, 0)));

	double a = 0;
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
			double a, b;
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

bool GH_tri_orient(struct GH_vertex_ll * prev, struct GH_vertex_ll * cur, struct GH_vertex_ll * next)
{
	double orient = (cur->c.x - prev->c.x) * (next->c.y - prev->c.y) - (next->c.x - prev->c.x) * (cur->c.y * prev->c.y);
	assert (orient != 0);
	return orient > 0;
}

bool GH_angle_between(double a, double b, double t)
{
    if (b < a)
        b += M_PI * 2;
    
    if (t < a)
        t += M_PI * 2;
    
    return !(t > b);
}

/*
 * a_p b_p
 *   \ /
 *    i------ h
 *   / \
 * a_n b_n
 *
 * Measures angle from *_[p_n] to h. If angle of b_p and a_n are on opposite sides of division made 
 * by a_p->i->b_n, then both polygons go the same way [aka, en is coupled to the en, ex coupled to the ex]
 *
 */
bool GH_intersection_same_way(struct GH_vertex_ll * a_p, struct GH_vertex_ll * a_n, 
                              struct GH_vertex_ll * b_p, struct GH_vertex_ll * b_n,
                              struct GH_vertex_ll * i)
{
    double a_p_sl = atan2(a_p->c.y - i->c.y, a_p->c.x - i->c.x);
    double a_n_sl = atan2(a_n->c.y - i->c.y, a_n->c.x - i->c.x);
    
    double b_p_sl = atan2(b_p->c.y - i->c.y, b_p->c.x - i->c.x);
    double b_n_sl = atan2(b_n->c.y - i->c.y, b_n->c.x - i->c.x);
    

    return GH_angle_between(a_p_sl, b_n_sl, a_n_sl) == GH_angle_between(a_p_sl, b_n_sl, b_p_sl);
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
		struct GH_vertex_ll * n_prev = Ci->neighbor->prev, * n_i = Ci->neighbor, * n_next =  Ci->neighbor->next;
		if (!n_prev) n_prev = __find_last(other);
		if (!n_next) n_next = other;
	
        if (Ci->flag == FLG_EN_EX || Ci->flag == FLG_EX_EN)
            Ci->cross_change = Ci->neighbor->cross_change = !GH_intersection_same_way(Cip, Cin, n_prev, n_next, Ci);
        else 
            Ci->cross_change = false;
    
#ifdef PHASE2_VERBOSE
		printf("(%4.2f %4.2f)->(%4.2f %4.2f)->(%4.2f %4.2f): %d %d %d\n", 
			   Cip->c.x, Cip->c.y,
			   Ci->c.x, Ci->c.y, 
			   Cin->c.x, Cin->c.y,
			   gamma_p, gamma_n, Ci->flag);
#endif
	END_FOR_VERTEX_I_CENTRI(p, Cip, Ci, Cin);
}

enum flag_type_e GHKK_invertFlag(enum flag_type_e e, bool cross_coupled)
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
            return FLG_EN_EX;
			//return cross_coupled ? FLG_EN_EX : FLG_EX_EN ;
		case FLG_EX_EN:
            return FLG_EX_EN;
			//return cross_coupled ? FLG_EX_EN : FLG_EN_EX;
		default:
			assert(false);
	}
}
/*
enum flag_type_e GHKK_doCC(enum flag_type_e e, bool cross_coupled)
{
	switch (e)
	{	
		case FLG_EN_EX:
			return cross_coupled ? FLG_EX_EN : FLG_EN_EX ;
		case FLG_EX_EN:
			return cross_coupled ? FLG_EN_EX : FLG_EX_EN;
		default:
			return e;
	}
}*/
void GHKK_phase_two_secondpoly(struct GH_vertex_ll * p, struct GH_vertex_ll * other, enum GH_op_t op)
{
	assert(p); assert(other);
	
	struct GH_vertex_ll * Ci = __find_intersect_nb_notnone_notdbl(p);
	
	struct GH_vertex_ll * Cip;
    struct GH_vertex_ll * Cin;
    bool flipflags = false;
    enum edge_status_t gamma_p, gamma_n;

    if (Ci)
    {
        Cip = Ci->prev;
        if (!Cip)
            Cip = __find_last(Ci);
        Cin = Ci->next;

        assert(Ci); assert(Cip); assert(Cin); assert(Ci->neighbor); assert(Ci->intersect);

        gamma_p = GHKK_edgeStatus(Cip, Ci, other);
        gamma_n = GHKK_edgeStatus(Ci, Cin, other);
        
        Ci->flag = GHKK_calcVertexFlag(gamma_p, gamma_n);
        enum flag_type_e Ciflag_inv = GHKK_invertFlag(Ci->flag, false);
#ifdef PHASE2_VERBOSE
        printf("(%4.2f %4.2f) -> (%4.2f %4.2f) -> (%4.2f %4.2f): gamma_p = %x, gamma_n = %x, Ci->flag = %x, CiInv = %x, Ci->neighbor->flag = %x\n", 
            Cip->c.x, Cip->c.y, Ci->c.x, Ci->c.y, Cin->c.x, Cin->c.y, 
            gamma_p, gamma_n, Ci->flag, Ciflag_inv, Ci->neighbor->flag);
#endif
    
        assert(Ci->flag == Ci->neighbor->flag || Ciflag_inv == Ci->neighbor->flag);

        flipflags = (Ciflag_inv == Ci->neighbor->flag);
    }
    
	struct GH_vertex_ll * i;
	FOR_VERTEX_I(p, i)
        if (i->neighbor->flag == FLG_EN_EX || i->neighbor->flag == FLG_EX_EN)
        {
            Cip = i->prev;
            Cin = i->next;
            
            if (!Cip)
                Cip =  __find_last(i); // TODO: expensive search not needed
                // We need to flip if one is inside other
                // Calculate the first EN_EX, save in boolean, flip boolean every EN/EX encountered
                // Do not count second element of a couple
            
            if (!Cin)
                Cin = p;
                
            gamma_p = GHKK_edgeStatus(Cip, i, other);
            gamma_n = GHKK_edgeStatus(i, Cin, other);
            i->flag = GHKK_calcVertexFlag(gamma_p, gamma_n);
        } else {
            if (flipflags)
                i->flag = GHKK_invertFlag(i->neighbor->flag, i->cross_change);
            else
                i->flag = i->neighbor->flag;
            //	i->flag = GHKK_doCC(i->neighbor->flag, i->cross_change);
        }
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

void GHKK_delete_flag(struct GH_vertex_ll * v, enum flag_type_e flag)
{
	if (flag == FLG_NONE)
		return;
	
	assert(v);
	assert(v->intersect);
	assert(v->flag != FLG_NONE);
	assert(v->neighbor);
	assert(!v->couple || v->couple->flag == v->flag);
	
	enum flag_type_e nf = FLG_NONE;
	
	if (v->flag == flag)
	{
		v->flag = FLG_NONE;
		v->intersect = false;
		
		if (v->couple)
		{
			v->couple->flag = FLG_NONE;
			v->couple->intersect = false;
		}
	}
	else if (v->flag == FLG_EN_EX && flag == FLG_EN)
	{
		assert (!v->couple);
		v->flag = FLG_EX;
		
	} else if (v->flag == FLG_EX_EN && flag == FLG_EX)
	{
		assert (!v->couple);
		v->flag = FLG_EN;
	}
	else if (v->flag == FLG_EN_EX && flag == FLG_EX)
	{
		assert (!v->couple);
		v->flag = FLG_EN;
		
	} else if (v->flag == FLG_EX_EN && flag == FLG_EN)
	{
		assert (!v->couple);
		v->flag = FLG_EX;
	} else if (v->flag == FLG_EX_EN && flag == FLG_EX_EN)
	{
		assert (!v->couple);
		v->flag = FLG_NONE;
	} else if (v->flag == FLG_EN_EX && flag == FLG_EN_EX)
	{
		assert (!v->couple);
		v->flag = FLG_NONE;
	} else {
		assert(false);
	}

}

enum flag_type_e GH_choose_flag_intersect(enum flag_type_e e, enum trv_dir d)
{
	switch (e)
	{
		case FLG_NONE:
			return FLG_NONE;
		case FLG_EN:
			return FLG_EN;
		case FLG_EX:
			return FLG_EX;
			
		case FLG_EN_EX:
			if (trvIsAcross(d))
				return FLG_EN_EX;
			return trvIsForward(d) ? FLG_EN : FLG_EX;
			
		case FLG_EX_EN:
			return trvIsForward(d) ? FLG_EX : FLG_EN;
	}
}

enum flag_type_e GH_choose_flag_union(enum flag_type_e e, enum trv_dir d)
{
	switch (e)
	{
		case FLG_NONE:
			return FLG_NONE;
		case FLG_EN:
			return FLG_EN;
		case FLG_EX:
			return FLG_EX;
		case FLG_EN_EX:
			return FLG_EN;
		case FLG_EX_EN:
			return FLG_EX;
	}
}

enum trv_dir GH_startdir_union(enum flag_type_e flag)
{
	
	// Choose starting direction based on flag.
	switch (flag)
	{
		case FLG_EX:
			return DIR_REV;
		case FLG_EN:
			return DIR_FWD;
		case FLG_EN_EX:
			return DIR_FWD;
		case FLG_EX_EN:
			return DIR_ACFWD;	// Only get to this via the other vertex
	}
}

enum trv_dir GH_startdir_intersect(enum flag_type_e flag)
{
	
	// Choose starting direction based on flag.
	switch (flag)
	{
		case FLG_EX:
			return DIR_FWD;
			break;
		case FLG_EN:
			return DIR_REV;
			break;
		case FLG_EN_EX:
			return DIR_ACFWD;
			break;
		case FLG_EX_EN:	// either works
			return DIR_FWD;
			break;
	}
}

enum trv_dir trvAcross(enum trv_dir a)
{
	switch (a)
	{
		case DIR_FWD:
			return DIR_ACFWD;
		case DIR_REV:
			return DIR_ACREV;
		case DIR_ACFWD:
			return DIR_FWD;
		case DIR_ACREV:
			return DIR_REV;
	}
}
enum trv_dir trvReverse(enum trv_dir a)
{
	switch (a)
	{
		case DIR_FWD:
			return DIR_REV;
		case DIR_REV:
			return DIR_FWD;
		case DIR_ACFWD:
			return DIR_ACREV;
		case DIR_ACREV:
			return DIR_ACFWD;
	}
			
}

bool trvIsAcross(enum trv_dir a)
{
	switch (a)
	{
		case DIR_FWD:
		case DIR_REV:
			return false;
		case DIR_ACFWD:
		case DIR_ACREV:
			return true;
	}
}

bool trvIsForward(enum trv_dir a)
{
	switch (a)
	{
		case DIR_FWD:
		case DIR_ACFWD:
			return true;
		case DIR_REV:
		case DIR_ACREV:
			return false;
	}
}

bool trvDirSame(enum trv_dir a, enum trv_dir b)
{
	return trvIsForward(a) == trvIsForward(b);
}

struct GH_vertex_ll * GH_followTransversal(struct GH_vertex_ll * src, enum trv_dir in, enum trv_dir out, enum flag_type_e flag)
{
	
	GHKK_delete_flag(src, flag);
	
	switch (out)
	{
		case DIR_ACFWD:
			assert(src->neighbor);
			return src->neighbor;
			
		case DIR_ACREV:
			assert(src->neighbor);
			return src->neighbor;
			
		case DIR_FWD:
			if (trvDirSame(in, out) && src->flag != FLG_NONE)
				return src;
			return src->next;
			
		case DIR_REV:
			if (trvDirSame(in, out) && src->flag != FLG_NONE)
				return src;
			return src->prev;
	}
	
}


enum trv_dir GH_state_transition_union(enum trv_dir direction, enum flag_type_e chosen_flag)
{
    
	switch (chosen_flag)
	{
		case FLG_EN:
			if (trvIsAcross(direction))
			{
				return trvReverse(trvAcross(direction));
			} else {
				return trvAcross(direction);
			}
			break;
			
		case FLG_EX:
			if (trvIsAcross(direction))
			{
				return trvAcross(direction);
			} else {
				return trvReverse(trvAcross(direction));
			}
			break;
			
			// make warning go away
		default:
			assert(false);
	}

}

enum trv_dir GH_state_transition_intersect(enum trv_dir indir, enum flag_type_e chosen_flag)
{
	switch (chosen_flag)
	{
		case FLG_EN:
			if (trvIsAcross(indir))
			{
				return DIR_FWD;
			} else {
				return DIR_ACFWD;
			}
			break;
			
		case FLG_EX:
			if (trvIsAcross(indir))
			{
				return DIR_REV;
			} else {
				return DIR_ACFWD;
			}
			break;
		
		case FLG_EN_EX:
			if (trvIsAcross(indir))
				return DIR_ACFWD;
				
			// make warning go away
		default:
			assert(false);
	}
	
}

// Prepare for phase 3 by closing the polygon rings
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

bool GHKK_phase_3_fp(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, enum GH_op_t op, struct GH_vertex_ll ** outpoly)
{
	struct GH_vertex_ll * l;
	assert(p1); assert(p2); assert(outpoly);
	
	enum trv_dir direction;
	l = __find_meets_p3_criteria(p1);

	if (!l)
		return false;
	
	switch (op)
	{
		case GH_op_union:
			direction = GH_startdir_union(l->flag);
			break;
		case GH_op_intersect:
			direction = GH_startdir_intersect(l->flag);
			break;
		default:
			assert(false);
	}
	
	enum trv_dir start_dir = direction;
	
	struct GH_vertex_ll * v = l;
	
	struct GH_vertex_ll * newpoly = NULL;
	struct GH_vertex_ll * nextout = NULL;
	struct GH_vertex_ll * last = NULL;
	do {
		// If not across-path, and not internal
		if (!trvIsAcross(direction) && v != last)
			nextout = GH_polyPoint(nextout, v->c.x, v->c.y);
		
		if (!newpoly)
			newpoly = nextout;

		enum trv_dir newdir;
		struct GH_vertex_ll * newvert;
		
		
#ifdef PHASE3_VERBOSE
		printf("Starting vertex %4.2f %4.2f %s [%p] - %s\n", v->c.x, v->c.y, flg_dec(v->flag), v, td_dec(direction));
#endif
		if (!v->intersect)
		{
			newvert = GH_followTransversal(v, direction, direction, FLG_NONE);
			newdir = direction;
		} else {
			
			enum flag_type_e chosen_flag;
			switch (op)
			{
				case GH_op_union:
					chosen_flag = GH_choose_flag_union(v->flag, direction);
					newdir = GH_state_transition_union(direction,chosen_flag);
					break;
				case GH_op_intersect:
					chosen_flag = GH_choose_flag_intersect(v->flag, direction);
					newdir = GH_state_transition_intersect(direction,chosen_flag);
					break;
			}
			
#ifdef PHASE3_VERBOSE	
			printf("\tChosen flag %s; incoming dir %s, outgoing dir %s\n", flg_dec(chosen_flag), td_dec(direction), td_dec(newdir));
#endif
			newvert = GH_followTransversal(v, direction, newdir, chosen_flag);
#ifdef PHASE3_VERBOSE	
			printf("\tTraversed to vertex %4.2f %4.2f %s [%p]\n",newvert->c.x, newvert->c.y, flg_dec(newvert->flag),newvert);
#endif
			
		}
		last = v;
		v = newvert;
		direction = newdir;
		
	} while (v != l || start_dir != direction);
	
	*outpoly = newpoly;
	return true;
}



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
struct GH_polygon_ll * GH_polygon_boolean(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, enum GH_op_t op)
{
	if (!GH_phase_one(p1, p2))
		return NULL;

	GH_phase_two(p1, p2, op);
	GHKK_phase_3_prep(p1, p2);
	
	struct GH_polygon_ll * head = NULL;
	struct GH_polygon_ll  ** cur = &head;
	
	struct GH_vertex_ll * newverticies;
	while (GHKK_phase_3_fp(p1, p2, op, &newverticies))
	{
		*cur = (struct GH_polygon_ll*)malloc(sizeof(struct GH_polygon_ll));
		(*cur)->next = NULL;
		(*cur)->firstv = newverticies;
		cur = &(*cur)->next;
	}
	
	return head;
}


void GH_free_polygons(struct GH_vertex_ll * polys)
{
	struct GH_vertex_ll * cur_vertex;
	
	// If the loop has been closed, then we need to break it
	if (polys->prev)
		polys->prev->next = NULL;
	
	cur_vertex = polys;

	while (cur_vertex)
	{
		struct GH_vertex_ll * tofree = cur_vertex;
		cur_vertex = cur_vertex->next;
		free(cur_vertex);
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

