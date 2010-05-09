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



#ifndef _POLYMATH_H_
#define _POLYMATH_H_

enum GH_op_t {
	GH_op_subtract, // return where only a present and not b [same walker as intersect, invert b inpoly checks]
	GH_op_intersect, // return where only both present
	
	GH_op_union, // Return where either present
	GH_op_xor,	// Return where only 1 or other, not both
	};

struct GH_polygon_ll {
	struct GH_polygon_ll * next;
    
	struct GH_vertex_ll * firstv;
};

struct GH_vertex_ll;

// Pass NULL to start a new polygon
struct GH_vertex_ll * GH_polyPoint(struct GH_vertex_ll * v, double x, double y);
struct GH_vertex_ll * GH_getPolyPoint(struct GH_vertex_ll *v, double * x, double * y);

struct GH_polygon_ll * GH_polygon_boolean(struct GH_vertex_ll * p1, struct GH_vertex_ll * p2, enum GH_op_t op);
void GH_free_polygons(struct GH_vertex_ll * polys);

#endif