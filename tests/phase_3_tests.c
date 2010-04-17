#include "libtest.h"
#include "polymath.h"
#include "polymath_internal.h"
#include <stddef.h>
#include "stdio.h"
#include "math.h"
#include "support.h"
static void test_overlapping_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 4,4
	struct GH_vertex_ll * r2 = createRect(4,4,4,4);
	
	
	bool result = GH_phase_one(r1, r2); 
	enum GH_op_t op = GH_op_union;
	GH_phase_two(r1, r2, op);
	
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
	
	
	struct GH_vertex_ll * o;
	
	GHKK_phase_3_prep(r1, r2);

	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_ASSERT(polySize(o) == 8);
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 2, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 2, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}

static void test_overlapping_rects_2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 4,4
	struct GH_vertex_ll * r2 = createRectCW(4,4,4,4);
	
	
	bool result = GH_phase_one(r1, r2); 
	enum GH_op_t op = GH_op_union;
	GH_phase_two(r1, r2, op);
	
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
	
	struct GH_vertex_ll * o;
	
	GHKK_phase_3_prep(r1, r2);
	
	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_ASSERT(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 2, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 2, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
	
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}

static void test_adjacent_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct GH_vertex_ll * r2 = createRect(2,6,4,4);
	
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	
	GH_phase_two(r1, r2, op);

	
	GHKK_phase_3_prep(r1, r2);
	struct GH_vertex_ll * o;

	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_ASSERT(polySize(o) == 6);
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 4, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 4, 0));
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}


static void test_adjacent_rects_2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct GH_vertex_ll * r2 = createRectCW(2,6,4,4);
	
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	
	GH_phase_two(r1, r2, op);
	
	
	GHKK_phase_3_prep(r1, r2);
	struct GH_vertex_ll * o;
	
	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_ASSERT(polySize(o) == 6);
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 4, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 4, 0));
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}


static void test_overlapborder_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct GH_vertex_ll * r2 = createRect(3,6,4,4);
	
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	GH_phase_two(r1, r2, op);
	
	GHKK_phase_3_prep(r1, r2);
	struct GH_vertex_ll * o;
	
	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 1, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}


static void test_overlapborder_rects_2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct GH_vertex_ll * r2 = createRectCW(3,6,4,4);
	
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	GH_phase_two(r1, r2, op);
	
	GHKK_phase_3_prep(r1, r2);
	struct GH_vertex_ll * o;
	
	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 1, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}


static void test_onelarger(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct GH_vertex_ll * r2 = createRect(3,3,6,6);
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	
	
	LT_REQUIRE(polySize(r1) == 4); 
	LT_REQUIRE(polySize(r2) == 6);
	
	GH_phase_two(r1, r2, op);
	
	GHKK_phase_3_prep(r1, r2);
	struct GH_vertex_ll * o;
	
	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 6);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 0));
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}

static void test_onelarger2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct GH_vertex_ll * r2 = createRectCW(3,3,6,6);
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	
	
	LT_REQUIRE(polySize(r1) == 4); 
	LT_REQUIRE(polySize(r2) == 6);
	
	GH_phase_two(r1, r2, op);
	
	GHKK_phase_3_prep(r1, r2);
	struct GH_vertex_ll * o;
	
	bool ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 6);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 0));
	
	ok = GHKK_phase_3_fp(r1, r2, &o);
	LT_ASSERT(!ok);
}



void phase_3_tests(void)
{
	_T(test_overlapping_rects_1);
	_T(test_overlapping_rects_2);
	_T(test_adjacent_rects_1);
	_T(test_adjacent_rects_2);
	_T(test_overlapborder_rects_1);
	_T(test_overlapborder_rects_2);
	_T(test_onelarger);
	_T(test_onelarger2);
}