#include <stdlib.h>

#include "physics.h"

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

		qsort(overlap, sizeof(overlap) / sizeof(*overlap), sizeof(i32),
			lambda(i32 _(const void* a, const void* b) {
				return *(i32*)a > *(i32*)b;
			}));

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
