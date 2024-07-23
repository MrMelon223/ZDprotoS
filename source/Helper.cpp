	// Helper.cpp
#include "../include/Helper.h"

vec3_t zd::cross(vec3_t& a, vec3_t& b) {
	vec3_t r;

	r.x = a.y * b.z - b.y * a.z;
	r.y = a.x * b.z - b.x * a.z;
	r.z = a.x * b.y - b.x * a.y;

	return r;
}

vec3_t zd::normalize(vec3_t v) {
	float mag = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	vec3_t r{};

	r.x = v.x;
	r.y = v.y;
	r.z = v.z;

	r.x = r.x / mag;
	r.y = r.y / mag;
	r.z = r.z / mag;

	return r;
}

vec3_t zd::scale(vec3_t v, float s) {
	v.x *= s;
	v.y *= s;
	v.z *= s;

	return v;
}

mat3_t zd::rotate(mat4_t m, float angle, vec3_t v) {
	float const a = angle;
	float const c = cos(a);
	float const s = sin(a);

	vec3_t axis = zd::normalize(v);
	vec3_t temp = zd::scale(axis, (1.0f - c));

	mat3_t Rotate = mat3_t();
	Rotate[0][0] = c + temp.x * axis.x;
	Rotate[0][1] = temp.x * axis.y + s * axis.z;
	Rotate[0][2] = temp.x * axis.z - s * axis.y;

	Rotate[1][0] = temp.y * axis.x - s * axis.z;
	Rotate[1][1] = c + temp.y * axis.y;
	Rotate[1][2] = temp.y * axis.z + s * axis.x;

	Rotate[2][0] = temp.z * axis.x + s * axis.y;
	Rotate[2][1] = temp.z * axis.y - s * axis.x;
	Rotate[2][2] = c + temp.z * axis.z;

	mat3_t Result = mat3_t(1.0f);
	Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
	Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
	Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
	return Result;
}

bool zd::intersects_triangle(vec3_t orig, vec3_t dir, vec3_t vert0, vec3_t vert1, vec3_t vert2, uv_t& baryPosition, float& distance) {
	// find vectors for two edges sharing vert0
	vec3_t edge1 = vert1 - vert0;
	vec3_t edge2 = vert2 - vert0;

	// begin calculating determinant - also used to calculate U parameter
	vec3_t const p = zd::cross(dir, edge2);

	// if determinant is near zero, ray lies in plane of triangle
	float const det = zd::dot(edge1, p);

	vec3_t Perpendicular(0.0f, 0.0f, 0.0f);

	if (det > 0.0f)
	{
		// calculate distance from vert0 to ray origin
		vec3_t dist = orig - vert0;

		// calculate U parameter and test bounds
		baryPosition.x = zd::dot(dist, p);
		if (baryPosition.x < 0.0f || baryPosition.x > det)
			return false;

		// prepare to test V parameter
		Perpendicular = zd::cross(dist, edge1);

		// calculate V parameter and test bounds
		baryPosition.y = zd::dot(dir, Perpendicular);
		if ((baryPosition.y < 0.0f) || ((baryPosition.x + baryPosition.y) > det))
			return false;
	}
	else if (det < 0.0f)
	{
		// calculate distance from vert0 to ray origin
		vec3_t dist = orig - vert0;

		// calculate U parameter and test bounds
		baryPosition.x = zd::dot(dist, p);
		if ((baryPosition.x > 0.0f) || (baryPosition.x < det))
			return false;

		// prepare to test V parameter
		Perpendicular = zd::cross(dist, edge1);

		// calculate V parameter and test bounds
		baryPosition.y = zd::dot(dir, Perpendicular);
		if ((baryPosition.y > 0.0f) || (baryPosition.x + baryPosition.y < det))
			return false;
	}
	else
		return false; // ray is parallel to the plane of the triangle

	float inv_det = 1.0f / det;

	// calculate distance, ray intersects triangle
	distance = zd::dot(edge2, Perpendicular) * inv_det;
	baryPosition *= inv_det;

	return true;
}

vec3_t zd::max(vec3_t a, vec3_t b) {
	vec3_t r;
	if (a.x < b.x) {
		r.x = b.x;
	}
	else {
		r.x = a.x;
	}

	if (a.y < b.y) {
		r.y = b.y;
	}
	else {
		r.y = a.y;
	}

	if (a.z < b.z) {
		r.z = b.z;
	}
	else {
		r.z = a.z;
	}

	return r;
}

vec3_t zd::min(vec3_t a, vec3_t b) {
	vec3_t r;
	if (a.x > b.x) {
		r.x = b.x;
	}
	else {
		r.x = a.x;
	}

	if (a.y > b.y) {
		r.y = b.y;
	}
	else {
		r.y = a.y;
	}

	if (a.z > b.z) {
		r.z = b.z;
	}
	else {
		r.z = a.z;
	}

	return r;
}

float zd::maxf(float a, float b) { if (a > b) { return a; } else { return b; } }
float zd::minf(float a, float b) { if (a < b) { return a; } else { return b; } }

extern float zd::length(vec3_t v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }