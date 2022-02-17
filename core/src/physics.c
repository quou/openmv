#include <math.h>
#include <stdlib.h>

#include "physics.h"

static i32 sort_cmp(const void* a, const void* b) {
	return *(i32*)a > *(i32*)b;
}

bool rect_overlap(struct rect a, struct rect b, v2i* normal) {
	if (!(
		a.x + a.w > b.x &&
		a.y + a.h > b.y &&
		a.x < b.x + b.w &&
		a.y < b.y + b.h)) {
		if (normal) {
			*normal = make_v2i(0, 0);
		}
		return false;
	}

	if (normal) {
		i32 right  = (a.x + a.w) - b.x;
		i32 left   = (b.x + b.w) - a.x;
		i32 top    = (b.y + b.h) - a.y;
		i32 bottom = (a.y + a.h) - b.y;

		i32 overlap[] = { right, left, top, bottom };

		qsort(overlap, sizeof(overlap) / sizeof(*overlap), sizeof(i32), sort_cmp);

		*normal = make_v2i(0, 0);
		if (overlap[0]        == abs(right)) {
			normal->x =  1;
		} else if (overlap[0] == abs(left)) {
			normal->x = -1;
		} else if (overlap[0] == abs(bottom)) {
			normal->y =  1;
		} else if (overlap[0] == abs(top)) {
			normal->y = -1;
		}
	}

	return true;
}

static i32 area(v2i a, v2i b, v2i c) {
	return (i32)abs((i32)((double)((a.x * (b.y - c.y) + b.x *(c.y - a.y) + c.x *(a.y - b.y))) / 2.0));
}

bool point_vs_tri(v2i p, v2i a, v2i b, v2i c) {
	i32 A  = area(a, b, c);
	i32 A1 = area(p, b, c);
	i32 A2 = area(p, a, c);
	i32 A3 = area(p, a, b);

	return (A == A1 + A2 + A3);
}

bool point_vs_rtri(v2i p, v2i a, v2i b) {
	v2i c = make_v2i(a.x, b.y);

	return point_vs_tri(p, a, b, c);
}
