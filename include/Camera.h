#ifndef CAMERA_H
#define CAMERA_H

#include "Primitive.h"
#include "Model.h"
#include "Window.h"

class Camera {
protected:
	sycl::queue* queue;
	dim_t dims;
	ray_t* rays;
	
	float hori_fov;

	vec3_t position;
	vec2_t rotation;
	vec3_t direction;

	vec3_t* d_cam_position;
	vec3_t* d_cam_direction;
	dim_t* d_dims;
	float* d_hori_fov;

public:
	Camera(int, int, sycl::queue*);

	void resize(int, int);

	dim_t get_dims() { return this->dims; }
	ray_t* get_rays_ptr() { return this->rays; }

	void capture(d_ModelInstance*, unsigned int);

	void copy_data_out(Window*);
};

#endif
