#ifndef HELPER_H
#define HELPER_H

#pragma comment(lib, "opengl32.lib")

// Standard Libraries
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <immintrin.h>
	// Window API
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp>
	// GPU/CPU Compute API
#include <sycl/sycl.hpp>
#include <ipp.h>
#include "oneapi/mkl.hpp"

typedef glm::vec3 vec3_t;

typedef glm::vec2 vec2_t;
typedef vec2_t uv_t;

typedef glm::vec4 color_t;

typedef glm::mat3 mat3_t;

typedef glm::mat4 mat4_t;

typedef glm::ivec2 dim_t;

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


	extern SYCL_EXTERNAL vec3_t normalize(vec3_t&);

	extern SYCL_EXTERNAL vec3_t scale(vec3_t, float);

}

#endif
