#pragma once

#include "common.h"
#include "maths.h"
#include "video.h"

API bool rect_overlap(struct rect a, struct rect b, v2i* normal);
