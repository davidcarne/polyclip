#include "libtest.h"
#include "polymath.h"
#include "polymath_internal.h"
#include <stddef.h>
#include "stdio.h"
#include "math.h"
#include "support.h"

static void p1_test_overlapping_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 4,4
	struct GH_vertex_ll * r2 = createRect(4,4,4,4);
	
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	LT_REQUIRE(polySize(r1) == 6);
	LT_REQUIRE(polySize(r2) == 6);
	
	// Check intersections as expected
	LT_REQUIRE(_I(r1,2));
	LT_ASSERT (_I(r1,2)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,2), 4,2));
	
	LT_REQUIRE(_I(r1,4));
	LT_ASSERT (_I(r1,4)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,4), 2,4));
	
	LT_REQUIRE(_I(r2,1));
	LT_ASSERT (_I(r2,1)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,1), 4,2));
	
	LT_REQUIRE(_I(r2,5));
	LT_ASSERT (_I(r2,5)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,5), 2,4));
	
	// Check neighbours
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,1));
	LT_ASSERT (_I(r2,1)->neighbor == _I(r1,2));
	LT_ASSERT (_I(r1,4)->neighbor == _I(r2,5));
	LT_ASSERT (_I(r2,5)->neighbor == _I(r1,4));
	
	// Should be:
	/*
	 *
	 *
	 *           R2_4-------------------R2_3
	 *             |                      |
	 *             |                      |
	 *    R1_5--R1_4/R2_5--R1_3           |
	 *     |       |         |            |
	 *     |       |         |            |
	 *     |     R2_0------R1_2/R2_1----R2_2
	 *     |                 |
	 *     |                 |
	 *    R1_0-------------R1_1
	 *
	 */
}


static void p1_test_overlapping_rects_2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 4,4
	struct GH_vertex_ll * r2 = createRectCW(4,4,4,4);
	
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	LT_REQUIRE(polySize(r1) == 6);
	LT_REQUIRE(polySize(r2) == 6);
	
	// Check intersections as expected
	LT_REQUIRE(_I(r1,2));
	LT_ASSERT (_I(r1,2)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,2), 4,2));
	
	LT_REQUIRE(_I(r1,4));
	LT_ASSERT (_I(r1,4)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,4), 2,4));
	
	LT_REQUIRE(_I(r2,1));
	LT_ASSERT (_I(r2,1)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,1), 2,4));
	
	LT_REQUIRE(_I(r2,5));
	LT_ASSERT (_I(r2,5)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,5), 4,2));
	
	// Check neighbours
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,5));
	LT_ASSERT (_I(r2,5)->neighbor == _I(r1,2));
	LT_ASSERT (_I(r1,4)->neighbor == _I(r2,1));
	LT_ASSERT (_I(r2,1)->neighbor == _I(r1,4));
	
	// Should be:
	/*
	 *
	 *
	 *           R2_2-------------------R2_3
	 *             |                      |
	 *             |                      |
	 *    R1_5--R1_4/R2_1--R1_3           |
	 *     |       |         |            |
	 *     |       |         |            |
	 *     |     R2_0------R1_2/R2_5----R2_4
	 *     |                 |
	 *     |                 |
	 *    R1_0-------------R1_1
	 *
	 */
}

static void p1_test_adjacent_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct GH_vertex_ll * r2 = createRect(2,6,4,4);
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	LT_REQUIRE(polySize(r1) == 4);
	LT_REQUIRE(polySize(r2) == 4);
	
	// Check intersections as expected
	LT_REQUIRE(_I(r1,2));
	LT_ASSERT (_I(r1,2)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,2), 4,4));
	LT_REQUIRE(_I(r1,3));
	LT_ASSERT (_I(r1,3)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,3), 0,4));
	LT_REQUIRE(_I(r2,0));
	LT_ASSERT (_I(r2,0)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,0), 0,4));
	LT_REQUIRE(_I(r2,1));
	LT_ASSERT (_I(r2,1)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,1), 4,4));
	
	// Check neighbours
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,1));
	LT_ASSERT (_I(r2,1)->neighbor == _I(r1,2));
	
	LT_ASSERT (_I(r1,3)->neighbor == _I(r2,0));
	LT_ASSERT (_I(r2,0)->neighbor == _I(r1,3));
}

static void p1_test_adjacent_rects_2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct GH_vertex_ll * r2 = createRectCW(2,6,4,4);
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	LT_REQUIRE(polySize(r1) == 4);
	LT_REQUIRE(polySize(r2) == 4);
	
	// Check intersections as expected
	LT_REQUIRE(_I(r1,2));
	LT_ASSERT (_I(r1,2)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,2), 4,4));
	LT_REQUIRE(_I(r1,3));
	LT_ASSERT (_I(r1,3)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,3), 0,4));
	LT_REQUIRE(_I(r2,0));
	LT_ASSERT (_I(r2,0)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,0), 0,4));
	LT_REQUIRE(_I(r2,3));
	LT_ASSERT (_I(r2,3)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,3), 4,4));
	
	// Check neighbours
	LT_ASSERT (_I(r1,3)->neighbor == _I(r2,0));
	LT_ASSERT (_I(r2,0)->neighbor == _I(r1,3));
	
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,3));
	LT_ASSERT (_I(r2,3)->neighbor == _I(r1,2));
}

static void p1_test_overlapborder_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct GH_vertex_ll * r2 = createRect(3,6,4,4);
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	
	LT_REQUIRE(polySize(r1) == 5);
	LT_REQUIRE(polySize(r2) == 5);
	
	// Check intersections as expected
	LT_REQUIRE(_I(r1,2));
	LT_ASSERT (_I(r1,2)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,2), 4,4));
	LT_REQUIRE(_I(r1,3));
	LT_ASSERT (_I(r1,3)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,3), 1,4));
	
	LT_REQUIRE(_I(r2,0));
	LT_ASSERT (_I(r2,0)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,0), 1,4));
	LT_REQUIRE(_I(r2,1));
	LT_ASSERT (_I(r2,1)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,1), 4,4));
	
	// Check neighbours
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,1));
	LT_ASSERT (_I(r2,1)->neighbor == _I(r1,2));
	
	LT_ASSERT (_I(r1,3)->neighbor == _I(r2,0));
	LT_ASSERT (_I(r2,0)->neighbor == _I(r1,3));
}

static void p1_test_overlapborder_rects_2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct GH_vertex_ll * r2 = createRectCW(3,6,4,4);
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	LT_REQUIRE(polySize(r1) == 5);
	LT_REQUIRE(polySize(r2) == 5);
	
	// Check intersections as expected
	LT_REQUIRE(_I(r1,2));
	LT_ASSERT (_I(r1,2)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,2), 4,4));
	LT_REQUIRE(_I(r1,3));
	LT_ASSERT (_I(r1,3)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r1,3), 1,4));
	
	LT_REQUIRE(_I(r2,0));
	LT_ASSERT (_I(r2,0)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,0), 1,4));
	LT_REQUIRE(_I(r2,4));
	LT_ASSERT (_I(r2,4)->intersect);
	LT_ASSERT (VERTEX_COMPARE_TO(_I(r2,4), 4,4));
	
	// Check neighbours
	LT_ASSERT (_I(r1,3)->neighbor == _I(r2,0));
	LT_ASSERT (_I(r2,0)->neighbor == _I(r1,3));
	
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,4));
	LT_ASSERT (_I(r2,4)->neighbor == _I(r1,2));
}

static void p1_test_touching_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct GH_vertex_ll * r2 = createRect(6,6,4,4);
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	LT_REQUIRE(polySize(r1) == 4);
	LT_REQUIRE(polySize(r2) == 4);
	
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,0));
	LT_ASSERT (_I(r2,0)->neighbor == _I(r1,2));
}

static void p1_test_touching_rects_2(void)
{
	
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct GH_vertex_ll * r2 = createRectCW(6,6,4,4);
	
	bool result = GH_phase_one(r1, r2); 
	LT_REQUIRE(result);
	
	LT_REQUIRE(polySize(r1) == 4);
	LT_REQUIRE(polySize(r2) == 4);
	
	LT_ASSERT (_I(r1,2)->neighbor == _I(r2,0));
	LT_ASSERT (_I(r2,0)->neighbor == _I(r1,2));
}


void phase_1_tests(void)
{
	_T(p1_test_overlapping_rects_1);
	_T(p1_test_overlapping_rects_2);
	_T(p1_test_adjacent_rects_1);
	_T(p1_test_adjacent_rects_2);
	_T(p1_test_touching_rects_1);
	_T(p1_test_touching_rects_2);
	_T(p1_test_touching_rects_1);
	_T(p1_test_overlapborder_rects_1);
	_T(p1_test_overlapborder_rects_2);

}