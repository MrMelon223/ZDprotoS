#ifndef CAMERA_H
#define CAMERA_H

#include "Primitive.h"
#include "Model.h"

class Camera {
protected:
	sycl::queue* queue;
	dim_t dims;
	ray_t* rays;
	
public:
	Camera(int, int, sycl::queue*);

	void resize(int, int);

	dim_t get_dims() { return this->dims; }
	ray_t* get_rays_ptr() { return this->rays; }
};

#endif
