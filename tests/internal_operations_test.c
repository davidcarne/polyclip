#include "libtest.h"
#include "polymath.h"
#include "polymath_internal.h"
#include "math.h"
void testNoCrossingLine(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {16,15}, d = {30, 29};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}

void testCloseLines(void)
{
	
	struct GH_point a = {10,10}, b = {20,20}, c = {10.001, 10}, d = {20.001, 20};
	float alphaP, alphaQ;
	
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}

void testCoaxialOverlappingLines(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {5, 5}, d = {15, 15};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_COINCIDE);
}

void testIdenticalLines(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {10, 10}, d = {20, 20};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_COINCIDE);
}


void testTouch1(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {5, 10}, d = {20, 20};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_TOUCH);
	LT_ASSERT(alphaP == 1.0);
	LT_ASSERT(alphaQ == 1.0);
}

void testTouch2(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {5, 10}, d = {15, 15};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_TOUCH);
	LT_ASSERT(alphaP == 0.5);
	LT_ASSERT(alphaQ == 1.0);
}

void testTouch3(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {15, 15}, d = {5, 10};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}

void testTouch4(void)
{
	struct GH_point a = {10,10}, b = {20,20}, c = {15, 15}, d = {5, 10};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	LT_ASSERT(inter_type == INTER_NONE);
}


void testCoincide1(void)
{
	struct GH_point a = {4,4}, b = {0,4}, c = {1, 4}, d = {5, 4};
	float alphaP, alphaQ;
	
	enum intertype_e inter_type = GH_intersect(&a,&b,&c,&d, &alphaP, &alphaQ);
	
	int bits = GH_lineCoincideBits(&a, &b, &c, &d);
	
	LT_ASSERT(inter_type == INTER_COINCIDE);
	LT_ASSERT(bits & A_ONCD);
	LT_ASSERT(bits & C_ONAB);
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
}