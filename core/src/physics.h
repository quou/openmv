#pragma once

#include "common.h"
#include "maths.h"
#include "video.h"

API bool rect_overlap(struct rect a, struct rect b, v2i* normal);
API bool point_vs_tri(v2i p, v2i a, v2i b, v2i c);
API bool point_vs_rtri(v2i p, v2i a, v2i b);
