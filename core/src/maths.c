#include <math.h>

#include "maths.h"

f64 todeg(f64 rad) {
	return rad * (180.0f / pid);
}

f64 torad(f64 deg) {
	return deg * (pid / 180.0f);
}

f32 todegf(f32 rad) {
	return rad * (180.0f / pif);
}

f32 toradf(f32 deg) {
	return deg * (pif / 180.0f);
}
