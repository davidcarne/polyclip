/*
 * Copyright 2009, 2001 David Carne. Released under the revised BSD License.
 *
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifndef INT_ASSERT
#include <assert.h>
#else
#include "../tests/libtest.h"
#define assert LT_REQUIRE
#define malloc LT_LOGGED_MALLOC
#define free LT_LOGGED_FREE
#endif

#include "polymath.h"
#include "polymath_internal.h"


#define COMPARE_EPSILON DBL_EPSILON
#define _FC(a,b) (fabs((a) - (b)) < COMPARE_EPSILON)
#define POINT_COMPARE(a, b) (_FC((a)->x, (b)->x) && _FC((a)->y, (b)->y))
#define RECT_BETWEEN(a, c,d)	((a->x > c->x && a->x < d->x || a->x > d->x && a->x < c->x) || \
(a->y > c->y && a->y < d->y || a->y > d->y && a->y < c->y))

/**
 * Find the next non-intersecting vertex [including the passed element]
 */
static struct PC_vertex_ll * __find_non_intersect(struct PC_vertex_ll * v)
{
	while (v && v->intersect)
		v = v->next;
	return v;
}

/**
 * Find the next intersecting vertex [including the passed element]
 * Will not follow all the way around the polygon if it has been closed, as is done in stage 3.
 */
static struct PC_vertex_ll * __find_intersect(struct PC_vertex_ll * vo)
{
	struct PC_vertex_ll * v = vo;
	while (v && !v->intersect)
	{
		v = v->next;
		
		if (v == vo)
			return NULL;
	}
	return v;
}

/**
 * Find a meaningful vertex to calculate the second polygon from.
 * If there is no meaningful second vertex, return NULL
 */
static struct PC_vertex_ll * __find_intersect_nb_notnone_notdbl(struct PC_vertex_ll * v)
{
	while (v && !(v->intersect && v->neighbor->flag != FLG_NONE
                    && v->neighbor->flag != FLG_EN_EX
                    && v->neighbor->flag != FLG_EX_EN))
		v = v->next;
	return v;
}

/**
 * Find the last vertex of a polygon.
 * Deprecated, to mark inefficient code that needs cleanup
 */
static struct PC_vertex_ll * __find_last(struct PC_vertex_ll * vo) __attribute__((deprecated));
static struct PC_vertex_ll * __find_last(struct PC_vertex_ll * vo)
{
	struct PC_vertex_ll * v = vo;
	while (v && v->next)
	{
		v = v->next;
		
		if (v == vo)
			return NULL;
	}
	return v;
}

/**
 * Find the vertex that meets the criteria for a starting vertex in phase 3
 */
static struct PC_vertex_ll * __find_meets_p3_criteria(struct PC_vertex_ll * vo)
{
	struct PC_vertex_ll * v = vo;
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

/**
 * Allocate an empty vertex
 */
struct PC_vertex_ll * PC_alloc_vertex()
{
	struct PC_vertex_ll * v = (struct PC_vertex_ll *)malloc(sizeof(struct PC_vertex_ll));
	
	if (!v) return NULL;
	
	v->c.x = v->c.y = INFINITY;
	v->next = v->prev = v->neighbor = NULL;
	v->intersect = false;
	v->flag = FLG_NONE;
	v->cross_change = false;
	v->couple = NULL;
	v->alpha = 0;
	return v;
}

/**
 * Create an Intersection vertex between points A and B, with alpha in (0..1) mapping to (A..B)
 */
struct PC_vertex_ll * PC_createIVertex(struct PC_point * A, struct PC_point * B, double alpha)
{
	assert(A != NULL);
	assert(B != NULL);
	assert(alpha > 0 && alpha < 1);

	double x = A->x * (1 - alpha) + B->x * alpha;
	double y = A->y * (1 - alpha) + B->y * alpha;
	struct PC_vertex_ll * v = PC_alloc_vertex();
	v->alpha = alpha;
	v->intersect = true;
	v->c.x = x;
	v->c.y = y;
	return v;
}

/**
 * Link two verticies as Neighbours .
 */
void PC_linkNeighbours(struct PC_vertex_ll * a, struct PC_vertex_ll * b)
{
	assert(a != NULL);
	assert(b != NULL);

	a->neighbor = b;
	b->neighbor = a;
}

/**
 * Insert a Vertex into the linked-list of verticies
 * ip is the vertex to insert after, ti is the vertex to insert
 */
void PC_insertAfter(struct PC_vertex_ll * ip, struct PC_vertex_ll * ti)
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

/**
 * Unlink a vertex from the linked-list of verticies. Does not dispose of memory.
 */
void PC_unlink(struct PC_vertex_ll * v)
{
	assert(v);
	
	if (v->prev)
		v->prev->next = v->next;
	if (v->next)
		v->next->prev = v->prev;
}

/**
 * Insert vertex I into the linked list between P1 and P2 such that 
 *  it is ordered correctly among already generated intersection verticies.
 */
void PC_sortedInsert(struct PC_vertex_ll * P1, struct PC_vertex_ll * P2, struct PC_vertex_ll * I)
{
	assert(P1 != NULL);	assert(P2 != NULL);	assert(I != NULL);
	assert(!P1->intersect);	assert(!P2->intersect);	assert(I->intersect);
	assert(P1->alpha == 0);	assert(P2->alpha == 0);	assert(I->alpha > 0 && I->alpha < 1);

	struct PC_vertex_ll * J = P1;
    
	// Need to find vertex J
    //      - That doesn't intersect, therefore has alpha of 0, which is below
    //        OR
    //      - Has alpha less than our alpha
    //  AND
    //      - Has no point following it [wraps around, ergo, next point is not an intersection, alpha of 1]
    //      - Has a nonintersection point following it, [alpha of 1]
    //      - Has an intersection point greater than our alpha
	while (	J && !((!J->intersect || J->alpha <= I->alpha) && 
		(!J->next || !J->next->intersect || J->next->alpha >= I->alpha)))
	{
		J = J->next;
		assert(J != P2);
	}
    
	assert(J);
    PC_insertAfter(J,I);
}

/**
 * Calculate the window edge coordinates
 */
double PC_calc_WEC(struct PC_point * A, struct PC_point * B, struct PC_point * C, struct PC_point * D)
{
	assert(A != NULL); assert(B != NULL); assert(C != NULL); assert(D != NULL);
    
	// (A - B) dot perpendicular(C - D)
	return (double)(A->y - B->y) * (C->x - D->x) - (A->x - B->x) * (C->y - D->y);
}

/**
 * Calculate the outcode for a point and a set of bounds forming a rectangle.
 */
int PC_outcode(struct PC_point * p, double L, double R, double B, double T)
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


/**
 * Calculates the intersection of P1->P2 with Q1->Q2.
 * P1 Open, P2 Closed [aka, P1 on Q1->Q2 = NO INTERSECT] Same with interval of (Q1->Q2]
 */
enum intertype_e PC_intersect(struct PC_point * P1, struct PC_point * P2, struct PC_point * Q1, struct PC_point * Q2,
	double * alphaP, double * alphaQ)
{
	assert(P1 != NULL); assert(P2 != NULL); assert(Q1 != NULL); assert(Q2 != NULL);
	assert(alphaP != NULL); assert(alphaQ != NULL);

	
	*alphaQ = -INFINITY;
	*alphaP = -INFINITY;
	
    /* Calculate a bounding box for line Q */
	double winL = Q1->x > Q2->x ? Q2->x : Q1->x;
	double winR = Q1->x > Q2->x ? Q1->x : Q2->x;
	double winB = Q1->y > Q2->y ? Q2->y : Q1->y;
	double winT = Q1->y > Q2->y ? Q1->y : Q2->y;

    /* Calculate outcodes for P1 + P2 against the bounding box of Q1 + Q2 */
	int outcode_P1 = PC_outcode(P1, winL, winR, winB, winT);
	int outcode_P2 = PC_outcode(P2, winL, winR, winB, winT);
	
	/* If any bits set in both outcodes, no intersection possible, so abort.
     * This works because it means that one line must be fully outside of the bounding box
     * of the other line.
     */
	if (outcode_P1 & outcode_P2)
		return INTER_NONE;

	/* Now do the window-edge-coordinate method of finding intersections */
	double WEC_P1 = PC_calc_WEC(P1, Q1, Q2, Q1);
	double WEC_P2 = PC_calc_WEC(P2, Q1, Q2, Q1);

	if (WEC_P1 * WEC_P2 <= 0)
	{
		double WEC_Q1 = PC_calc_WEC(Q1, P1, P2, P1);
		double WEC_Q2 = PC_calc_WEC(Q2, P1, P2, P1);
		if (WEC_Q1 * WEC_Q2 <= 0)
		{
			*alphaP = WEC_P1 / (WEC_P1 - WEC_P2);
			*alphaQ = WEC_Q1 / (WEC_Q1 - WEC_Q2);

			assert(isnan(*alphaP) && isnan(*alphaQ) || !isnan(*alphaP) && !isnan(*alphaQ));

			if (isnan(*alphaP) && isnan(*alphaQ))
			{
				return INTER_COINCIDE;
			}

			/* TODO: Revisit assumption that these will always come out "on-the-button" */
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

/**
 * Checks if a point is within a polygon
 * TODO: Many bugfixes needed
 */
bool PC_pointInPoly(struct PC_vertex_ll * poly, struct PC_point * point)
{
	assert(poly); assert(point);
	
	/* TODO: optimize + fix */
	struct PC_point end;
	double aa, ab;

	/* TODO: end.x should be > max_x */
	end.x = point->x+1000000.0;
	end.y = point->y;

	int c = 0;
	struct PC_vertex_ll * a, *b;
	
	bool check_slope = false;
	float old_slope = 0;
	
	bool first_slope_set = false;
	float first_slope;
#ifdef POINT_IN_POLY_VERBOSE
	printf("Checking point %f %f\n", point->x, point->y);
#endif
	FOR_VERTEX_PAIR(poly, a, b)
		enum intertype_e i = PC_intersect(VERTEX_POINT(a), VERTEX_POINT(b), point, &end, &aa, &ab);
	
		assert(!check_slope || i == INTER_NONE || i == INTER_COINCIDE);
	
		if (i != INTER_NONE && i != INTER_COINCIDE)
			c++;
#ifdef POINT_IN_POLY_VERBOSE
		printf("Intersection of %f %f -> %f %f : %d\n", a->c.x, a->c.y, b->c.x, b->c.y, i);
#endif
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
			
			/* different sides of the horiz test line, so another winding*/
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
	
	return c & 0x1; /* Odd winding # means inside poly, even outside. */
}

/*
 * Calculate the normalized position of point on line start->finish, in range [0..1]
 */
double PC_calcAlpha(struct PC_point * point, struct PC_point * start, struct PC_point * finish)
{
	assert(point);
	assert(start);
	assert(finish);

	double dx = finish->x - start->x;
	double dy = finish->y - start->y;
	assert(!(_FC(dx, 0) && _FC(dy, 0)));

	double a = 0;
	if (dx != 0)
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

/*
 * Clone, insert and link a vertex into a polygon based on an existing vertex in another polygon
 */
void PC_insertLinkClonedVertex(struct PC_vertex_ll * toclone, struct PC_vertex_ll * before, struct PC_vertex_ll * after)
{
	assert(toclone);
	assert(before);
	assert(after);
	assert(!toclone->neighbor);
	assert(before != after);
	assert(toclone != before);
	assert(toclone != after);

	struct PC_vertex_ll * i1 = PC_alloc_vertex();

	i1->alpha = PC_calcAlpha(VERTEX_POINT(toclone), VERTEX_POINT(before), VERTEX_POINT(after));


	i1->intersect = true;
	i1->c.x = toclone->c.x;
	i1->c.y = toclone->c.y;

	/* NOTE: we do NOT mark the original point as an intersection yet!
     * [The original point could be a corner, and therefore must be considered when doing sorted inserts]
     */
	PC_linkNeighbours(i1, toclone);
	PC_sortedInsert(before, after, i1);
}

/**
 * Create all intersection points on both polygons, and link them together
 *
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
bool PC_phase_one(struct PC_vertex_ll * subject, struct PC_vertex_ll * clip)
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
	struct PC_vertex_ll * s0, * s1;
	struct PC_vertex_ll * c0, * c1;

	bool intersect_found = false;

	/* poly insertion list pointers */
	struct PC_vertex_ll * sI = NULL, * cI = NULL;
	
	// Iterate across both polygons
	FOR_VERTEX_NI_PAIR(subject, s0, s1)
		FOR_VERTEX_NI_PAIR(clip, c0, c1)
			double a, b;
			enum intertype_e inter_type = PC_intersect(
					VERTEX_POINT(s0), VERTEX_POINT(s1), 
					VERTEX_POINT(c0), VERTEX_POINT(c1),
					&a, &b);

			if (inter_type != INTER_NONE)
			{
				if (inter_type == INTER_CROSS)
				{
                    // The lines are simply crossing, so generate 
                    // verticies at the intersection point
					struct PC_vertex_ll * i1 = PC_createIVertex(
						VERTEX_POINT(s0), VERTEX_POINT(s1),
						a);
					struct PC_vertex_ll * i2 = PC_createIVertex(
						VERTEX_POINT(c0), VERTEX_POINT(c1),
						b);
				
                    // Link them together
					PC_linkNeighbours(i1, i2);

                    // And add them to both polygons
					PC_sortedInsert(s0, s1, i1);
					PC_sortedInsert(c0, c1, i2);
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
							PC_insertLinkClonedVertex(c1, s0, s1);
						else
							PC_insertLinkClonedVertex(s1, c0, c1);
					} else {
#ifdef PHASE1_VERBOSE
						printf("\tLinked two verticies [%f %f]\n",a,b);
#endif
						// We're touching at the ends [both a == 1 && b == 1]
						PC_linkNeighbours(s1, c1);
					}

				} else if (inter_type == INTER_COINCIDE) {
					int bits = PC_lineCoincideBits(
						VERTEX_POINT(s0), VERTEX_POINT(s1),
						VERTEX_POINT(c0), VERTEX_POINT(c1));
					
					/* A on CD intersection already generated by touch comparison*/
					if (bits & B_ONCD)
						PC_insertLinkClonedVertex(s1, c0, c1);
					/* C on AB intersection already generated by touch comparison*/
					if (bits & D_ONAB)
						PC_insertLinkClonedVertex(c1, s0, s1);
					
                    /* Link verticies if the polygons share a vertex */
					if (bits & A_IS_C)
						PC_linkNeighbours(s0, c0);
					if (bits & A_IS_D)
						PC_linkNeighbours(s0, c1);
					if (bits & B_IS_C)
						PC_linkNeighbours(s1, c0);
					if (bits & B_IS_D)
						PC_linkNeighbours(s1, c1);

				}
				intersect_found = true;
			}
		END_FOR_VERTEX_NI_PAIR(clip, c0, c1);
	END_FOR_VERTEX_NI_PAIR(subject, s0, s1);

    /* Walk both polygons and mark as intersections anything we found a neighbor for
     * This is not yet done for any "touch" intersections on the "touching" polygon
     */
	struct PC_vertex_ll * i;
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

/**
 * Check if (a,b) maps to (n_a,n_b) or (n_b, n_a) on the other polygon
 */
bool PC_is_on(struct PC_vertex_ll * a, struct PC_vertex_ll * b)
{
	if (!a->intersect || !b->intersect)
		return false;

	assert(a->neighbor);
	assert(b->neighbor);

	struct PC_vertex_ll * n_a = a->neighbor;
	struct PC_vertex_ll * n_b = b->neighbor;

	// wraparound case
	if (n_a->next == NULL && n_b->prev == NULL ||
		n_a->prev == NULL && n_b->next == NULL)
		return true;
	
	// standard case
	if (n_a->next == n_b || n_b->next == n_a)
		return true;

	return false;
}


/**
 * Find the midpoint of a line given by (a,b)
 */
struct PC_point PC_midPoint(struct PC_point * a, struct PC_point * b)
{
	assert(a);
	assert(b);
	struct PC_point mid;
	mid.x = (a->x + b->x) / 2.0;
	mid.y = (a->y + b->y) / 2.0;
	return mid;
}

/**
 * Calculates edge status according to Kim-Kim 2006 4.1
 * 
 * Lines outside the other polygon get status OUT, lines that share an edge with the other polygon get status ON,
 * lines that are entirely inside the other polygon get status IN
 */
enum edge_status_t PC_edgeStatus(struct PC_vertex_ll * a, struct PC_vertex_ll * b, struct PC_vertex_ll * other)
{
	assert(a);
	assert(b);
	assert(other);

	if (PC_is_on(a, b))
		return edge_on;

	struct PC_point mid = PC_midPoint(VERTEX_POINT(a),VERTEX_POINT(b));
	return PC_pointInPoly(other, &mid) ? edge_in : edge_out;
}

/**
 * Calculate the flag of a vertex, given the edge status of the previous and next lines
 */
enum flag_type_e PC_calcVertexFlag(enum edge_status_t gamma_p, enum edge_status_t gamma_n)
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

/**
 * returns true if t is within the closed angle (a,b), with the angle being clipped to less than 2pi
 */
bool PC_angle_between(double a, double b, double t)
{
    if (b < a)
        b += M_PI * 2;
    
    if (t < a)
        t += M_PI * 2;
    
    return !(t > b);
}

/**
 * Returns true if two polygons on each side of an intersection run the same way
 */
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
 * TODO: Can this be rewritten without trig?
 */
bool PC_intersection_same_way(struct PC_vertex_ll * a_p, struct PC_vertex_ll * a_n, 
                              struct PC_vertex_ll * b_p, struct PC_vertex_ll * b_n,
                              struct PC_vertex_ll * i)
{
    double a_p_sl = atan2(a_p->c.y - i->c.y, a_p->c.x - i->c.x);
    double a_n_sl = atan2(a_n->c.y - i->c.y, a_n->c.x - i->c.x);
    
    double b_p_sl = atan2(b_p->c.y - i->c.y, b_p->c.x - i->c.x);
    double b_n_sl = atan2(b_n->c.y - i->c.y, b_n->c.x - i->c.x);
    

    return PC_angle_between(a_p_sl, b_n_sl, a_n_sl) == PC_angle_between(a_p_sl, b_n_sl, b_p_sl);
}

/**
 * Calculate intersection flags on the first polygon. Slower than the pass done on the second.
 * TODO: pass full first + last polygon pointers
 */
void PC_phase_two_firstpoly(struct PC_vertex_ll * p, struct PC_vertex_ll * other, enum PC_op_t op)
{
	assert(p); assert(other);
	
	enum edge_status_t gamma_p, gamma_n;

	struct PC_vertex_ll * Cip, * Ci, * Cin;
	FOR_VERTEX_I_CENTRI(p, Cip, Ci, Cin)
    
        /* Calculate previous and next edge statuses */
		gamma_p = PC_edgeStatus(Cip, Ci, other);
		gamma_n = PC_edgeStatus(Ci, Cin, other);
    
        /* Using edge statuses, calculate vertex flag */
		Ci->flag = PC_calcVertexFlag(gamma_p, gamma_n);
    
        /* find the prev + next verticies on the other polygon */
		struct PC_vertex_ll * n_prev = Ci->neighbor->prev, * n_i = Ci->neighbor, * n_next =  Ci->neighbor->next;
		if (!n_prev) n_prev = __find_last(other);
		if (!n_next) n_next = other;
	
        /* Calculate cross_change based on intersection ordering.
         * Cross change indicates that we need to change direction when crossing the neighbour link
         * to avoid creating a self intersection point.
         */
        if (Ci->flag == FLG_EN_EX || Ci->flag == FLG_EX_EN)
            Ci->cross_change = Ci->neighbor->cross_change = !PC_intersection_same_way(Cip, Cin, n_prev, n_next, Ci);
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

/**
 * Complement an intersection flag
 */
enum flag_type_e PC_invertFlag(enum flag_type_e e)
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
		case FLG_EX_EN:
		default:
			assert(false);
	}
}

/**
 * Fast vertex flag setting for the second polygon based on the flags in the first
 *
 * TODO: better work needs to be done RE: FLG_EN_EX / FLG_EX_EN. See TODO below
 */
void PC_phase_two_secondpoly(struct PC_vertex_ll * p, struct PC_vertex_ll * other, enum PC_op_t op)
{
	assert(p); assert(other);
	
    
	struct PC_vertex_ll * Ci = __find_intersect_nb_notnone_notdbl(p);
	
	struct PC_vertex_ll * Cip;
    struct PC_vertex_ll * Cin;

    bool flipflags = false;
    enum edge_status_t gamma_p, gamma_n;

    /* If Enter / Exit verticies exist, calculate if the second polygon flags are
     * the same as the first, or the complement
     */
    if (Ci)
    {
        
        /*
         * TODO: pass polygons with integral last ptr to here
         */
        Cip = Ci->prev;
        if (!Cip)
            Cip = __find_last(Ci);
        Cin = Ci->next;

        assert(Ci); assert(Cip); assert(Cin); assert(Ci->neighbor); assert(Ci->intersect);

        gamma_p = PC_edgeStatus(Cip, Ci, other);
        gamma_n = PC_edgeStatus(Ci, Cin, other);
        
        Ci->flag = PC_calcVertexFlag(gamma_p, gamma_n);
        enum flag_type_e Ciflag_inv = PC_invertFlag(Ci->flag);
        
        assert(Ci->flag == Ci->neighbor->flag || Ciflag_inv == Ci->neighbor->flag);
        
#ifdef PHASE2_VERBOSE
        printf("(%4.2f %4.2f) -> (%4.2f %4.2f) -> (%4.2f %4.2f): gamma_p = %x, gamma_n = %x, Ci->flag = %x, CiInv = %x, Ci->neighbor->flag = %x\n", 
            Cip->c.x, Cip->c.y, Ci->c.x, Ci->c.y, Cin->c.x, Cin->c.y, 
            gamma_p, gamma_n, Ci->flag, Ciflag_inv, Ci->neighbor->flag);
#endif
    
        flipflags = (Ciflag_inv == Ci->neighbor->flag);
    }
    
	struct PC_vertex_ll * i;
	FOR_VERTEX_I(p, i)
        if (i->neighbor->flag == FLG_EN_EX || i->neighbor->flag == FLG_EX_EN)
        {
            Cip = i->prev;
            Cin = i->next;
            
            if (!Cip)
                Cip =  __find_last(i); /* TODO: expensive search not needed
                                        * We need to flip if one is inside other
                                        * Calculate the first EN_EX, save in boolean, flip boolean every EN/EX encountered
                                        * Do not count second element of a couple 
                                        */
            
            if (!Cin)
                Cin = p;
                
            gamma_p = PC_edgeStatus(Cip, i, other);
            gamma_n = PC_edgeStatus(i, Cin, other);
            i->flag = PC_calcVertexFlag(gamma_p, gamma_n);
        } else {
            if (flipflags)
                i->flag = PC_invertFlag(i->neighbor->flag);
            else
                i->flag = i->neighbor->flag;
        }
	END_FOR_VERTEX_I(p, i);

}

/**
 * Clear intersection / neighbor markers for FLG_NONE points, since we just walk right by them during transversal.
 */
void PC_phase_two_clearNoFlags(struct PC_vertex_ll * p1)
{
	assert(p1);
	
	struct PC_vertex_ll * Ci;
	
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

/**
 * Link sequential En, En or Ex, Ex verticies into couples
 */
void PC_phase_two_buildcouples(struct PC_vertex_ll * p1)
{
	assert(p1);
	
	struct PC_vertex_ll * a, * b;
	
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

/**
 * Full Phase 2 of the polygon clipping.
 *
 * In this stage, vertex flags + couples are calculated for all intersection points on both verticies
 * and uninteresting verticies are marked as nonintersecting.
 *
 * an operation is specified since certain operations [subtraction for example] involve inverting the "inside"
 * check of the polygon to be subtracted.
 */
void PC_phase_two(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2, enum PC_op_t op)
{
	assert(p1); assert(p2);
	
	PC_phase_two_firstpoly(p2, p1, op);
	PC_phase_two_secondpoly(p1, p2, op);
	PC_phase_two_clearNoFlags(p2);
	PC_phase_two_buildcouples(p2);
}

/**
 * Delete a flag [or flags] from a vertex, and its couple.
 */
void PC_delete_flag(struct PC_vertex_ll * v, enum flag_type_e flag)
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

enum trv_dir PC_trvAcross(enum trv_dir a)
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
enum trv_dir PC_trvReverse(enum trv_dir a)
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

bool PC_trvIsAcross(enum trv_dir a)
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

bool PC_trvIsForward(enum trv_dir a)
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

bool PC_trvDirSame(enum trv_dir a, enum trv_dir b)
{
	return PC_trvIsForward(a) == PC_trvIsForward(b);
}

/**
 * Clear the selected flag on the current vertex, and follow the transversal to the next vertex
 */
struct PC_vertex_ll * PC_followTransversal(struct PC_vertex_ll * src, enum trv_dir in, enum trv_dir out, enum flag_type_e flag)
{
	
	PC_delete_flag(src, flag);
	
	switch (out)
	{
		case DIR_ACFWD:
			assert(src->neighbor);
			return src->neighbor;
			
		case DIR_ACREV:
			assert(src->neighbor);
			return src->neighbor;
			
		case DIR_FWD:
			if (PC_trvDirSame(in, out) && src->flag != FLG_NONE)
				return src;
			return src->next;
			
		case DIR_REV:
			if (PC_trvDirSame(in, out) && src->flag != FLG_NONE)
				return src;
			return src->prev;
	}
	
}


enum flag_type_e PC_choose_flag_intersect(enum flag_type_e e, enum trv_dir d)
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
			if (PC_trvIsAcross(d))
				return FLG_EN_EX;
			return PC_trvIsForward(d) ? FLG_EN : FLG_EX;
			
		case FLG_EX_EN:
			return PC_trvIsForward(d) ? FLG_EX : FLG_EN;
	}
}

enum flag_type_e PC_choose_flag_union(enum flag_type_e e, enum trv_dir d)
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

enum trv_dir PC_startdir_union(enum flag_type_e flag)
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

enum trv_dir PC_startdir_intersect(enum flag_type_e flag)
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


enum trv_dir PC_state_transition_union(enum trv_dir direction, enum flag_type_e chosen_flag)
{
    
	switch (chosen_flag)
	{
		case FLG_EN:
			if (PC_trvIsAcross(direction))
			{
				return PC_trvReverse(PC_trvAcross(direction));
			} else {
				return PC_trvAcross(direction);
			}
			break;
			
		case FLG_EX:
			if (PC_trvIsAcross(direction))
			{
				return PC_trvAcross(direction);
			} else {
				return PC_trvReverse(PC_trvAcross(direction));
			}
			break;
			
			// make warning go away
		default:
			assert(false);
	}

}

enum trv_dir PC_state_transition_intersect(enum trv_dir indir, enum flag_type_e chosen_flag)
{
	switch (chosen_flag)
	{
		case FLG_EN:
			if (PC_trvIsAcross(indir))
			{
				return DIR_FWD;
			} else {
				return DIR_ACFWD;
			}
			break;
			
		case FLG_EX:
			if (PC_trvIsAcross(indir))
			{
				return DIR_REV;
			} else {
				return DIR_ACFWD;
			}
			break;
		
		case FLG_EN_EX:
			if (PC_trvIsAcross(indir))
				return DIR_ACFWD;
				
			// make warning go away
		default:
			assert(false);
	}
	
}

/**
 * Prepare for phase 3 by closing the polygon rings
 */
void PC_phase_3_prep(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2)
{
	assert(p1); assert(p2);
	assert(!p1->prev); assert(!p2->prev);
	
	// close the loop
	struct PC_vertex_ll * l = __find_last(p1);
	p1->prev = l;
	l->next = p1;
	
	l = __find_last(p2);
	l->next = p2;
	p2->prev = l;
}

/**
 * Phase 3 - find output polygon 
 */
bool PC_phase_3_fp(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2, enum PC_op_t op, struct PC_vertex_ll ** outpoly)
{
	assert(p1); assert(p2); assert(outpoly);
	
	struct PC_vertex_ll * start_vertex = __find_meets_p3_criteria(p1);

    /* If there is no valid start vertex, we've found all polygons */
	if (!start_vertex)
		return false;
	
    /* Start direction is dependent on the operation that's being performed */
    enum trv_dir direction;
	switch (op)
	{
		case PC_op_union:
			direction = PC_startdir_union(start_vertex->flag);
			break;
		case PC_op_intersect:
			direction = PC_startdir_intersect(start_vertex->flag);
			break;
		default:
			assert(false);
	}
	
    /* Save the start direction, we are done transversing when we leave start vertex from the same direction */
	enum trv_dir start_dir = direction;
	
	struct PC_vertex_ll * v = start_vertex;
	struct PC_vertex_ll * newpoly = NULL;
	struct PC_vertex_ll * nextout = NULL;
	struct PC_vertex_ll * last = NULL;
	do {
		// If not across-path, and not internal
		if (!PC_trvIsAcross(direction) && v != last)
			nextout = PC_polyPoint(nextout, v->c.x, v->c.y);
		
		if (!newpoly)
			newpoly = nextout;

		enum trv_dir newdir;
		struct PC_vertex_ll * newvert;
		
		
#ifdef PHASE3_VERBOSE
		printf("Starting vertex %4.2f %4.2f %s [%p] - %s\n", v->c.x, v->c.y, flg_dec(v->flag), v, td_dec(direction));
#endif
		if (!v->intersect)
		{
			newvert = PC_followTransversal(v, direction, direction, FLG_NONE);
			newdir = direction;
		} else {
			enum flag_type_e chosen_flag;
			switch (op)
			{
				case PC_op_union:
					chosen_flag = PC_choose_flag_union(v->flag, direction);
					newdir = PC_state_transition_union(direction,chosen_flag);
					break;
				case PC_op_intersect:
					chosen_flag = PC_choose_flag_intersect(v->flag, direction);
					newdir = PC_state_transition_intersect(direction,chosen_flag);
					break;
			}
			
			newvert = PC_followTransversal(v, direction, newdir, chosen_flag);
            
#ifdef PHASE3_VERBOSE	
			printf("\tChosen flag %s; incoming dir %s, outgoing dir %s\n", flg_dec(chosen_flag), td_dec(direction), td_dec(newdir));	
			printf("\tTraversed to vertex %4.2f %4.2f %s [%p]\n",newvert->c.x, newvert->c.y, flg_dec(newvert->flag),newvert);
#endif
			
		}
		last = v;
		v = newvert;
		direction = newdir;
		
	} while (v != start_vertex || start_dir != direction);
	
	*outpoly = newpoly;
	return true;
}



/* allocate a new vertex, store its location where the vertex_output_ptr points to + advance the
 * vertex output ptr to the ->next field */ 
void PC_newVertex(struct PC_vertex_ll * current_vertex, struct PC_vertex_ll *** vertex_output_ptr)
{
	**vertex_output_ptr = PC_alloc_vertex();
	(**vertex_output_ptr)->c = current_vertex->c;
	*vertex_output_ptr = &((**vertex_output_ptr)->next);
	//printf("Create point %f %f [%p] %d\n", current_vertex->c.x, current_vertex->c.y, current_vertex, current_vertex->done);
}

int PC_polySize(struct PC_vertex_ll * a)
{
	assert(a);
	int c = 1;
	while (a = a->next)
	{
		c++;
	}
	return c;
}

int PC_lineCoincideBits(struct PC_point * a, struct PC_point * b, struct PC_point * c, struct PC_point * d)
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
struct PC_polygon_ll * PC_polygon_boolean(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2, enum PC_op_t op)
{
	if (!PC_phase_one(p1, p2))
		return NULL;

	PC_phase_two(p1, p2, op);
	PC_phase_3_prep(p1, p2);
	
	struct PC_polygon_ll * head = NULL;
	struct PC_polygon_ll  ** cur = &head;
	
	struct PC_vertex_ll * newverticies;
	while (PC_phase_3_fp(p1, p2, op, &newverticies))
	{
		*cur = (struct PC_polygon_ll*)malloc(sizeof(struct PC_polygon_ll));
		(*cur)->next = NULL;
		(*cur)->firstv = newverticies;
		cur = &(*cur)->next;
	}
	
	return head;
}


void PC_free_verticies(struct PC_vertex_ll * polys)
{
	struct PC_vertex_ll * cur_vertex;
	
	// If the loop has been closed, then we need to break it
	if (polys->prev)
		polys->prev->next = NULL;
	
	cur_vertex = polys;

	while (cur_vertex)
	{
		struct PC_vertex_ll * tofree = cur_vertex;
		cur_vertex = cur_vertex->next;
		free(tofree);
	}
	
}


void PC_free_polys(struct PC_polygon_ll * polys)
{
    while (polys)
    {
        struct PC_polygon_ll *tofree = polys;
        PC_free_verticies(polys->firstv);
        polys = polys->next;
        free (tofree);
    }
}

/* Polygon accessor functions */
struct PC_vertex_ll * PC_polyPoint(struct PC_vertex_ll * v, double x, double y)
{
	struct PC_vertex_ll * nv = PC_alloc_vertex();
	nv->c.x = x;
	nv->c.y = y;

	if (v)
		PC_insertAfter(v, nv);
	
	return nv;
}

struct PC_vertex_ll * PC_getPolyPoint(struct PC_vertex_ll *v, double * x, double * y)
{
	assert(v);
	assert(x);
	assert(y);

	*x = v->c.x;
	*y = v->c.y;

	return v->next;
}

