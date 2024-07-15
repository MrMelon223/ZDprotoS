	// Camera.cpp
#include "../include/Camera.h"

using namespace sycl;

class copy_render_buff_out;

Camera::Camera(int width, int height, sycl::queue* q) {
	this->queue = q;
	this->dims = dim_t{ width, height };

	this->rays = malloc_device<ray_t>(this->dims.y * this->dims.x, *q);

	this->hori_fov = 90.0f;

	dim_t* current_dims = malloc_device<dim_t>(1, *this->queue);
	float* horizontal_fov = malloc_device<float>(1, *this->queue);

	this->d_cam_position = malloc_device<vec3_t>(1, *this->queue);
	this->d_cam_direction = malloc_device<vec3_t>(1, *this->queue);
	this->d_dims = malloc_device<dim_t>(1, *this->queue);
	this->d_hori_fov = malloc_device<float>(1, *this->queue);
}

void Camera::resize(int width, int height) {
	free(this->rays, *this->queue);

	this->dims = dim_t{ width, height };

	this->rays = malloc_device<ray_t>(this->dims.y * this->dims.x, *this->queue);
}

void Camera::copy_data_out(Window* win) {
	color_t* buff = win->get_frame_buffer_ptr();

	dim_t* dims = &win->dims;

	ray_t* r = this->rays;

	try {
		this->queue->submit([&](sycl::handler& h) {
			h.parallel_for<class copy_render_buff_out>(sycl::range<1>(this->dims.y * this->dims.x), [=](sycl::id<1> idx) {
				int index = idx[0];

				ray_t* ray = &r[index];

				color_t* buff = win->get_frame_buffer_ptr();
				if (ray->has_hit) {

					buff[index] = color_t{ 1.0f, 1.0f, 1.0f, 1.0f };
				}
				else {
					buff[index] = color_t{ 0.0f, 0.0f, 0.0f, 0.0f };
				}
				});
			});
		this->queue->wait();
	}
	catch (sycl::exception& e) {
		std::cerr << "COPY_BUFFER_FROM_RAYS::ERROR: " << e.what() << std::endl;
	}
}