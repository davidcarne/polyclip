#include "libtest.h"
#include "polymath.h"
#include "polymath_internal.h"
#include "math.h"
#include "support.h"
#include <stdio.h>

static void testNoCrossingLine(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {16,15}, d = {30, 29};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}

static void testCloseLines(void)
{
	
	struct GH_point a = {10,10}, b = {20,20}, c = {10.001, 10}, d = {20.001, 20};
	double alphaP, alphaQ;
	
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}

static void testCoaxialOverlappingLines(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {5, 5}, d = {15, 15};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_COINCIDE);
}

static void testIdenticalLines(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {10, 10}, d = {20, 20};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_COINCIDE);
}


static void testTouch1(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {5, 10}, d = {20, 20};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_TOUCH);
	LT_ASSERT(alphaP == 1.0);
	LT_ASSERT(alphaQ == 1.0);
}

static void testTouch2(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {5, 10}, d = {15, 15};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_TOUCH);
	LT_ASSERT(alphaP == 0.5);
	LT_ASSERT(alphaQ == 1.0);
}

static void testTouch3(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {15, 15}, d = {5, 10};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}

static void testTouch4(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {15, 15}, d = {5, 10};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}


static void testCoincide1(void)
{
	struct GH_point a = {4,4}, b = {0,4}, c = {1, 4}, d = {5, 4};
	double alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	int bits = GH_lineCoincideBits(&a, &b, &c, &d);
	
	LT_ASSERT(inter_type == INTER_COINCIDE);
	LT_ASSERT(bits & A_ONCD);
	LT_ASSERT(bits & C_ONAB);
}

static void testPointInPolygonBug1(void)
{
	struct GH_vertex_ll * r1 = createRect(4,3,2,2);
	struct GH_point p1 = {1.5, 4};
	LT_ASSERT(!GH_pointInPoly(r1, &p1));

}


static void testPointInPolygonBug2(void)
{
	struct GH_vertex_ll * r = NULL, * p;
	r = GH_polyPoint(NULL, 3, 2);
	p = GH_polyPoint(r,    5, 2);
	p = GH_polyPoint(p,    5, 4);
	p = GH_polyPoint(p,    4, 4);
	p = GH_polyPoint(p,    3, 4);
	
	struct GH_point p1 = {1.5, 4};
	LT_ASSERT(!GH_pointInPoly(r, &p1));
	
}

static void testPointInPolygonBug3(void)
{
	struct GH_vertex_ll * r = NULL, * p;
	r = GH_polyPoint(NULL, 1, 2);
	p = GH_polyPoint(r,    2, 3);
	p = GH_polyPoint(p,    3, 2);
	p = GH_polyPoint(p,    2, 1);
	
	struct GH_point p1 = {2, 2};
	LT_ASSERT(GH_pointInPoly(r, &p1));
	
}

static void testPointInPolygonBug4(void)
{
	struct GH_vertex_ll * r = NULL, * p;
	r = GH_polyPoint(NULL, 1, 2);
	p = GH_polyPoint(r,    2, 3);
	p = GH_polyPoint(p,    3, 2);
	p = GH_polyPoint(p,    2, 1);
	
	
	struct GH_point p1 = {0, 2};
	LT_ASSERT(!GH_pointInPoly(r, &p1));
	
}

static void testPointInPolygonBug5(void)
{
	struct GH_vertex_ll * r = NULL, * p;
	r = GH_polyPoint(NULL, 6, 4);
	p = GH_polyPoint(r,    8, 4);
	p = GH_polyPoint(p,    4, 0);
	p = GH_polyPoint(p,    4, 4);
	
	
	struct GH_point p1 = {3, 4};
	LT_ASSERT(!GH_pointInPoly(r, &p1));
	
}

static void testPointInPolygonBug6(void)
{
	struct GH_vertex_ll * r = NULL, * p;
	r = GH_polyPoint(NULL, 1, 2);
	p = GH_polyPoint(r,    3, 2);
	p = GH_polyPoint(p,    2, 4);
	
	
	struct GH_point p1 = {0, 4};
	LT_ASSERT(!GH_pointInPoly(r, &p1));
	
}

static void testPointInPolygonBug7(void)
{
	struct GH_vertex_ll * pokey = NULL, * p;
	pokey = GH_polyPoint(NULL, 0, 0);
	p = GH_polyPoint(pokey,2, 0);
	p = GH_polyPoint(p,    2, 2);
	p = GH_polyPoint(p,    3, 2);
	p = GH_polyPoint(p,    2, 3);
	p = GH_polyPoint(p,    0, 3);
	
	
	struct GH_point p1 = {1, 2};
	LT_ASSERT(GH_pointInPoly(pokey, &p1));
	
}

static void test_same_way_1(void)
{
    struct GH_vertex_ll a1, a2, b1, b2, i;
    a1.c.x = 3; a1.c.y = 4;
    a2.c.x = 2; a2.c.y = -5;
    
    b1.c.x = 6; b1.c.y = 7;
    b2.c.x = 8; b1.c.y = 0;
    i .c.x = 4; i .c.y = 2;
    
    
    LT_ASSERT(GH_intersection_same_way(&a1, &a2, &b1, &b2, &i));
}

static void test_same_way_2(void)
{
    struct GH_vertex_ll a1, a2, b1, b2, i;
    a1.c.x = 3; a1.c.y = 4;
    a2.c.x = 2; a2.c.y = -5;
    
    b1.c.x = 6; b1.c.y = 0;
    b2.c.x = 8; b1.c.y = 7;
    i .c.x = 4; i .c.y = 2;
    
    
    LT_ASSERT(!GH_intersection_same_way(&a1, &a2, &b1, &b2, &i));
    
}

void internal_ops_tests(void)
{
	
	_T(testNoCrossingLine);
	_T(testCloseLines);
	_T(testCoaxialOverlappingLines);
	_T(testIdenticalLines);
	_T(testTouch1);
	_T(testTouch2);
	_T(testTouch3);
	_T(testCoincide1);
	_T(testPointInPolygonBug1);
	_T(testPointInPolygonBug2);
	_T(testPointInPolygonBug3);
	_T(testPointInPolygonBug4);
	_T(testPointInPolygonBug5);
	_T(testPointInPolygonBug6);
	_T(testPointInPolygonBug7);
    _T(test_same_way_1);
    _T(test_same_way_2);
}