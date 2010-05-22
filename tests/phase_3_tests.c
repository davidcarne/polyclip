#include "libtest.h"
#include "polymath.h"
#include "polymath_internal.h"
#include <stddef.h>
#include "stdio.h"
#include "math.h"
#include "support.h"
static void p3_test_overlapping_rects_1(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 4,4
	struct PC_vertex_ll * r2 = createRect(4,4,4,4);
	
	
	bool result = PC_phase_one(r1, r2); 
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r1, r2, op);
	
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
	
	
	struct PC_vertex_ll * o;
	
	//polyDump(r1);
	//polyDump(r2);
	PC_phase_3_prep(r1, r2);

	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_REQUIRE(polySize(o) == 8);
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 2, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 2, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
	
    
    PC_free_verticies(o);
    
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}

static void p3_test_overlapping_rects_2(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 4,4
	struct PC_vertex_ll * r2 = createRectCW(4,4,4,4);
	
	
	bool result = PC_phase_one(r1, r2); 
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r1, r2, op);
	
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
	
	struct PC_vertex_ll * o;
	
	PC_phase_3_prep(r1, r2);
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_REQUIRE(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 2, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 2, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
	
    PC_free_verticies(o);

	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}

static void p3_test_adjacent_rects_1(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct PC_vertex_ll * r2 = createRect(2,6,4,4);
	
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	PC_phase_two(r1, r2, op);

	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;

	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_REQUIRE(polySize(o) == 6);
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 4, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 4, 0));
    
    PC_free_verticies(o);

	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}


static void p3_test_adjacent_rects_2(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 2,4
	struct PC_vertex_ll * r2 = createRectCW(2,6,4,4);
	
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	PC_phase_two(r1, r2, op);
	
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	//polyDump(o);
	LT_REQUIRE(polySize(o) == 6);
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 4, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 4, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}


static void p3_test_overlapborder_rects_1(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct PC_vertex_ll * r2 = createRect(3,6,4,4);
	
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	PC_phase_two(r1, r2, op);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	LT_REQUIRE(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 1, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}


static void p3_test_overlapborder_rects_2(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct PC_vertex_ll * r2 = createRectCW(3,6,4,4);
	
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	PC_phase_two(r1, r2, op);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	LT_REQUIRE(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 1, 8));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}


static void p3_test_onelarger(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct PC_vertex_ll * r2 = createRect(3,3,6,6);
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	
	LT_REQUIRE(polySize(r1) == 4); 
	LT_REQUIRE(polySize(r2) == 6);
	
	PC_phase_two(r1, r2, op);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 6);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}

static void p3_test_onelarger2(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct PC_vertex_ll * r2 = createRectCW(3,3,6,6);
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	
	LT_REQUIRE(polySize(r1) == 4); 
	LT_REQUIRE(polySize(r2) == 6);
	
	PC_phase_two(r1, r2, op);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 6);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 6, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 6, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 6));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}

static void p3_test_semi1(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct PC_vertex_ll * r2 = createRect(4,3,2,2);
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	LT_REQUIRE(polySize(r1) == 6); 
	LT_REQUIRE(polySize(r2) == 6);
	
	PC_phase_two(r1, r2, op);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}

static void p3_test_semi2(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRect(2,2,4,4);
	
	// Square centered on 3,4
	struct PC_vertex_ll * r2 = createRectCW(4,3,2,2);
	
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	LT_REQUIRE(polySize(r1) == 6); 
	LT_REQUIRE(polySize(r2) == 6);
	
	PC_phase_two(r1, r2, op);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 8);
	
	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 4, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}

static void p3_test_semi3(void)
{
	struct PC_vertex_ll * r1 = createRectCW(2,2,4,4);
	struct PC_vertex_ll * r2 = createRect(4,3,2,2);
	
	
	enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	LT_REQUIRE(polySize(r1) == 6); 
	LT_REQUIRE(polySize(r2) == 6);
	
	PC_phase_two(r1, r2, op);
	
	
	//polyDump(r1);
	//polyDump(r2);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	
	//polyDump(o);
	
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 8);
	
	// We start with 4, 4 since only the second
	// element of an en, en couple is valid to start with
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 3, 4));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
    
}

static void p3_test_semi4(void)
{
	// Square centered on 2,2
	struct PC_vertex_ll * r1 = createRectCW(2,2,4,4);
	
	// Square centered on 3,4
	struct PC_vertex_ll * r2 = createRectCW(4,3,2,2);
		enum PC_op_t op = PC_op_union;
	bool result = PC_phase_one(r1, r2); 
	
	LT_REQUIRE(polySize(r1) == 6); 
	LT_REQUIRE(polySize(r2) == 6);
	
	PC_phase_two(r1, r2, op);
	
	PC_phase_3_prep(r1, r2);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(ok);
	LT_ASSERT(polySize(o) == 8);
	
	
	// We start with 4, 4 since only the second
	// element of an en, en couple is valid to start with
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 4, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 5, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 5, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 4, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 4, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 3, 4));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r1, r2, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r1);
    PC_free_verticies(r2);
}

static void p3_tri_inside(void)
{
	struct PC_vertex_ll * tri = NULL, * p;
	tri = PC_polyPoint(NULL, 1, 2);
	p = PC_polyPoint(tri,  3, 2);
	p = PC_polyPoint(p,    2, 4);
	
	struct PC_vertex_ll * r = createRect(2,2,4,4);
	
	bool result = PC_phase_one(r, tri); 
	
	LT_REQUIRE(polySize(r) == 5); 
	LT_REQUIRE(polySize(tri) == 3);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, tri, op);
	
	PC_phase_3_prep(r, tri);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r, tri, PC_op_union, &o);
	LT_ASSERT(ok);
	
	//polyDump(o);
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r, tri, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(r);
    PC_free_verticies(tri);
}



/*
 * shape_test_1_[1-4] are unioning a pointy shape with a rectangle
 *     [starting on the pointy, transitioning to the rect so it hits an ex-en]
 */
static void shape_test_1_1(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,2, 0);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    0, 3);
	
	struct PC_vertex_ll * r = createRect(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	//polyDump(r);
	//polyDump(pokey);
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	
	//polyDump(o);
	
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 3, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 2, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

static void shape_test_1_2(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,2, 0);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    0, 3);
	
	struct PC_vertex_ll * r = createRectCW(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 3, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 0, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 2, 0));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

static void shape_test_1_3(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,0, 3);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    2, 0);
	
	struct PC_vertex_ll * r = createRect(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 3, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 0, 3));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

static void shape_test_1_4(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,0, 3);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    2, 0);
	
	struct PC_vertex_ll * r = createRectCW(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 3, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 0, 3));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(pokey, r, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

/*
 * shape_test_1_[5-8] are unioning a pointy shape with a rectangle
 *     [starting on the rect, transitioning to the pointy so it hits an ex-en]
 */
static void shape_test_1_5(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,2, 0);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    0, 3);
	
	struct PC_vertex_ll * r = createRect(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	
	//polyDump(r);
	//polyDump(pokey);
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 3, 1));
    
    PC_free_verticies(o);
	//polyDump(o);
	
	ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

static void shape_test_1_6(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,2, 0);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    0, 3);
	
	struct PC_vertex_ll * r = createRectCW(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 0, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 3, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 1, 4));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

static void shape_test_1_7(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,0, 3);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    2, 0);
	
	struct PC_vertex_ll * r = createRect(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);

	
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 0, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 1, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 3, 1));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

static void shape_test_1_8(void)
{
	
	struct PC_vertex_ll * pokey = NULL, * p;
	pokey = PC_polyPoint(NULL, 0, 0);
	p = PC_polyPoint(pokey,0, 3);
	p = PC_polyPoint(p,    2, 3);
	p = PC_polyPoint(p,    3, 2);
	p = PC_polyPoint(p,    2, 2);
	p = PC_polyPoint(p,    2, 0);
	
	struct PC_vertex_ll * r = createRectCW(2,2.5,2,3);
	
	bool result = PC_phase_one(r, pokey); 
	
	
	LT_REQUIRE(polySize(r) == 7); 
	LT_REQUIRE(polySize(pokey) == 8);
	
	enum PC_op_t op = PC_op_union;
	PC_phase_two(r, pokey, op);
	
	PC_phase_3_prep(r, pokey);
	struct PC_vertex_ll * o;
	
	bool ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(ok);
	
	LT_REQUIRE(polySize(o) == 9);
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 0), 1, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 1), 0, 3));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 2), 0, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 3), 2, 0));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 4), 2, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 5), 3, 1));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 6), 3, 2));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 7), 3, 4));
	LT_ASSERT(VERTEX_COMPARE_TO(_I(o, 8), 1, 4));
    
    PC_free_verticies(o);
	
	ok = PC_phase_3_fp(r, pokey, PC_op_union, &o);
	LT_ASSERT(!ok);
    
    PC_free_verticies(pokey);
    PC_free_verticies(r);
}

void phase_3_tests(void)
{
	_T(p3_test_overlapping_rects_1);
	_T(p3_test_overlapping_rects_2);
	_T(p3_test_adjacent_rects_1);
	_T(p3_test_adjacent_rects_2);
	_T(p3_test_overlapborder_rects_1);
	_T(p3_test_overlapborder_rects_2);
	_T(p3_test_onelarger);
	_T(p3_test_onelarger2);
	_T(p3_test_semi1);
	_T(p3_test_semi2);
	_T(p3_test_semi3);
	_T(p3_test_semi4);
	
	_T(shape_test_1_1);
	_T(shape_test_1_2);
	_T(shape_test_1_3);
	_T(shape_test_1_4);
	
	_T(shape_test_1_5);
	_T(shape_test_1_6);
	_T(shape_test_1_7);
	_T(shape_test_1_8);
}