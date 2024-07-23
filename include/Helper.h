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

typedef glm::vec2 vec2_t;
typedef vec2_t uv_t;


typedef glm::mat3 mat3_t;

struct vec3_t {
    float x, y, z;

    vec3_t() {
        this->x = 0.0f;
        this->y = 0.0f;
        this->z = 0.0f;
    }

    vec3_t(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    SYCL_EXTERNAL vec3_t operator * (float& f) {
        return vec3_t{ f * this->x, f * this->y, f * this->z };
    }

    SYCL_EXTERNAL vec3_t operator -(vec3_t other) {
        return vec3_t{ this->x - other.x, this->y - other.y, this->z - other.z};
    }

    SYCL_EXTERNAL vec3_t operator +(vec3_t other) {
        return vec3_t{ this->x + other.x, this->y + other.y, this->z + other.z };
    }

    SYCL_EXTERNAL vec3_t operator *(mat3_t& m) {
        return vec3_t(
            x * m[0][0] + y * m[1][0] + z * m[2][0],
            x * m[0][1] + y * m[1][1] + z * m[2][1],
            x * m[0][2] + y * m[1][2] + z * m[2][2]);
    }

};

typedef glm::mat4 mat4_t;


struct color_t {
    float x, y, z, w;

    color_t() {
        this->x = this->y = this->z = this->w = 0.0f;
    }

    color_t(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w) {}

    color_t(vec3_t v, float w)
        : x(v.x), y(v.y), z(v.z), w(w) {}

    SYCL_EXTERNAL color_t operator +(color_t& other) {
        return color_t{ this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w };
    }

    SYCL_EXTERNAL color_t operator *(mat4_t& m) {
        return color_t(
            x * m[0][0] + y * m[1][0] + z * m[2][0] + w * m[3][0],
            x * m[0][1] + y * m[1][1] + z * m[2][1] + w * m[3][1],
            x * m[0][2] + y * m[1][2] + z * m[2][2] + w * m[3][2],
            x * m[0][3] + y * m[1][3] + z * m[2][3] + w * m[3][3]
        );
    }

    SYCL_EXTERNAL color_t operator * (float& f) {
        return color_t{ f * this->x, f * this->y, f * this->z, f * this->w };
    }
};

typedef glm::ivec2 dim_t;

typedef struct tri_t {
	unsigned int a, b, c;
};

typedef unsigned int uint_t;

const float PI = 3.1415f;

namespace zd {

	extern "C" SYCL_EXTERNAL vec3_t cross(vec3_t&, vec3_t&);

	extern "C" inline SYCL_EXTERNAL float dot(vec3_t a, vec3_t b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	extern "C" SYCL_EXTERNAL mat3_t rotate(mat4_t, float, vec3_t);

	extern "C" SYCL_EXTERNAL vec3_t normalize(vec3_t);

	extern "C" SYCL_EXTERNAL vec3_t scale(vec3_t, float);

	extern "C" SYCL_EXTERNAL bool intersects_triangle(vec3_t, vec3_t, vec3_t, vec3_t, vec3_t, uv_t&, float&);

    extern "C" SYCL_EXTERNAL float maxf(float a, float b);
    extern "C" SYCL_EXTERNAL float minf(float a, float b);

    extern "C" SYCL_EXTERNAL vec3_t max(vec3_t, vec3_t);
    extern "C" SYCL_EXTERNAL vec3_t min(vec3_t, vec3_t);

    extern "C" SYCL_EXTERNAL float length(vec3_t v);
}

#endif
