#ifndef LIGHT_H
#define LIGHT_H

#include "Helper.h"

typedef struct ambient_light_t {
	color_t diffuse_color;
	color_t specular_color;
	float intensity;
};

typedef struct point_light_t {
	vec3_t position;
	color_t diffuse_color;
	color_t specular_color;
	float intensity;
	float falloff;
	float range;
};

#endif
