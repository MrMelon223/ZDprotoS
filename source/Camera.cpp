	// Camera.cpp
#include "../include/Camera.h"

Camera::Camera(int width, int height, sycl::queue* q) {
	this->queue = q;
	this->dims = dim_t{ width, height };

	this->rays = malloc_device<ray_t>(this->dims.y * this->dims.x, *q);

}

void Camera::resize(int width, int height) {
	free(this->rays, *this->queue);

	this->dims = dim_t{ width, height };

	this->rays = malloc_device<ray_t>(this->dims.y * this->dims.x, *this->queue);
}