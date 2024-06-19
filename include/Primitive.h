#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "Helper.h"

typedef struct payload_t {
	color_t interpolated_color;
	uv_t barycentric_coordinate;

	uint_t visible_model_index;
	uint_t visible_model_triangle_index;

	uint_t hitbox_model_index;
	uint_t hitbox_model_triangle_index;
};

typedef struct ray_t{
	vec3_t position;
	vec3_t direction;

	payload_t payload;

	bool has_hit;
};

#endif
