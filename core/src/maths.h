#pragma once

#include "common.h"

#include <stdint.h>

#define pie 3.14159265358f

API double todeg(double rad);
API double torad(double deg);

typedef struct {
	u32 x;
	u32 y;
	u32 z;
	u32 w;
} v4u;

API u32 v4u_magnitude(v4u v);
API v4u make_v4u(u32 x, u32 y, u32 z, u32 w);
API v4u v4u_mul(v4u a, v4u b);
API v4u v4u_normalised(v4u v);
API u32 v4u_dot(v4u a, v4u b);
API v4u v4u_add(v4u a, v4u b);
API v4u v4u_scale(v4u v, u32 s);
API v4u v4u_div(v4u a, v4u b);
API v4u v4u_sub(v4u a, v4u b);

typedef struct {
	float x;
	float y;
	float z;
} v3f;

API float v3f_magnitude(v3f v);
API v3f make_v3f(float x, float y, float z);
API v3f v3f_mul(v3f a, v3f b);
API v3f v3f_cross(v3f a, v3f b);
API v3f v3f_normalised(v3f v);
API float v3f_dot(v3f a, v3f b);
API v3f v3f_add(v3f a, v3f b);
API v3f v3f_scale(v3f v, float s);
API v3f v3f_div(v3f a, v3f b);
API v3f v3f_sub(v3f a, v3f b);

typedef struct {
	double x;
	double y;
	double z;
	double w;
} v4d;

API double v4d_magnitude(v4d v);
API v4d make_v4d(double x, double y, double z, double w);
API v4d v4d_mul(v4d a, v4d b);
API v4d v4d_normalised(v4d v);
API double v4d_dot(v4d a, v4d b);
API v4d v4d_add(v4d a, v4d b);
API v4d v4d_scale(v4d v, double s);
API v4d v4d_div(v4d a, v4d b);
API v4d v4d_sub(v4d a, v4d b);

typedef struct {
	float x;
	float y;
	float z;
	float w;
} v4f;

API float v4f_magnitude(v4f v);
API v4f make_v4f(float x, float y, float z, float w);
API v4f v4f_mul(v4f a, v4f b);
API v4f v4f_normalised(v4f v);
API float v4f_dot(v4f a, v4f b);
API v4f v4f_add(v4f a, v4f b);
API v4f v4f_scale(v4f v, float s);
API v4f v4f_div(v4f a, v4f b);
API v4f v4f_sub(v4f a, v4f b);

typedef struct {
	i32 x;
	i32 y;
} v2i;

API i32 v2i_magnitude(v2i v);
API v2i make_v2i(i32 x, i32 y);
API v2i v2i_mul(v2i a, v2i b);
API v2i v2i_normalised(v2i v);
API i32 v2i_dot(v2i a, v2i b);
API v2i v2i_add(v2i a, v2i b);
API v2i v2i_scale(v2i v, i32 s);
API v2i v2i_div(v2i a, v2i b);
API v2i v2i_sub(v2i a, v2i b);

typedef struct {
	float x;
	float y;
} v2f;

API float v2f_magnitude(v2f v);
API v2f make_v2f(float x, float y);
API v2f v2f_mul(v2f a, v2f b);
API v2f v2f_normalised(v2f v);
API float v2f_dot(v2f a, v2f b);
API v2f v2f_add(v2f a, v2f b);
API v2f v2f_scale(v2f v, float s);
API v2f v2f_div(v2f a, v2f b);
API v2f v2f_sub(v2f a, v2f b);

typedef struct {
	double x;
	double y;
	double z;
} v3d;

API double v3d_magnitude(v3d v);
API v3d make_v3d(double x, double y, double z);
API v3d v3d_mul(v3d a, v3d b);
API v3d v3d_cross(v3d a, v3d b);
API v3d v3d_normalised(v3d v);
API double v3d_dot(v3d a, v3d b);
API v3d v3d_add(v3d a, v3d b);
API v3d v3d_scale(v3d v, double s);
API v3d v3d_div(v3d a, v3d b);
API v3d v3d_sub(v3d a, v3d b);

typedef struct {
	double x;
	double y;
} v2d;

API double v2d_magnitude(v2d v);
API v2d make_v2d(double x, double y);
API v2d v2d_mul(v2d a, v2d b);
API v2d v2d_normalised(v2d v);
API double v2d_dot(v2d a, v2d b);
API v2d v2d_add(v2d a, v2d b);
API v2d v2d_scale(v2d v, double s);
API v2d v2d_div(v2d a, v2d b);
API v2d v2d_sub(v2d a, v2d b);

typedef struct {
	u32 x;
	u32 y;
	u32 z;
} v3u;

API u32 v3u_magnitude(v3u v);
API v3u make_v3u(u32 x, u32 y, u32 z);
API v3u v3u_mul(v3u a, v3u b);
API v3u v3u_cross(v3u a, v3u b);
API v3u v3u_normalised(v3u v);
API u32 v3u_dot(v3u a, v3u b);
API v3u v3u_add(v3u a, v3u b);
API v3u v3u_scale(v3u v, u32 s);
API v3u v3u_div(v3u a, v3u b);
API v3u v3u_sub(v3u a, v3u b);

typedef struct {
	u32 x;
	u32 y;
} v2u;

API u32 v2u_magnitude(v2u v);
API v2u make_v2u(u32 x, u32 y);
API v2u v2u_mul(v2u a, v2u b);
API v2u v2u_normalised(v2u v);
API u32 v2u_dot(v2u a, v2u b);
API v2u v2u_add(v2u a, v2u b);
API v2u v2u_scale(v2u v, u32 s);
API v2u v2u_div(v2u a, v2u b);
API v2u v2u_sub(v2u a, v2u b);

typedef struct {
	i32 x;
	i32 y;
	i32 z;
} v3i;

API i32 v3i_magnitude(v3i v);
API v3i make_v3i(i32 x, i32 y, i32 z);
API v3i v3i_mul(v3i a, v3i b);
API v3i v3i_cross(v3i a, v3i b);
API v3i v3i_normalised(v3i v);
API i32 v3i_dot(v3i a, v3i b);
API v3i v3i_add(v3i a, v3i b);
API v3i v3i_scale(v3i v, i32 s);
API v3i v3i_div(v3i a, v3i b);
API v3i v3i_sub(v3i a, v3i b);

typedef struct {
	i32 x;
	i32 y;
	i32 z;
	i32 w;
} v4i;

API i32 v4i_magnitude(v4i v);
API v4i make_v4i(i32 x, i32 y, i32 z, i32 w);
API v4i v4i_mul(v4i a, v4i b);
API v4i v4i_normalised(v4i v);
API i32 v4i_dot(v4i a, v4i b);
API v4i v4i_add(v4i a, v4i b);
API v4i v4i_scale(v4i v, i32 s);
API v4i v4i_div(v4i a, v4i b);
API v4i v4i_sub(v4i a, v4i b);

typedef struct {
	i32 m[2][2];
} m2i;

API m2i m2i_identity();
API m2i make_m2i(i32 d);
API m2i m2i_mul(m2i a, m2i b);

typedef struct {
	u32 m[2][2];
} m2u;

API m2u m2u_identity();
API m2u make_m2u(u32 d);
API m2u m2u_mul(m2u a, m2u b);

typedef struct {
	i32 m[3][3];
} m3i;

API m3i m3i_scale(m3i m, v3i v);
API m3i m3i_identity();
API m3i make_m3i(i32 d);
API m3i m3i_rotate(m3i m, i32 a, v3i v);
API m3i m3i_mul(m3i a, m3i b);

typedef struct {
	u32 m[4][4];
} m4u;

API m4u m4u_translate(m4u m, v3u v);
API m4u m4u_scale(m4u m, v3u v);
API m4u m4u_identity();
API m4u make_m4u(u32 d);
API m4u m4u_rotate(m4u m, u32 a, v3u v);
API v4u m4u_transform(m4u m, v4u v);
API m4u m4u_mul(m4u a, m4u b);
API m4u m4u_lookat(v3u c, v3u o, v3u u);
API m4u m4u_pers(u32 fov, u32 asp, u32 n, u32 f);
API m4u m4u_orth(u32 l, u32 r, u32 b, u32 t, u32 n, u32 f);

typedef struct {
	double m[4][4];
} m4d;

API m4d m4d_translate(m4d m, v3d v);
API m4d m4d_scale(m4d m, v3d v);
API m4d m4d_identity();
API m4d make_m4d(double d);
API m4d m4d_rotate(m4d m, double a, v3d v);
API v4d m4d_transform(m4d m, v4d v);
API m4d m4d_mul(m4d a, m4d b);
API m4d m4d_lookat(v3d c, v3d o, v3d u);
API m4d m4d_pers(double fov, double asp, double n, double f);
API m4d m4d_orth(double l, double r, double b, double t, double n, double f);

typedef struct {
	i32 m[4][4];
} m4i;

API m4i m4i_translate(m4i m, v3i v);
API m4i m4i_scale(m4i m, v3i v);
API m4i m4i_identity();
API m4i make_m4i(i32 d);
API m4i m4i_rotate(m4i m, i32 a, v3i v);
API v4i m4i_transform(m4i m, v4i v);
API m4i m4i_mul(m4i a, m4i b);
API m4i m4i_lookat(v3i c, v3i o, v3i u);
API m4i m4i_pers(i32 fov, i32 asp, i32 n, i32 f);
API m4i m4i_orth(i32 l, i32 r, i32 b, i32 t, i32 n, i32 f);

typedef struct {
	float m[4][4];
} m4f;

API m4f m4f_translate(m4f m, v3f v);
API m4f m4f_scale(m4f m, v3f v);
API m4f m4f_identity();
API m4f make_m4f(float d);
API m4f m4f_rotate(m4f m, float a, v3f v);
API v4f m4f_transform(m4f m, v4f v);
API m4f m4f_mul(m4f a, m4f b);
API m4f m4f_lookat(v3f c, v3f o, v3f u);
API m4f m4f_pers(float fov, float asp, float n, float f);
API m4f m4f_orth(float l, float r, float b, float t, float n, float f);

typedef struct {
	u32 m[3][3];
} m3u;

API m3u m3u_scale(m3u m, v3u v);
API m3u m3u_identity();
API m3u make_m3u(u32 d);
API m3u m3u_rotate(m3u m, u32 a, v3u v);
API m3u m3u_mul(m3u a, m3u b);
 
typedef struct {
	double m[2][2];
} m2d;

API m2d m2d_identity();
API m2d make_m2d(double d);
API m2d m2d_mul(m2d a, m2d b);

typedef struct {
	double m[3][3];
} m3d;

API m3d m3d_scale(m3d m, v3d v);
API m3d m3d_identity();
API m3d make_m3d(double d);
API m3d m3d_rotate(m3d m, double a, v3d v);
API m3d m3d_mul(m3d a, m3d b);

typedef struct {
	float m[2][2];
} m2f;

API m2f m2f_identity();
API m2f make_m2f(float d);
API m2f m2f_mul(m2f a, m2f b);

typedef struct {
	float m[3][3];
} m3f;

API m3f m3f_scale(m3f m, v3f v);
API m3f m3f_identity();
API m3f make_m3f(float d);
API m3f m3f_rotate(m3f m, float a, v3f v);
API m3f m3f_mul(m3f a, m3f b);

/* Implementation */
#ifdef SML_IMPL

#include <math.h>

double todeg(double rad) {
	return rad * (180.0 / pie);
}

double torad(double deg) {
	return deg * (pie / 180.0);
}

u32 v4u_magnitude(v4u v) {
	return (u32)sqrt(((u32)v.x * (u32)v.x) + ((u32)v.y * (u32)v.y) + ((u32)v.z * (u32)v.z) + ((u32)v.w * (u32)v.w));
}

v4u make_v4u(u32 x, u32 y, u32 z, u32 w) {
	return (v4u) { x, y, z, w };
}

v4u v4u_mul(v4u a, v4u b) {
	return (v4u) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

v4u v4u_normalised(v4u v) {
	const u32 l = v4u_magnitude(v);
	return l == (u32)0 ? (v4u) { (u32)0, (u32)0, (u32)0, (u32)0} : (v4u) { (u32)v.x / l, (u32)v.y / l, (u32)v.z / l, (u32)v.w / l};
}

u32 v4u_dot(v4u a, v4u b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w));
}

v4u v4u_add(v4u a, v4u b) {
	return (v4u) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

v4u v4u_scale(v4u v, u32 s) {
	return (v4u) { v.x * s, v.y * s, v.z * s, v.w * s };
}

v4u v4u_div(v4u a, v4u b) {
	return (v4u) { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

v4u v4u_sub(v4u a, v4u b) {
	return (v4u) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

float v3f_magnitude(v3f v) {
	return (float)sqrt(((float)v.x * (float)v.x) + ((float)v.y * (float)v.y) + ((float)v.z * (float)v.z));
}

v3f make_v3f(float x, float y, float z) {
	return (v3f) { x, y, z };
}

v3f v3f_mul(v3f a, v3f b) {
	return (v3f) { a.x * b.x, a.y * b.y, a.z * b.z };
}

v3f v3f_cross(v3f a, v3f b) {
	return make_v3f(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.y * b.z,
			a.x * b.y - a.y * b.x);
}

v3f v3f_normalised(v3f v) {
	const float l = v3f_magnitude(v);
	return l == (float)0 ? (v3f) { (float)0, (float)0, (float)0} : (v3f) { (float)v.x / l, (float)v.y / l, (float)v.z / l};
}

float v3f_dot(v3f a, v3f b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

v3f v3f_add(v3f a, v3f b) {
	return (v3f) { a.x + b.x, a.y + b.y, a.z + b.z };
}

v3f v3f_scale(v3f v, float s) {
	return (v3f) { v.x * s, v.y * s, v.z * s };
}

v3f v3f_div(v3f a, v3f b) {
	return (v3f) { a.x / b.x, a.y / b.y, a.z / b.z };
}

v3f v3f_sub(v3f a, v3f b) {
	return (v3f) { a.x - b.x, a.y - b.y, a.z - b.z };
}

double v4d_magnitude(v4d v) {
	return (double)sqrt(((double)v.x * (double)v.x) + ((double)v.y * (double)v.y) + ((double)v.z * (double)v.z) + ((double)v.w * (double)v.w));
}

v4d make_v4d(double x, double y, double z, double w) {
	return (v4d) { x, y, z, w };
}

v4d v4d_mul(v4d a, v4d b) {
	return (v4d) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

v4d v4d_normalised(v4d v) {
	const double l = v4d_magnitude(v);
	return l == (double)0 ? (v4d) { (double)0, (double)0, (double)0, (double)0} : (v4d) { (double)v.x / l, (double)v.y / l, (double)v.z / l, (double)v.w / l};
}

double v4d_dot(v4d a, v4d b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w));
}

v4d v4d_add(v4d a, v4d b) {
	return (v4d) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

v4d v4d_scale(v4d v, double s) {
	return (v4d) { v.x * s, v.y * s, v.z * s, v.w * s };
}

v4d v4d_div(v4d a, v4d b) {
	return (v4d) { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

v4d v4d_sub(v4d a, v4d b) {
	return (v4d) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

float v4f_magnitude(v4f v) {
	return (float)sqrt(((float)v.x * (float)v.x) + ((float)v.y * (float)v.y) + ((float)v.z * (float)v.z) + ((float)v.w * (float)v.w));
}

v4f make_v4f(float x, float y, float z, float w) {
	return (v4f) { x, y, z, w };
}

v4f v4f_mul(v4f a, v4f b) {
	return (v4f) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

v4f v4f_normalised(v4f v) {
	const float l = v4f_magnitude(v);
	return l == (float)0 ? (v4f) { (float)0, (float)0, (float)0, (float)0} : (v4f) { (float)v.x / l, (float)v.y / l, (float)v.z / l, (float)v.w / l};
}

float v4f_dot(v4f a, v4f b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w));
}

v4f v4f_add(v4f a, v4f b) {
	return (v4f) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

v4f v4f_scale(v4f v, float s) {
	return (v4f) { v.x * s, v.y * s, v.z * s, v.w * s };
}

v4f v4f_div(v4f a, v4f b) {
	return (v4f) { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

v4f v4f_sub(v4f a, v4f b) {
	return (v4f) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

i32 v2i_magnitude(v2i v) {
	return (i32)sqrt(((i32)v.x * (i32)v.x) + ((i32)v.y * (i32)v.y));
}

v2i make_v2i(i32 x, i32 y) {
	return (v2i) { x, y };
}

v2i v2i_mul(v2i a, v2i b) {
	return (v2i) { a.x * b.x, a.y * b.y };
}

v2i v2i_normalised(v2i v) {
	const i32 l = v2i_magnitude(v);
	return l == (i32)0 ? (v2i) { (i32)0, (i32)0} : (v2i) { (i32)v.x / l, (i32)v.y / l};
}

i32 v2i_dot(v2i a, v2i b) {
	return ((a.x * b.x) + (a.y * b.y));
}

v2i v2i_add(v2i a, v2i b) {
	return (v2i) { a.x + b.x, a.y + b.y };
}

v2i v2i_scale(v2i v, i32 s) {
	return (v2i) { v.x * s, v.y * s };
}

v2i v2i_div(v2i a, v2i b) {
	return (v2i) { a.x / b.x, a.y / b.y };
}

v2i v2i_sub(v2i a, v2i b) {
	return (v2i) { a.x - b.x, a.y - b.y };
}

float v2f_magnitude(v2f v) {
	return (float)sqrt(((float)v.x * (float)v.x) + ((float)v.y * (float)v.y));
}

v2f make_v2f(float x, float y) {
	return (v2f) { x, y };
}

v2f v2f_mul(v2f a, v2f b) {
	return (v2f) { a.x * b.x, a.y * b.y };
}

v2f v2f_normalised(v2f v) {
	const float l = v2f_magnitude(v);
	return l == (float)0 ? (v2f) { (float)0, (float)0} : (v2f) { (float)v.x / l, (float)v.y / l};
}

float v2f_dot(v2f a, v2f b) {
	return ((a.x * b.x) + (a.y * b.y));
}

v2f v2f_add(v2f a, v2f b) {
	return (v2f) { a.x + b.x, a.y + b.y };
}

v2f v2f_scale(v2f v, float s) {
	return (v2f) { v.x * s, v.y * s };
}

v2f v2f_div(v2f a, v2f b) {
	return (v2f) { a.x / b.x, a.y / b.y };
}

v2f v2f_sub(v2f a, v2f b) {
	return (v2f) { a.x - b.x, a.y - b.y };
}

double v3d_magnitude(v3d v) {
	return (double)sqrt(((double)v.x * (double)v.x) + ((double)v.y * (double)v.y) + ((double)v.z * (double)v.z));
}

v3d make_v3d(double x, double y, double z) {
	return (v3d) { x, y, z };
}

v3d v3d_mul(v3d a, v3d b) {
	return (v3d) { a.x * b.x, a.y * b.y, a.z * b.z };
}

v3d v3d_cross(v3d a, v3d b) {
	return make_v3d(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.y * b.z,
			a.x * b.y - a.y * b.x);
}

v3d v3d_normalised(v3d v) {
	const double l = v3d_magnitude(v);
	return l == (double)0 ? (v3d) { (double)0, (double)0, (double)0} : (v3d) { (double)v.x / l, (double)v.y / l, (double)v.z / l};
}

double v3d_dot(v3d a, v3d b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

v3d v3d_add(v3d a, v3d b) {
	return (v3d) { a.x + b.x, a.y + b.y, a.z + b.z };
}

v3d v3d_scale(v3d v, double s) {
	return (v3d) { v.x * s, v.y * s, v.z * s };
}

v3d v3d_div(v3d a, v3d b) {
	return (v3d) { a.x / b.x, a.y / b.y, a.z / b.z };
}

v3d v3d_sub(v3d a, v3d b) {
	return (v3d) { a.x - b.x, a.y - b.y, a.z - b.z };
}

double v2d_magnitude(v2d v) {
	return (double)sqrt(((double)v.x * (double)v.x) + ((double)v.y * (double)v.y));
}

v2d make_v2d(double x, double y) {
	return (v2d) { x, y };
}

v2d v2d_mul(v2d a, v2d b) {
	return (v2d) { a.x * b.x, a.y * b.y };
}

v2d v2d_normalised(v2d v) {
	const double l = v2d_magnitude(v);
	return l == (double)0 ? (v2d) { (double)0, (double)0} : (v2d) { (double)v.x / l, (double)v.y / l};
}

double v2d_dot(v2d a, v2d b) {
	return ((a.x * b.x) + (a.y * b.y));
}

v2d v2d_add(v2d a, v2d b) {
	return (v2d) { a.x + b.x, a.y + b.y };
}

v2d v2d_scale(v2d v, double s) {
	return (v2d) { v.x * s, v.y * s };
}

v2d v2d_div(v2d a, v2d b) {
	return (v2d) { a.x / b.x, a.y / b.y };
}

v2d v2d_sub(v2d a, v2d b) {
	return (v2d) { a.x - b.x, a.y - b.y };
}

u32 v3u_magnitude(v3u v) {
	return (u32)sqrt(((u32)v.x * (u32)v.x) + ((u32)v.y * (u32)v.y) + ((u32)v.z * (u32)v.z));
}

v3u make_v3u(u32 x, u32 y, u32 z) {
	return (v3u) { x, y, z };
}

v3u v3u_mul(v3u a, v3u b) {
	return (v3u) { a.x * b.x, a.y * b.y, a.z * b.z };
}

v3u v3u_cross(v3u a, v3u b) {
	return make_v3u(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.y * b.z,
			a.x * b.y - a.y * b.x);
}

v3u v3u_normalised(v3u v) {
	const u32 l = v3u_magnitude(v);
	return l == (u32)0 ? (v3u) { (u32)0, (u32)0, (u32)0} : (v3u) { (u32)v.x / l, (u32)v.y / l, (u32)v.z / l};
}

u32 v3u_dot(v3u a, v3u b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

v3u v3u_add(v3u a, v3u b) {
	return (v3u) { a.x + b.x, a.y + b.y, a.z + b.z };
}

v3u v3u_scale(v3u v, u32 s) {
	return (v3u) { v.x * s, v.y * s, v.z * s };
}

v3u v3u_div(v3u a, v3u b) {
	return (v3u) { a.x / b.x, a.y / b.y, a.z / b.z };
}

v3u v3u_sub(v3u a, v3u b) {
	return (v3u) { a.x - b.x, a.y - b.y, a.z - b.z };
}

u32 v2u_magnitude(v2u v) {
	return (u32)sqrt(((u32)v.x * (u32)v.x) + ((u32)v.y * (u32)v.y));
}

v2u make_v2u(u32 x, u32 y) {
	return (v2u) { x, y };
}

v2u v2u_mul(v2u a, v2u b) {
	return (v2u) { a.x * b.x, a.y * b.y };
}

v2u v2u_normalised(v2u v) {
	const u32 l = v2u_magnitude(v);
	return l == (u32)0 ? (v2u) { (u32)0, (u32)0} : (v2u) { (u32)v.x / l, (u32)v.y / l};
}

u32 v2u_dot(v2u a, v2u b) {
	return ((a.x * b.x) + (a.y * b.y));
}

v2u v2u_add(v2u a, v2u b) {
	return (v2u) { a.x + b.x, a.y + b.y };
}

v2u v2u_scale(v2u v, u32 s) {
	return (v2u) { v.x * s, v.y * s };
}

v2u v2u_div(v2u a, v2u b) {
	return (v2u) { a.x / b.x, a.y / b.y };
}

v2u v2u_sub(v2u a, v2u b) {
	return (v2u) { a.x - b.x, a.y - b.y };
}

i32 v3i_magnitude(v3i v) {
	return (i32)sqrt(((i32)v.x * (i32)v.x) + ((i32)v.y * (i32)v.y) + ((i32)v.z * (i32)v.z));
}

v3i make_v3i(i32 x, i32 y, i32 z) {
	return (v3i) { x, y, z };
}

v3i v3i_mul(v3i a, v3i b) {
	return (v3i) { a.x * b.x, a.y * b.y, a.z * b.z };
}

v3i v3i_cross(v3i a, v3i b) {
	return make_v3i(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.y * b.z,
			a.x * b.y - a.y * b.x);
}

v3i v3i_normalised(v3i v) {
	const i32 l = v3i_magnitude(v);
	return l == (i32)0 ? (v3i) { (i32)0, (i32)0, (i32)0} : (v3i) { (i32)v.x / l, (i32)v.y / l, (i32)v.z / l};
}

i32 v3i_dot(v3i a, v3i b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

v3i v3i_add(v3i a, v3i b) {
	return (v3i) { a.x + b.x, a.y + b.y, a.z + b.z };
}

v3i v3i_scale(v3i v, i32 s) {
	return (v3i) { v.x * s, v.y * s, v.z * s };
}

v3i v3i_div(v3i a, v3i b) {
	return (v3i) { a.x / b.x, a.y / b.y, a.z / b.z };
}

v3i v3i_sub(v3i a, v3i b) {
	return (v3i) { a.x - b.x, a.y - b.y, a.z - b.z };
}

i32 v4i_magnitude(v4i v) {
	return (i32)sqrt(((i32)v.x * (i32)v.x) + ((i32)v.y * (i32)v.y) + ((i32)v.z * (i32)v.z) + ((i32)v.w * (i32)v.w));
}

v4i make_v4i(i32 x, i32 y, i32 z, i32 w) {
	return (v4i) { x, y, z, w };
}

v4i v4i_mul(v4i a, v4i b) {
	return (v4i) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

v4i v4i_normalised(v4i v) {
	const i32 l = v4i_magnitude(v);
	return l == (i32)0 ? (v4i) { (i32)0, (i32)0, (i32)0, (i32)0} : (v4i) { (i32)v.x / l, (i32)v.y / l, (i32)v.z / l, (i32)v.w / l};
}

i32 v4i_dot(v4i a, v4i b) {
	return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w));
}

v4i v4i_add(v4i a, v4i b) {
	return (v4i) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

v4i v4i_scale(v4i v, i32 s) {
	return (v4i) { v.x * s, v.y * s, v.z * s, v.w * s };
}

v4i v4i_div(v4i a, v4i b) {
	return (v4i) { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w };
}

v4i v4i_sub(v4i a, v4i b) {
	return (v4i) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

m2i m2i_identity() {
	return make_m2i((i32)1);
}

m2i make_m2i(i32 d) {
	m2i r;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			r.m[x][y] = (i32)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;

	return r;
}

m2i m2i_mul(m2i a, m2i b) {
	m2i r = make_m2i((i32)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1];

	return r;
}

m2u m2u_identity() {
	return make_m2u((u32)1);
}

m2u make_m2u(u32 d) {
	m2u r;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			r.m[x][y] = (u32)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;

	return r;
}

m2u m2u_mul(m2u a, m2u b) {
	m2u r = make_m2u((u32)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1];

	return r;
}

m3i m3i_scale(m3i m, v3i v) {
	m3i r = make_m3i((i32)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m3i_mul(m, r);
}

m3i m3i_identity() {
	return make_m3i((i32)1);
}

m3i make_m3i(i32 d) {
	m3i r;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			r.m[x][y] = (i32)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;

	return r;
}

m3i m3i_rotate(m3i m, i32 a, v3i v) {
	m3i r = make_m3i((i32)1);

	const i32 c = (i32)cos((double)a);
	const i32 s = (i32)sin((double)a);

	const i32 omc = (i32)1 - c;

	const i32 x = v.x;
	const i32 y = v.y;
	const i32 z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m3i_mul(m, r);
}

m3i m3i_mul(m3i a, m3i b) {
	m3i r = make_m3i((i32)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1] + a.m[2][0] * b.m[0][2];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1] + a.m[2][0] * b.m[1][2];
	r.m[2][0] = a.m[0][0] * b.m[2][0] + a.m[1][0] * b.m[2][1] + a.m[2][0] * b.m[2][2];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1] + a.m[2][1] * b.m[0][2];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1] + a.m[2][1] * b.m[1][2];
	r.m[2][1] = a.m[0][1] * b.m[2][0] + a.m[1][1] * b.m[2][1] + a.m[2][1] * b.m[2][2];
	r.m[0][2] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[0][1] + a.m[2][2] * b.m[0][2];
	r.m[1][2] = a.m[0][2] * b.m[1][0] + a.m[1][2] * b.m[1][1] + a.m[2][2] * b.m[1][2];
	r.m[2][2] = a.m[0][2] * b.m[2][0] + a.m[1][2] * b.m[2][1] + a.m[2][2] * b.m[2][2];

	return r;
}

m4u m4u_translate(m4u m, v3u v) {
	m4u r = make_m4u((u32)1);

	r.m[3][0] = v.x;
	r.m[3][1] = v.y;
	r.m[3][2] = v.z;

	return m4u_mul(m, r);
}

m4u m4u_scale(m4u m, v3u v) {
	m4u r = make_m4u((u32)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m4u_mul(m, r);
}

m4u m4u_identity() {
	return make_m4u((u32)1);
}

m4u make_m4u(u32 d) {
	m4u r;
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			r.m[x][y] = (u32)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;
	r.m[3][3] = d;

	return r;
}

m4u m4u_rotate(m4u m, u32 a, v3u v) {
	m4u r = make_m4u((u32)1);

	const u32 c = (u32)cos((double)a);
	const u32 s = (u32)sin((double)a);

	const u32 omc = (u32)1 - c;

	const u32 x = v.x;
	const u32 y = v.y;
	const u32 z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m4u_mul(m, r);
}

v4u m4u_transform(m4u m, v4u v) {
	return make_v4u(
		m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] + v.w,
		m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] + v.w,
		m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] + v.w,
		m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] + v.w);
}

m4u m4u_mul(m4u a, m4u b) {
	m4u r = make_m4u((u32)1);

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

m4u m4u_lookat(v3u c, v3u o, v3u u) {
	m4u r = make_m4u((u32)1);

	const v3u f = v3u_normalised(v3u_sub(o, c));
	u = v3u_normalised(u);
	const v3u s = v3u_normalised(v3u_cross(f, u));
	u = v3u_cross(s, f);

	r.m[0][0] = s.x;
	r.m[1][0] = s.y;
	r.m[2][0] = s.z;
	r.m[0][1] = u.x;
	r.m[1][1] = u.y;
	r.m[2][1] = u.z;
	r.m[0][2] = -f.x;
	r.m[1][2] = -f.y;
	r.m[2][2] = -f.z;
	r.m[3][0] = -v3u_dot(s, c);
	r.m[3][1] = -v3u_dot(u, c);
	r.m[3][2] = v3u_dot(f, c);

	return r;
}

m4u m4u_pers(u32 fov, u32 asp, u32 n, u32 f) {
	m4u r = make_m4u((u32)1);

	const u32 q = (u32)1 / (u32)tan(torad((double)fov) / 2.0);
	const u32 a = q / asp;
	const u32 b = (n + f) / (n - f);
	const u32 c = ((u32)2 * n * f) / (n - f);

	r.m[0][0] = a;
	r.m[1][1] = q;
	r.m[2][2] = b;
	r.m[2][3] = (u32)-1;
	r.m[3][2] = c;

	return r;
}

m4u m4u_orth(u32 l, u32 r, u32 b, u32 t, u32 n, u32 f) {
	m4u res = make_m4u((u32)1);

	res.m[0][0] = (u32)2 / (r - l);
	res.m[1][1] = (u32)2 / (t - b);
	res.m[2][2] = (u32)2 / (n - f);

	res.m[3][0] = (l + r) / (l - r);
	res.m[3][1] = (b + t) / (b - t);
	res.m[3][2] = (f + n) / (f - n);

	return res;
}

m4d m4d_translate(m4d m, v3d v) {
	m4d r = make_m4d((double)1);

	r.m[3][0] = v.x;
	r.m[3][1] = v.y;
	r.m[3][2] = v.z;

	return m4d_mul(m, r);
}

m4d m4d_scale(m4d m, v3d v) {
	m4d r = make_m4d((double)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m4d_mul(m, r);
}

m4d m4d_identity() {
	return make_m4d((double)1);
}

m4d make_m4d(double d) {
	m4d r;
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			r.m[x][y] = (double)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;
	r.m[3][3] = d;

	return r;
}

m4d m4d_rotate(m4d m, double a, v3d v) {
	m4d r = make_m4d((double)1);

	const double c = (double)cos((double)a);
	const double s = (double)sin((double)a);

	const double omc = (double)1 - c;

	const double x = v.x;
	const double y = v.y;
	const double z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m4d_mul(m, r);
}

v4d m4d_transform(m4d m, v4d v) {
	return make_v4d(
		m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] + v.w,
		m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] + v.w,
		m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] + v.w,
		m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] + v.w);
}

m4d m4d_mul(m4d a, m4d b) {
	m4d r = make_m4d((double)1);

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

m4d m4d_lookat(v3d c, v3d o, v3d u) {
	m4d r = make_m4d((double)1);

	const v3d f = v3d_normalised(v3d_sub(o, c));
	u = v3d_normalised(u);
	const v3d s = v3d_normalised(v3d_cross(f, u));
	u = v3d_cross(s, f);

	r.m[0][0] = s.x;
	r.m[1][0] = s.y;
	r.m[2][0] = s.z;
	r.m[0][1] = u.x;
	r.m[1][1] = u.y;
	r.m[2][1] = u.z;
	r.m[0][2] = -f.x;
	r.m[1][2] = -f.y;
	r.m[2][2] = -f.z;
	r.m[3][0] = -v3d_dot(s, c);
	r.m[3][1] = -v3d_dot(u, c);
	r.m[3][2] = v3d_dot(f, c);

	return r;
}

m4d m4d_pers(double fov, double asp, double n, double f) {
	m4d r = make_m4d((double)1);

	const double q = (double)1 / (double)tan(torad((double)fov) / 2.0);
	const double a = q / asp;
	const double b = (n + f) / (n - f);
	const double c = ((double)2 * n * f) / (n - f);

	r.m[0][0] = a;
	r.m[1][1] = q;
	r.m[2][2] = b;
	r.m[2][3] = (double)-1;
	r.m[3][2] = c;

	return r;
}

m4d m4d_orth(double l, double r, double b, double t, double n, double f) {
	m4d res = make_m4d((double)1);

	res.m[0][0] = (double)2 / (r - l);
	res.m[1][1] = (double)2 / (t - b);
	res.m[2][2] = (double)2 / (n - f);

	res.m[3][0] = (l + r) / (l - r);
	res.m[3][1] = (b + t) / (b - t);
	res.m[3][2] = (f + n) / (f - n);

	return res;
}

m4i m4i_translate(m4i m, v3i v) {
	m4i r = make_m4i((i32)1);

	r.m[3][0] = v.x;
	r.m[3][1] = v.y;
	r.m[3][2] = v.z;

	return m4i_mul(m, r);
}

m4i m4i_scale(m4i m, v3i v) {
	m4i r = make_m4i((i32)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m4i_mul(m, r);
}

m4i m4i_identity() {
	return make_m4i((i32)1);
}

m4i make_m4i(i32 d) {
	m4i r;
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			r.m[x][y] = (i32)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;
	r.m[3][3] = d;

	return r;
}

m4i m4i_rotate(m4i m, i32 a, v3i v) {
	m4i r = make_m4i((i32)1);

	const i32 c = (i32)cos((double)a);
	const i32 s = (i32)sin((double)a);

	const i32 omc = (i32)1 - c;

	const i32 x = v.x;
	const i32 y = v.y;
	const i32 z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m4i_mul(m, r);
}

v4i m4i_transform(m4i m, v4i v) {
	return make_v4i(
		m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] + v.w,
		m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] + v.w,
		m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] + v.w,
		m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] + v.w);
}

m4i m4i_mul(m4i a, m4i b) {
	m4i r = make_m4i((i32)1);

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

m4i m4i_lookat(v3i c, v3i o, v3i u) {
	m4i r = make_m4i((i32)1);

	const v3i f = v3i_normalised(v3i_sub(o, c));
	u = v3i_normalised(u);
	const v3i s = v3i_normalised(v3i_cross(f, u));
	u = v3i_cross(s, f);

	r.m[0][0] = s.x;
	r.m[1][0] = s.y;
	r.m[2][0] = s.z;
	r.m[0][1] = u.x;
	r.m[1][1] = u.y;
	r.m[2][1] = u.z;
	r.m[0][2] = -f.x;
	r.m[1][2] = -f.y;
	r.m[2][2] = -f.z;
	r.m[3][0] = -v3i_dot(s, c);
	r.m[3][1] = -v3i_dot(u, c);
	r.m[3][2] = v3i_dot(f, c);

	return r;
}

m4i m4i_pers(i32 fov, i32 asp, i32 n, i32 f) {
	m4i r = make_m4i((i32)1);

	const i32 q = (i32)1 / (i32)tan(torad((double)fov) / 2.0);
	const i32 a = q / asp;
	const i32 b = (n + f) / (n - f);
	const i32 c = ((i32)2 * n * f) / (n - f);

	r.m[0][0] = a;
	r.m[1][1] = q;
	r.m[2][2] = b;
	r.m[2][3] = (i32)-1;
	r.m[3][2] = c;

	return r;
}

m4i m4i_orth(i32 l, i32 r, i32 b, i32 t, i32 n, i32 f) {
	m4i res = make_m4i((i32)1);

	res.m[0][0] = (i32)2 / (r - l);
	res.m[1][1] = (i32)2 / (t - b);
	res.m[2][2] = (i32)2 / (n - f);

	res.m[3][0] = (l + r) / (l - r);
	res.m[3][1] = (b + t) / (b - t);
	res.m[3][2] = (f + n) / (f - n);

	return res;
}

m4f m4f_translate(m4f m, v3f v) {
	m4f r = make_m4f((float)1);

	r.m[3][0] = v.x;
	r.m[3][1] = v.y;
	r.m[3][2] = v.z;

	return m4f_mul(m, r);
}

m4f m4f_scale(m4f m, v3f v) {
	m4f r = make_m4f((float)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m4f_mul(m, r);
}

m4f m4f_identity() {
	return make_m4f((float)1);
}

m4f make_m4f(float d) {
	m4f r;
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			r.m[x][y] = (float)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;
	r.m[3][3] = d;

	return r;
}

m4f m4f_rotate(m4f m, float a, v3f v) {
	m4f r = make_m4f((float)1);

	const float c = (float)cos((double)a);
	const float s = (float)sin((double)a);

	const float omc = (float)1 - c;

	const float x = v.x;
	const float y = v.y;
	const float z = v.z;

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

v4f m4f_transform(m4f m, v4f v) {
	return make_v4f(
		m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] + v.w,
		m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] + v.w,
		m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] + v.w,
		m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] + v.w);
}

m4f m4f_mul(m4f a, m4f b) {
	m4f r = make_m4f((float)1);

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

m4f m4f_lookat(v3f c, v3f o, v3f u) {
	m4f r = make_m4f((float)1);

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

m4f m4f_pers(float fov, float asp, float n, float f) {
	m4f r = make_m4f((float)1);

	const float q = (float)1 / (float)tan(torad((double)fov) / 2.0);
	const float a = q / asp;
	const float b = (n + f) / (n - f);
	const float c = ((float)2 * n * f) / (n - f);

	r.m[0][0] = a;
	r.m[1][1] = q;
	r.m[2][2] = b;
	r.m[2][3] = (float)-1;
	r.m[3][2] = c;

	return r;
}

m4f m4f_orth(float l, float r, float b, float t, float n, float f) {
	m4f res = make_m4f((float)1);

	res.m[0][0] = (float)2 / (r - l);
	res.m[1][1] = (float)2 / (t - b);
	res.m[2][2] = (float)2 / (n - f);

	res.m[3][0] = (l + r) / (l - r);
	res.m[3][1] = (b + t) / (b - t);
	res.m[3][2] = (f + n) / (f - n);

	return res;
}

m3u m3u_scale(m3u m, v3u v) {
	m3u r = make_m3u((u32)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m3u_mul(m, r);
}

m3u m3u_identity() {
	return make_m3u((u32)1);
}

m3u make_m3u(u32 d) {
	m3u r;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			r.m[x][y] = (u32)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;

	return r;
}

m3u m3u_rotate(m3u m, u32 a, v3u v) {
	m3u r = make_m3u((u32)1);

	const u32 c = (u32)cos((double)a);
	const u32 s = (u32)sin((double)a);

	const u32 omc = (u32)1 - c;

	const u32 x = v.x;
	const u32 y = v.y;
	const u32 z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m3u_mul(m, r);
}

m3u m3u_mul(m3u a, m3u b) {
	m3u r = make_m3u((u32)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1] + a.m[2][0] * b.m[0][2];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1] + a.m[2][0] * b.m[1][2];
	r.m[2][0] = a.m[0][0] * b.m[2][0] + a.m[1][0] * b.m[2][1] + a.m[2][0] * b.m[2][2];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1] + a.m[2][1] * b.m[0][2];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1] + a.m[2][1] * b.m[1][2];
	r.m[2][1] = a.m[0][1] * b.m[2][0] + a.m[1][1] * b.m[2][1] + a.m[2][1] * b.m[2][2];
	r.m[0][2] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[0][1] + a.m[2][2] * b.m[0][2];
	r.m[1][2] = a.m[0][2] * b.m[1][0] + a.m[1][2] * b.m[1][1] + a.m[2][2] * b.m[1][2];
	r.m[2][2] = a.m[0][2] * b.m[2][0] + a.m[1][2] * b.m[2][1] + a.m[2][2] * b.m[2][2];

	return r;
}

m2d m2d_identity() {
	return make_m2d((double)1);
}

m2d make_m2d(double d) {
	m2d r;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			r.m[x][y] = (double)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;

	return r;
}

m2d m2d_mul(m2d a, m2d b) {
	m2d r = make_m2d((double)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1];

	return r;
}

m3d m3d_scale(m3d m, v3d v) {
	m3d r = make_m3d((double)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m3d_mul(m, r);
}

m3d m3d_identity() {
	return make_m3d((double)1);
}

m3d make_m3d(double d) {
	m3d r;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			r.m[x][y] = (double)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;

	return r;
}

m3d m3d_rotate(m3d m, double a, v3d v) {
	m3d r = make_m3d((double)1);

	const double c = (double)cos((double)a);
	const double s = (double)sin((double)a);

	const double omc = (double)1 - c;

	const double x = v.x;
	const double y = v.y;
	const double z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m3d_mul(m, r);
}

m3d m3d_mul(m3d a, m3d b) {
	m3d r = make_m3d((double)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1] + a.m[2][0] * b.m[0][2];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1] + a.m[2][0] * b.m[1][2];
	r.m[2][0] = a.m[0][0] * b.m[2][0] + a.m[1][0] * b.m[2][1] + a.m[2][0] * b.m[2][2];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1] + a.m[2][1] * b.m[0][2];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1] + a.m[2][1] * b.m[1][2];
	r.m[2][1] = a.m[0][1] * b.m[2][0] + a.m[1][1] * b.m[2][1] + a.m[2][1] * b.m[2][2];
	r.m[0][2] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[0][1] + a.m[2][2] * b.m[0][2];
	r.m[1][2] = a.m[0][2] * b.m[1][0] + a.m[1][2] * b.m[1][1] + a.m[2][2] * b.m[1][2];
	r.m[2][2] = a.m[0][2] * b.m[2][0] + a.m[1][2] * b.m[2][1] + a.m[2][2] * b.m[2][2];

	return r;
}

m2f m2f_identity() {
	return make_m2f((float)1);
}

m2f make_m2f(float d) {
	m2f r;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			r.m[x][y] = (float)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;

	return r;
}

m2f m2f_mul(m2f a, m2f b) {
	m2f r = make_m2f((float)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1];

	return r;
}

m3f m3f_scale(m3f m, v3f v) {
	m3f r = make_m3f((float)1);

	r.m[0][0] = v.x;
	r.m[1][1] = v.y;
	r.m[2][2] = v.z;

	return m3f_mul(m, r);
}

m3f m3f_identity() {
	return make_m3f((float)1);
}

m3f make_m3f(float d) {
	m3f r;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			r.m[x][y] = (float)0;
		}
	}

	r.m[0][0] = d;
	r.m[1][1] = d;
	r.m[2][2] = d;

	return r;
}

m3f m3f_rotate(m3f m, float a, v3f v) {
	m3f r = make_m3f((float)1);

	const float c = (float)cos((double)a);
	const float s = (float)sin((double)a);

	const float omc = (float)1 - c;

	const float x = v.x;
	const float y = v.y;
	const float z = v.z;

	r.m[0][0] = x * x * omc + c;
	r.m[0][1] = y * x * omc + z * s;
	r.m[0][2] = x * z * omc - y * s;
	r.m[1][0] = x * y * omc - z * s;
	r.m[1][1] = y * y * omc + c;
	r.m[1][2] = y * z * omc + x * s;
	r.m[2][0] = x * z * omc + y * s;
	r.m[2][1] = y * z * omc - x * s;
	r.m[2][2] = z * z * omc + c;

	return m3f_mul(m, r);
}

m3f m3f_mul(m3f a, m3f b) {
	m3f r = make_m3f((float)1);

	r.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1] + a.m[2][0] * b.m[0][2];
	r.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1] + a.m[2][0] * b.m[1][2];
	r.m[2][0] = a.m[0][0] * b.m[2][0] + a.m[1][0] * b.m[2][1] + a.m[2][0] * b.m[2][2];
	r.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1] + a.m[2][1] * b.m[0][2];
	r.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1] + a.m[2][1] * b.m[1][2];
	r.m[2][1] = a.m[0][1] * b.m[2][0] + a.m[1][1] * b.m[2][1] + a.m[2][1] * b.m[2][2];
	r.m[0][2] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[0][1] + a.m[2][2] * b.m[0][2];
	r.m[1][2] = a.m[0][2] * b.m[1][0] + a.m[1][2] * b.m[1][1] + a.m[2][2] * b.m[1][2];
	r.m[2][2] = a.m[0][2] * b.m[2][0] + a.m[1][2] * b.m[2][1] + a.m[2][2] * b.m[2][2];

	return r;
}

#endif /* SML_IMPL */

