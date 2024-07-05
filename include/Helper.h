#ifndef HELPER_H
#define HELPER_H

// Standard Libraries
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
	// Window API
#include <GLFW/glfw3.h>
	// GPU/CPU Compute API
#include <sycl/sycl.hpp>

typedef struct vec3_t {
	float x, y, z;
};

typedef struct vec2_t {
	float x, y;
};
typedef vec2_t uv_t;

typedef struct color_t {
	float x, y, z, w;
};

typedef struct dim_t {
	int x, y;
};

typedef struct tri_t {
	unsigned int a, b, c;
};

typedef unsigned int uint_t;

const float PI = 3.1415f;

namespace zd {

	extern SYCL_EXTERNAL vec3_t cross(vec3_t&, vec3_t&);

	extern inline SYCL_EXTERNAL float dot(vec3_t a, vec3_t b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	extern SYCL_EXTERNAL vec3_t add(vec3_t&, vec3_t&);
	extern SYCL_EXTERNAL vec3_t subtract(vec3_t&, vec3_t&);

	extern SYCL_EXTERNAL vec3_t normalize(vec3_t&);

	extern SYCL_EXTERNAL bool intersects_ray_triangle(vec3_t&, vec3_t&, vec3_t&, vec3_t&, vec3_t&, uv_t&, float&);
}

#endif
