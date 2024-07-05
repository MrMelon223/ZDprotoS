	// Helper.cpp
#include "../include/Helper.h"

vec3_t zd::cross(vec3_t& a, vec3_t& b) {
	vec3_t r{};

	r.x = a.y * b.z - b.y * a.z;
	r.y = a.x * b.z - b.x * a.z;
	r.z = a.x * b.y - b.x * a.y;

	return r;
}

vec3_t zd::subtract(vec3_t& a, vec3_t& b) {
	vec3_t r{};

	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;

	return r;
}

vec3_t zd::add(vec3_t& a, vec3_t& b) {
	vec3_t r{};

	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;

	return r;
}

vec3_t zd::normalize(vec3_t& v) {
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