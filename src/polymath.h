/*
 *  Copyright 2009, 2010 David Carne. Released under the revised BSD license.
 */



#ifndef _POLYMATH_H_
#define _POLYMATH_H_

enum PC_op_t {
	PC_op_subtract, // return where only a present and not b [same walker as intersect, invert b inpoly checks]
	PC_op_intersect, // return where only both present
	
	PC_op_union, // Return where either present
	PC_op_xor,	// Return where only 1 or other, not both
	};

struct PC_polygon_ll {
	struct PC_polygon_ll * next;
    
	struct PC_vertex_ll * firstv;
};

struct PC_vertex_ll;

// Pass NULL to start a new polygon
struct PC_vertex_ll * PC_polyPoint(struct PC_vertex_ll * v, double x, double y);
struct PC_vertex_ll * PC_getPolyPoint(struct PC_vertex_ll *v, double * x, double * y);

struct PC_polygon_ll * PC_polygon_boolean(struct PC_vertex_ll * p1, struct PC_vertex_ll * p2, enum PC_op_t op);
void PC_free_verticies(struct PC_vertex_ll * verticies);
void PC_free_polys(struct PC_polygon_ll * polys);
#endif