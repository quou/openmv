#pragma once

#include "common.h"

#include <math.h>

#define pif 3.14159265358f
#define pid 3.14159265358979323846

API f64 todegd(f64 rad);
API f64 toradd(f64 deg);
API f32 todegf(f32 rad);
API f32 toradf(f32 deg);

typedef struct {
	f32 x, y;
} v2f;

force_inline v2f make_v2f(f32 x, f32 y) {
	return (v2f) { x, y };
}

force_inline v2f v2f_zero() {
	return (v2f) { 0.0f, 0.0f };
}

force_inline v2f v2f_add(v2f a, v2f b) {
	return (v2f) { a.x + b.x, a.y + b.y };
}

force_inline v2f v2f_sub(v2f a, v2f b) {
	return (v2f) { a.x - b.x, a.y - b.y };
}

force_inline v2f v2f_mul(v2f a, v2f b) {
	return (v2f) { a.x * b.x, a.y * b.y };
}

force_inline v2f v2f_div(v2f a, v2f b) {
	return (v2f) { a.x / b.x, a.y / b.y };
}

force_inline v2f v2f_scale(v2f v, f32 s) {
	return (v2f) { v.x * s, v.y * s };
}

force_inline f32 v2f_mag_sqrd(v2f v) {
	return v.x * v.x + v.y * v.y;
}

force_inline f32 v2f_mag(v2f v) {
	return sqrtf(v2f_mag_sqrd(v));
}

force_inline v2f v2f_normalised(v2f v) {
	const f32 l = v2f_mag(v);
	return (v2f) { v.x / l, v.y / l };
}

force_inline bool v2f_eq(v2f a, v2f b) {
	const f32 tol = 1.192092896e-07f;

    if (fabsf(a.x - b.x) > tol) {
        return false;
    }

    if (fabsf(a.y - b.y) > tol) {
        return false;
    }

    return true;
}

typedef struct {
	i32 x, y;
} v2i;

force_inline v2i make_v2i(i32 x, i32 y) {
	return (v2i) { x, y };
}

force_inline v2i v2i_zero() {
	return (v2i) { 0.0f, 0.0f };
}

force_inline v2i v2i_add(v2i a, v2i b) {
	return (v2i) { a.x + b.x, a.y + b.y };
}

force_inline v2i v2i_sub(v2i a, v2i b) {
	return (v2i) { a.x - b.x, a.y - b.y };
}

force_inline v2i v2i_mul(v2i a, v2i b) {
	return (v2i) { a.x * b.x, a.y * b.y };
}

force_inline v2i v2i_div(v2i a, v2i b) {
	return (v2i) { a.x / b.x, a.y / b.y };
}

force_inline v2i v2i_scale(v2i v, f32 s) {
	return (v2i) { v.x * s, v.y * s };
}

force_inline bool v2i_eq(v2i a, v2i b) {
	return a.x == b.x && a.y == b.y;
}

force_inline i32 v2i_mag_sqrd(v2i v) {
	return v.x * v.x + v.y * v.y;
}

force_inline i32 v2i_mag(v2i v) {
	return (i32)sqrtf((f32)v2i_mag_sqrd(v));
}

typedef struct {
	f32 x, y, z;
} v3f;

force_inline v3f make_v3f(f32 x, f32 y, f32 z) {
	return (v3f) { x, y, z };
}

force_inline v3f v3f_zero() {
	return (v3f) { 0.0f, 0.0f, 0.0f };
}

force_inline v3f v3f_add(v3f a, v3f b) {
	return (v3f) { a.x + b.x, a.y + b.y, a.z + b.z };
}

force_inline v3f v3f_sub(v3f a, v3f b) {
	return (v3f) { a.x - b.x, a.y - b.y, a.z - b.z };
}

force_inline v3f v3f_mul(v3f a, v3f b) {
	return (v3f) { a.x * b.x, a.y * b.y, a.z * b.z };
}

force_inline v3f v3f_div(v3f a, v3f b) {
	return (v3f) { a.x / b.x, a.y / b.y, a.z / b.z };
}

force_inline v3f v3f_scale(v3f v, f32 s) {
	return (v3f) { v.x * s, v.y * s, v.z * s };
}

force_inline f32 v3f_mag_sqrd(v3f v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

force_inline f32 v3f_mag(v3f v) {
	return sqrtf(v3f_mag_sqrd(v));
}

force_inline v3f v3f_normalised(v3f v) {
	const f32 l = v3f_mag(v);
	return (v3f) { v.x / l, v.y / l, v.z / l };
}

force_inline bool v3f_eq(v3f a, v3f b) {
	const f32 tol = 1.192092896e-07f;

    if (fabsf(a.x - b.x) > tol) {
        return false;
    }

    if (fabsf(a.y - b.y) > tol) {
        return false;
    }

    if (fabsf(a.z - b.z) > tol) {
        return false;
    }

    return true;
}

force_inline f32 v3f_dot(v3f a, v3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

force_inline v3f v3f_cross(v3f a, v3f b) {
	return (v3f) {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

typedef struct {
	i32 x, y, z;
} v3i;

force_inline v3i make_v3i(i32 x, i32 y, i32 z) {
	return (v3i) { x, y, z };
}

force_inline v3i v3i_zero() {
	return (v3i) { 0, 0, 0 };
}

force_inline v3i v3i_add(v3i a, v3i b) {
	return (v3i) { a.x + b.x, a.y + b.y, a.z + b.z };
}

force_inline v3i v3i_sub(v3i a, v3i b) {
	return (v3i) { a.x - b.x, a.y - b.y, a.z - b.z };
}

force_inline v3i v3i_mul(v3i a, v3i b) {
	return (v3i) { a.x * b.x, a.y * b.y, a.z * b.z };
}

force_inline v3i v3i_div(v3i a, v3i b) {
	return (v3i) { a.x / b.x, a.y / b.y, a.z / b.z };
}

force_inline v3i v3i_scale(v3i v, i32 s) {
	return (v3i) { v.x * s, v.y * s, v.z * s };
}

force_inline bool v3i_eq(v3i a, v3i b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

force_inline i32 v3i_mag_sqrd(v3i v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

force_inline i32 v3i_mag(v3i v) {
	return (i32)sqrtf((f32)v3i_mag_sqrd(v));
}

typedef struct {
	f32 x, y, z, w;
} v4f;

force_inline v4f make_v4f(f32 x, f32 y, f32 z, f32 w) {
	return (v4f) { x, y, z, w };
}

force_inline v4f v4f_zero() {
	return (v4f) { 0.0f, 0.0f, 0.0f, 0.0f };
}

force_inline v4f v4f_add(v4f a, v4f b) {
	return (v4f) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

force_inline v4f v4f_sub(v4f a, v4f b) {
	return (v4f) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

force_inline v4f v4f_mul(v4f a, v4f b) {
	return (v4f) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

force_inline v4f v4f_div(v4f a, v4f b) {
	return (v4f) { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

force_inline v4f v4f_scale(v4f v, f32 s) {
	return (v4f) { v.x * s, v.y * s, v.z * s, v.w * s };
}

force_inline f32 v4f_mag_sqrd(v4f v) {
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

force_inline f32 v4f_mag(v4f v) {
	return sqrtf(v4f_mag_sqrd(v));
}

force_inline v4f v4f_normalised(v4f v) {
	const f32 l = v4f_mag(v);
	return (v4f) { v.x / l, v.y / l, v.z / l, v.w / l };
}

force_inline bool v4f_eq(v4f a, v4f b) {
	const f32 tol = 1.192092896e-07f;

    if (fabsf(a.x - b.x) > tol) {
        return false;
    }

    if (fabsf(a.y - b.y) > tol) {
        return false;
    }

    if (fabsf(a.z - b.z) > tol) {
        return false;
    }

    return true;
}

typedef struct {
	i32 x, y, z, w;
} v4i;

force_inline v4i make_v4i(i32 x, i32 y, i32 z, i32 w) {
	return (v4i) { x, y, z, w };
}

force_inline v4i v4i_zero() {
	return (v4i) { 0, 0, 0, 0 };
}

force_inline v4i v4i_add(v4i a, v4i b) {
	return (v4i) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

force_inline v4i v4i_sub(v4i a, v4i b) {
	return (v4i) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

force_inline v4i v4i_mul(v4i a, v4i b) {
	return (v4i) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

force_inline v4i v4i_div(v4i a, v4i b) {
	return (v4i) { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

force_inline v4i v4i_scale(v4i v, i32 s) {
	return (v4i) { v.x * s, v.y * s, v.z * s, v.w * s };
}

force_inline bool v4i_eq(v4i a, v4i b) {
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

force_inline i32 v4i_mag_sqrd(v4i v) {
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

force_inline i32 v4i_mag(v4i v) {
	return (i32)sqrtf((f32)v4i_mag_sqrd(v));
}

typedef struct {
	f32 m[4][4];
} m4f;

force_inline m4f make_m4f(f32 d) {
	m4f r = { 0 };

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;
	r.m[3][3] = d;

	return r;
}

force_inline m4f m4f_identity() {
	return make_m4f(1.0f);
}

force_inline m4f m4f_mul(m4f a, m4f b) {
	m4f r = m4f_identity();

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1] + a.m[2][0] * b.m[0][2] + a.m[3][0] * b.m[0][3];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1] + a.m[2][0] * b.m[1][2] + a.m[3][0] * b.m[1][3];
	r.m[2][0] = a.m[0][0] * b.m[2][0] + a.m[1][0] * b.m[2][1] + a.m[2][0] * b.m[2][2] + a.m[3][0] * b.m[2][3];
	r.m[3][0] = a.m[0][0] * b.m[3][0] + a.m[1][0] * b.m[3][1] + a.m[2][0] * b.m[3][2] + a.m[3][0] * b.m[3][3];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1] + a.m[2][1] * b.m[0][2] + a.m[3][1] * b.m[0][3];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1] + a.m[2][1] * b.m[1][2] + a.m[3][1] * b.m[1][3];
	r.m[2][1] = a.m[0][1] * b.m[2][0] + a.m[1][1] * b.m[2][1] + a.m[2][1] * b.m[2][2] + a.m[3][1] * b.m[2][3];
	r.m[3][1] = a.m[0][1] * b.m[3][0] + a.m[1][1] * b.m[3][1] + a.m[2][1] * b.m[3][2] + a.m[3][1] * b.m[3][3];
	r.m[0][2] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[0][1] + a.m[2][2] * b.m[0][2] + a.m[3][2] * b.m[0][3];
	r.m[1][2] = a.m[0][2] * b.m[1][0] + a.m[1][2] * b.m[1][1] + a.m[2][2] * b.m[1][2] + a.m[3][2] * b.m[1][3];
	r.m[2][2] = a.m[0][2] * b.m[2][0] + a.m[1][2] * b.m[2][1] + a.m[2][2] * b.m[2][2] + a.m[3][2] * b.m[2][3];
	r.m[3][2] = a.m[0][2] * b.m[3][0] + a.m[1][2] * b.m[3][1] + a.m[2][2] * b.m[3][2] + a.m[3][2] * b.m[3][3];
	r.m[0][3] = a.m[0][3] * b.m[0][0] + a.m[1][3] * b.m[0][1] + a.m[2][3] * b.m[0][2] + a.m[3][3] * b.m[0][3];
	r.m[1][3] = a.m[0][3] * b.m[1][0] + a.m[1][3] * b.m[1][1] + a.m[2][3] * b.m[1][2] + a.m[3][3] * b.m[1][3];
	r.m[2][3] = a.m[0][3] * b.m[2][0] + a.m[1][3] * b.m[2][1] + a.m[2][3] * b.m[2][2] + a.m[3][3] * b.m[2][3];
	r.m[3][3] = a.m[0][3] * b.m[3][0] + a.m[1][3] * b.m[3][1] + a.m[2][3] * b.m[3][2] + a.m[3][3] * b.m[3][3];

	return r;
}

force_inline m4f m4f_translate(m4f m, v3f v) {
	m4f r = m4f_identity();

	r.m[3][0] = v.x;
	r.m[3][1] = v.y;
	r.m[3][2] = v.z;

	return m4f_mul(m, r);
}

force_inline m4f m4f_rotate(m4f m, f32 a, v3f v) {
	m4f r = m4f_identity();

	const f32 c = (f32)cos((f64)a);
	const f32 s = (f32)sin((f64)a);

	const f32 omc = (f32)1 - c;

	const f32 x = v.x;
	const f32 y = v.y;
	const f32 z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m4f_mul(m, r);
}

force_inline m4f m4f_scale(m4f m, v3f v) {
	m4f r = m4f_identity();

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m4f_mul(m, r);
}

force_inline v4f m4f_transform(m4f m, v4f v) {
	return make_v4f(
		m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] + v.w,
		m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] + v.w,
		m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] + v.w,
		m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] + v.w);
}

force_inline m4f m4f_lookat(v3f c, v3f o, v3f u) {
	m4f r = m4f_identity();

	const v3f f = v3f_normalised(v3f_sub(o, c));
	u = v3f_normalised(u);
	const v3f s = v3f_normalised(v3f_cross(f, u));
	u = v3f_cross(s, f);

	r.m[0][0] = s.x;
	r.m[1][0] = s.y;
	r.m[2][0] = s.z;
	r.m[0][1] = u.x;
	r.m[1][1] = u.y;
	r.m[2][1] = u.z;
	r.m[0][2] = -f.x;
	r.m[1][2] = -f.y;
	r.m[2][2] = -f.z;
	r.m[3][0] = -v3f_dot(s, c);
	r.m[3][1] = -v3f_dot(u, c);
	r.m[3][2] = v3f_dot(f, c);

	return r;
}

force_inline m4f m4f_pers(f32 fov, f32 asp, f32 n, f32 f) {
	m4f r = m4f_identity();

	const f32 q = 1.0f / tanf(toradf(fov) / 2.0f);
	const f32 a = q / asp;
	const f32 b = (n + f) / (n - f);
	const f32 c = (2.0f * n * f) / (n - f);

	r.m[0][0] = a;
	r.m[1][1] = q;
	r.m[2][2] = b;
	r.m[2][3] = -1.0f;
	r.m[3][2] = c;

	return r;
}

force_inline m4f m4f_orth(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
	m4f res = m4f_identity();

	res.m[0][0] = (f32)2 / (r - l);
	res.m[1][1] = (f32)2 / (t - b);
	res.m[2][2] = (f32)2 / (n - f);

	res.m[3][0] = (l + r) / (l - r);
	res.m[3][1] = (b + t) / (b - t);
	res.m[3][2] = (f + n) / (f - n);

	return res;
}

force_inline m4f m4f_invert(m4f mat) {
	const f32* m = (f32*)mat.m;

	f32 t0 = m[10] * m[15];
	f32 t1 = m[14] * m[11];
	f32 t2 = m[6] * m[15];
	f32 t3 = m[14] * m[7];
	f32 t4 = m[6] * m[11];
	f32 t5 = m[10] * m[7];
	f32 t6 = m[2] * m[15];
	f32 t7 = m[14] * m[3];
	f32 t8 = m[2] * m[11];
	f32 t9 = m[10] * m[3];
	f32 t10 = m[2] * m[7];
	f32 t11 = m[6] * m[3];
	f32 t12 = m[8] * m[13];
	f32 t13 = m[12] * m[9];
	f32 t14 = m[4] * m[13];
	f32 t15 = m[12] * m[5];
	f32 t16 = m[4] * m[9];
	f32 t17 = m[8] * m[5];
	f32 t18 = m[0] * m[13];
	f32 t19 = m[12] * m[1];
	f32 t20 = m[0] * m[9];
	f32 t21 = m[8] * m[1];
	f32 t22 = m[0] * m[5];
	f32 t23 = m[4] * m[1];

	m4f r;
	f32* o = (f32*)r.m;

	o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
	o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
	o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
	o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

	f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

	o[0] = d * o[0];
	o[1] = d * o[1];
	o[2] = d * o[2];
	o[3] = d * o[3];
	o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
	o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
	o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
	o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
	o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
	o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
	o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
	o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
	o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
	o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
	o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
	o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

	return r;
}

force_inline m4f m4f_transpose(m4f m) {
    m4f r = m4f_identity();

    r.m[0][0] = m.m[0][0];
    r.m[1][0] = m.m[0][1];
    r.m[2][0] = m.m[0][2];
    r.m[3][0] = m.m[0][3];
    r.m[0][1] = m.m[1][0];
    r.m[1][1] = m.m[1][1];
    r.m[2][1] = m.m[1][2];
    r.m[3][1] = m.m[1][3];
    r.m[0][2] = m.m[2][0];
    r.m[1][2] = m.m[2][1];
    r.m[2][2] = m.m[2][2];
    r.m[3][2] = m.m[2][3];
    r.m[0][3] = m.m[3][0];
    r.m[1][3] = m.m[3][1];
    r.m[2][3] = m.m[3][2];
    r.m[3][3] = m.m[3][3];

    return r;
}

typedef struct {
	f32 m[4][4];
} m3f;
