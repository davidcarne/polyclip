#include "libtest.h"
#include "polymath.h"
#include "polymath_internal.h"
#include <stddef.h>
#include "stdio.h"
#include "math.h"
#include "support.h"

static bool no_couples(struct GH_vertex_ll * a)
{
	do {
		if (a->couple)
			return 0;
	} while (a = a->next);
	return 1;
}
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
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 4)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 5)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 5)->flag == FLG_EN);
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
}


static void test_overlapping_rects_2(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 4,4
	struct GH_vertex_ll * r2 = createRectCW(4,4,4,4);
	
	enum GH_op_t op = GH_op_union;
	
	bool result = GH_phase_one(r1, r2); 
	GH_phase_two(r1, r2, op);
	
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
	
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 4)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 5)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 5)->flag == FLG_EN);
	
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
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
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_EN);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
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
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_EN);
	
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
}

static void test_touching_rects_1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 6,6
	struct GH_vertex_ll * r2 = createRect(6,6,4,4);
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	GH_phase_two(r1, r2, op);
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN_EX);
	LT_ASSERT(_I(r1, 3)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_EN_EX);
	LT_ASSERT(_I(r2, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));

}

static void test_touching_rects_2(void)
{
	
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct GH_vertex_ll * r2 = createRectCW(6,6,4,4);
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	GH_phase_two(r1, r2, op);
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN_EX);
	LT_ASSERT(_I(r1, 3)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_EN_EX);
	LT_ASSERT(_I(r2, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));

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
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 4)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_EN);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_NONE);
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
	
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
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 4)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_EN);
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
}


static void test_onelarger(void)
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
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 5)->flag == FLG_EN);
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
}

static void test_onelarger2(void)
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
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 5)->flag == FLG_EN);
	
	LT_ASSERT(no_couples(r1));
	LT_ASSERT(no_couples(r2));
}

static void test_semi1(void)
{
	// Square centered on 2,2
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct GH_vertex_ll * r2 = createRect(4,3,2,2);
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	
	LT_REQUIRE(polySize(r1) == 6); 
	LT_REQUIRE(polySize(r2) == 6);
	
	GH_phase_two(r1, r2, op);
	
	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 4)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 5)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_EN);
	LT_ASSERT(_I(r2, 5)->flag == FLG_EN);
	
	
	// Test couple creation
	LT_ASSERT(_I(r1, 0)->couple == NULL);
	LT_ASSERT(_I(r1, 1)->couple == NULL);
	LT_ASSERT(_I(r1, 2)->couple == NULL);
	LT_ASSERT(_I(r1, 3)->couple == _I(r1, 4));
	LT_ASSERT(_I(r1, 4)->couple == _I(r1, 3));
	LT_ASSERT(_I(r1, 5)->couple == NULL);
	
	LT_ASSERT(_I(r2, 0)->couple == NULL);
	LT_ASSERT(_I(r2, 1)->couple == NULL);
	LT_ASSERT(_I(r2, 2)->couple == NULL);
	LT_ASSERT(_I(r2, 3)->couple == NULL);
	LT_ASSERT(_I(r2, 4)->couple == _I(r2, 5));
	LT_ASSERT(_I(r2, 5)->couple == _I(r2, 4));
}

static void test_semi2(void)
{
	struct GH_vertex_ll * r1 = createRect(2,2,4,4);
	struct GH_vertex_ll * r2 = createRectCW(4,3,2,2);
	
	enum GH_op_t op = GH_op_union;
	bool result = GH_phase_one(r1, r2); 
	
	
	
	LT_REQUIRE(polySize(r1) == 6); 
	LT_REQUIRE(polySize(r2) == 6);
	
	GH_phase_two(r1, r2, op);
	

	LT_ASSERT(_I(r1, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 1)->flag == FLG_NONE);
	LT_ASSERT(_I(r1, 2)->flag == FLG_EN);
	LT_ASSERT(_I(r1, 3)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 4)->flag == FLG_EX);
	LT_ASSERT(_I(r1, 5)->flag == FLG_NONE);
	
	LT_ASSERT(_I(r2, 0)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 1)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 2)->flag == FLG_EX);
	LT_ASSERT(_I(r2, 3)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 4)->flag == FLG_NONE);
	LT_ASSERT(_I(r2, 5)->flag == FLG_EN);
	
	
	// Test couple creation
	LT_ASSERT(_I(r1, 0)->couple == NULL);
	LT_ASSERT(_I(r1, 1)->couple == NULL);
	LT_ASSERT(_I(r1, 2)->couple == NULL);
	LT_ASSERT(_I(r1, 3)->couple == _I(r1, 4));
	LT_ASSERT(_I(r1, 4)->couple == _I(r1, 3));
	LT_ASSERT(_I(r1, 5)->couple == NULL);
	
	LT_ASSERT(_I(r2, 0)->couple == NULL);
	LT_ASSERT(_I(r2, 1)->couple == _I(r2, 2));
	LT_ASSERT(_I(r2, 2)->couple == _I(r2, 1));
	LT_ASSERT(_I(r2, 3)->couple == NULL);
	LT_ASSERT(_I(r2, 4)->couple == NULL);
	LT_ASSERT(_I(r2, 5)->couple == NULL);
}

void phase_2_tests(void)
{
	_T(test_overlapping_rects_1);
	_T(test_overlapping_rects_2);
	_T(test_adjacent_rects_1);
	_T(test_adjacent_rects_2);
	_T(test_touching_rects_1);
	_T(test_touching_rects_2);
	_T(test_overlapborder_rects_1);
	_T(test_overlapborder_rects_2);
	_T(test_onelarger);
	_T(test_onelarger2);
	_T(test_semi1);
	_T(test_semi2);
}