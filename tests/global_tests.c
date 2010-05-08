#include "libtest.h"
#include "polymath.h"
#include "polymath_internal.h"
#include <stddef.h>
#include "stdio.h"
#include "math.h"
#include "support.h"

static int nPolys(struct GH_polygon_ll * p)
{
	int c = 1;
	while (p = p->next) c++;
	return c;
}

struct GH_vertex_ll * reversePoly(struct GH_vertex_ll * v)
{
	struct GH_vertex_ll * next = v;
	struct GH_vertex_ll * cur;
	while (next)
	{
		cur = next;
		next = next->next;
		
		struct GH_vertex_ll * temp;
		temp = cur->next;
		cur->next = cur->prev;
		cur->prev = temp;
	}
	return cur;
}

static void testpattern_1(struct GH_vertex_ll ** f1, struct GH_vertex_ll ** f2)
{
	/* 8  --   --
	 * 7  - - - -
	 * 6  - *+**+*
	 * 5  -*    -*
 	 * 4  +      +
	 * 3  -*    -*
	 * 2  - ****+*
	 * 1  -     -
	 * 0  -------
	 *    
	 *    01234567
	 * -'s = f1
	 * *'s = f2
	 */
	
	struct GH_vertex_ll * p;
	*f1 = GH_polyPoint(NULL, 0, 0);
	p = GH_polyPoint(*f1,    0, 8);
	p = GH_polyPoint(p,     3, 6);
	p = GH_polyPoint(p,     6, 8);
	p = GH_polyPoint(p,     6, 5);
	p = GH_polyPoint(p,     7, 4);
	p = GH_polyPoint(p,     6, 3);
	p = GH_polyPoint(p,     6, 0);
	
	*f2 = GH_polyPoint(NULL, 0, 4);
	p = GH_polyPoint(*f2,    2, 6);
	p = GH_polyPoint(p,     7, 6);
	p = GH_polyPoint(p,     7, 2);
	p = GH_polyPoint(p,     2, 2);
}

static void test_simple_union_1()
{
	struct GH_vertex_ll * f1, * f2;
	testpattern_1(&f1, &f2);
	
	struct GH_polygon_ll * res = GH_polygon_boolean(f1, f2, GH_op_union);
	LT_REQUIRE(res);
	
	LT_REQUIRE(nPolys(res) == 1);
	
	struct GH_vertex_ll * rpoly = res->firstv;
	LT_REQUIRE(polySize(rpoly) == 11);
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  0), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  1), 0, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  2), 3, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  3), 6, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  4), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  5), 7, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  6), 7, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  7), 7, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  8), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  9), 6, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly, 10), 0, 0));
}

static void test_simple_intersect_1()
{
	struct GH_vertex_ll * f1, * f2;
	testpattern_1(&f1, &f2);
	
	struct GH_polygon_ll * res = GH_polygon_boolean(f1, f2, GH_op_intersect);
	LT_REQUIRE(res);
	LT_REQUIRE(nPolys(res) == 1);
	
	struct GH_vertex_ll * rpoly = res->firstv;
	LT_REQUIRE(polySize(rpoly) == 9);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  0), 2, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  1), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  2), 6, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  3), 7, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  4), 6, 5));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  5), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  6), 3, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  7), 2, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  8), 0, 4));
}


static void test_simple_intersect_2()
{
	struct GH_vertex_ll * f1, * f2;
	testpattern_1(&f1, &f2);
	f2 = reversePoly(f2);
	
	//polyDump(f2);
	
	struct GH_polygon_ll * res = GH_polygon_boolean(f1, f2, GH_op_intersect);
	LT_REQUIRE(res);
	LT_REQUIRE(nPolys(res) == 1);
	
	struct GH_vertex_ll * rpoly = res->firstv;
	LT_REQUIRE(polySize(rpoly) == 9);
	
	
    //polyDump(rpoly);
    
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  0), 2, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  1), 3, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  3), 6, 5));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  4), 7, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  5), 6, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  6), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  7), 2, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  8), 0, 4));
}

static void test_winding_bug_1()
{
	
	struct GH_vertex_ll * f1, * f2;
	testpattern_1(&f1, &f2);
	f2 = reversePoly(f2);
	
	
	bool result = GH_phase_one(f1, f2); 

	enum GH_op_t op = GH_op_intersect;
	GH_phase_two(f1, f2, op);
	
    //polyDump(f1);
	//polyDump(f2);
}

/*
 *  Hard testcase such as the one found in the Kim-Kim paper.
 */
static void setup_KK_example(struct GH_vertex_ll ** c, struct GH_vertex_ll ** s)
{
    struct GH_vertex_ll * p;
	*c = GH_polyPoint(NULL, 0, 0);
	p = GH_polyPoint(*c,    6, 0);
	p = GH_polyPoint(p,      6, 7);
	p = GH_polyPoint(p,      0, 4);
	
	*s = GH_polyPoint(NULL,   2, 0);
	p = GH_polyPoint(*s,      2, 1);
	p = GH_polyPoint(p,      0, 2);
	p = GH_polyPoint(p,      2, 3);
	p = GH_polyPoint(p,      2, 5);
    p = GH_polyPoint(p,      4, 6);
	p = GH_polyPoint(p,      2, 7);
	p = GH_polyPoint(p,      6, 7);
	p = GH_polyPoint(p,      9, 2);
	p = GH_polyPoint(p,      5, 2);
	p = GH_polyPoint(p,      5, -2);
	p = GH_polyPoint(p,      4, 0);
}

static void test_KK_example()
{
    struct GH_vertex_ll * c; // clip polygon
    struct GH_vertex_ll * s; // subject polygon
    

    setup_KK_example(&c, &s);
    
	struct GH_polygon_ll * res = GH_polygon_boolean(s, c, GH_op_intersect);
	LT_REQUIRE(res);
	LT_REQUIRE(nPolys(res) == 1);
	
	struct GH_vertex_ll * rpoly = res->firstv;
	LT_REQUIRE(polySize(rpoly) == 11);
	
    
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  0), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  1), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  2), 5, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  3), 5, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  4), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  5), 6, 7));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  6), 4, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  7), 2, 5));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  8), 2, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  9), 0, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly, 10), 2, 1));
}

static void test_KK_example_R2()
{
    
    struct GH_vertex_ll * c; // clip polygon
    struct GH_vertex_ll * s; // subject polygon
    
    setup_KK_example(&c, &s);
    s = reversePoly(s);
    
	struct GH_polygon_ll * res = GH_polygon_boolean(s, c, GH_op_intersect);
	LT_REQUIRE(res);
	LT_REQUIRE(nPolys(res) == 1);
	
	struct GH_vertex_ll * rpoly = res->firstv;
	LT_REQUIRE(polySize(rpoly) == 11);
	    
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  0), 5, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  1), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  2), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  3), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  4), 0, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  5), 2, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  6), 2, 5));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  7), 4, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  8), 6, 7));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  9), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  10), 5, 2));
}

static void test_KK_example_R3()
{
    
    struct GH_vertex_ll * c; // clip polygon
    struct GH_vertex_ll * s; // subject polygon
    
    setup_KK_example(&c, &s);
    c = reversePoly(c);
    
	struct GH_polygon_ll * res = GH_polygon_boolean(s, c, GH_op_intersect);
	LT_REQUIRE(res);
	LT_REQUIRE(nPolys(res) == 1);
	
	struct GH_vertex_ll * rpoly = res->firstv;
	LT_REQUIRE(polySize(rpoly) == 11);
    
    
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  0), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  1), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  2), 5, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  3), 5, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  4), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  5), 6, 7));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  6), 4, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  7), 2, 5));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  8), 2, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  9), 0, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly, 10), 2, 1));
}

static void test_KK_example_R4()
{
    
    
    struct GH_vertex_ll * c; // clip polygon
    struct GH_vertex_ll * s; // subject polygon
    
    setup_KK_example(&c, &s);
    s = reversePoly(s);
    c = reversePoly(c);
    
	struct GH_polygon_ll * res = GH_polygon_boolean(s, c, GH_op_intersect);
	LT_REQUIRE(res);
	LT_REQUIRE(nPolys(res) == 1);
	
	struct GH_vertex_ll * rpoly = res->firstv;
	LT_REQUIRE(polySize(rpoly) == 11);
    
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  0), 5, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  1), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  2), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  3), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  4), 0, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  5), 2, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  6), 2, 5));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  7), 4, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  8), 6, 7));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  9), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(rpoly,  10), 5, 2));
}

void global_tests()
{
	_T(test_winding_bug_1);
	_T(test_simple_union_1);
	_T(test_simple_intersect_1);
	_T(test_simple_intersect_2);
	_T(test_KK_example);
	_T(test_KK_example_R2);
	_T(test_KK_example_R3);
	_T(test_KK_example_R4);
}