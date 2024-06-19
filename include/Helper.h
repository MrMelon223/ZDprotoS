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
	// GLM Vector Types
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
	// Window API
#include <GLFW/glfw3.h>
	// GPU/CPU Compute API
#include <sycl/sycl.hpp>

typedef glm::vec3 vec3_t;

typedef glm::vec2 vec2_t;
typedef vec2_t uv_t;

typedef glm::vec4 color_t;

typedef glm::ivec2 dim_t;

typedef struct tri_t {
	unsigned int a, b, c;
};

typedef unsigned int uint_t;

#endif
