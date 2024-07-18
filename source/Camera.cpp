	// Camera.cpp
#include "../include/Camera.h"

using namespace sycl;

class copy_render_buff_out;
class shade_basic;

Camera::Camera(int width, int height, sycl::queue* q) {
	this->queue = q;
	this->dims = dim_t{ width, height };

	this->rays = malloc_device<ray_t>(this->dims.y * this->dims.x, *q);

	this->hori_fov = 90.0f;

	this->render_buffer = malloc_device<color_t>(this->dims.y * this->dims.x, *this->queue);

	dim_t* current_dims = malloc_device<dim_t>(1, *this->queue);
	float* horizontal_fov = malloc_device<float>(1, *this->queue);

	this->d_cam_position = malloc_device<vec3_t>(1, *this->queue);
	this->d_cam_direction = malloc_device<vec3_t>(1, *this->queue);
	this->d_dims = malloc_device<dim_t>(1, *this->queue);
	this->d_hori_fov = malloc_device<float>(1, *this->queue);

	this->queue->memcpy(this->d_hori_fov, &this->hori_fov, sizeof(float));
	this->queue->wait();

	this->position = vec3_t{ 0.0f, 0.0f, 0.0f };
	this->rotation = vec2_t{ 0.0f, 0.0f };
}

void Camera::resize(int width, int height) {
	free(this->rays, *this->queue);
	free(this->render_buffer, *this->queue);

	this->dims = dim_t{ width, height };

	this->rays = malloc_device<ray_t>(this->dims.y * this->dims.x, *this->queue);
	this->render_buffer = malloc_device<color_t>(this->dims.y * this->dims.x, *this->queue);
}

void Camera::shade(ambient_light_t* ambient_light, point_light_t* point_lights, unsigned int point_light_count) {

	dim_t* dims = &this->dims;

	ray_t* r = this->rays;
	color_t* render_buff = this->render_buffer;
	unsigned int* d_point_light_count = malloc_device<unsigned int>(1, *this->queue);
	ambient_light_t* d_ambient_light = malloc_device<ambient_light_t>(1, *this->queue);

	this->queue->memcpy(d_point_light_count, &point_light_count, sizeof(unsigned int));
	this->queue->memcpy(d_ambient_light, ambient_light, sizeof(ambient_light_t));
	this->queue->wait();

	try {
		this->queue->submit([&](sycl::handler& h) {
			h.parallel_for<class shade_basic>(sycl::nd_range<1>(this->dims.y * this->dims.x, 1), [=](sycl::nd_item<1> idx) {
				int index = idx.get_global_id(0);

				ray_t* ray = &r[index];

				color_t* pixel = &render_buff[index];
				*pixel = color_t{ 0.0f, 0.0f, 0.0f, 0.0f };

				if (ray->has_hit) {
					color_t result = color_t(0.0f, 0.0f, 0.0f, 0.0f);
					result = result + d_ambient_light->diffuse_color;
					for (unsigned int i = 0; i < *d_point_light_count; i++) {
						vec3_t distance = ray->direction;
						distance.x *= ray->payload.distance;
						distance.y *= ray->payload.distance;
						distance.z *= ray->payload.distance;

						vec3_t light_direction = point_lights[i].position - (ray->position + distance);
						float magnitude = sqrtf(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);

						if (magnitude <= point_lights[i].range) {
							float diff = zd::dot(ray->payload.triangle_normal, light_direction);
							if (diff <= 0.0f) {
								float intensity = point_lights[i].intensity / (magnitude * magnitude);
								color_t diffuse;

								diffuse.x = intensity * point_lights[i].diffuse_color.x;
								diffuse.y = intensity * point_lights[i].diffuse_color.y;
								diffuse.z = intensity * point_lights[i].diffuse_color.z;
								diffuse.w = 0.0f;

								result = result + diffuse;
							}

							render_buff[index] = result;
						}
					}
					render_buff[index].w = 1.0f;
				}
				else {
					render_buff[index] = color_t{ 0.32f, 0.32f, 0.1f, 1.0f };
				}
			});
		});
		this->queue->wait();
	}
	catch (sycl::exception& e) {
		std::cerr << "SHADE_AFTER_CAPTURE::ERROR: " << e.what() << std::endl;
	}
}

void Camera::copy_data_out(Window* win) {
	color_t* buff = win->get_frame_buffer_ptr();

	dim_t* dims = &win->dims;

	ray_t* r = this->rays;
	color_t* render_buff = this->render_buffer;

	try {
		this->queue->memcpy(buff, render_buff, sizeof(color_t) * this->dims.y * this->dims.x);
		/*this->queue->submit([&](sycl::handler& h) {
			h.parallel_for<class copy_render_buff_out>(sycl::nd_range<1>(this->dims.y * this->dims.x, 1), [=](sycl::nd_item<1> idx) {
				int index = idx.get_global_id(0);

				ray_t* ray = &r[index];

				buff[index] = render_buff[index];
			});
		});*/
		this->queue->wait();
	}
	catch (sycl::exception& e) {
		std::cerr << "COPY_BUFFER_FROM_RAYS::ERROR: " << e.what() << std::endl;
	}
}

void Camera::forward(float t) {

	this->direction = zd::normalize(this->direction);

	this->position.x += this->direction.x * t;
	this->position.y += this->direction.y * t;
	this->position.z += this->direction.z * t;
}

void Camera::backward(float t) {

	this->direction = zd::normalize(this->direction);

	this->position.x -= this->direction.x * t;
	this->position.y -= this->direction.y * t;
	this->position.z -= this->direction.z * t;
}

void Camera::right(float t) {
	this->direction = zd::normalize(this->direction);
	vec3_t up{ 0.0f, 1.0f, 0.0f };

	vec3_t cross = zd::cross(this->direction, up);

	this->position.x += t * cross.x;
	this->position.y += t * cross.y;
	this->position.z += t * cross.z;
}

void Camera::left(float t) {
	this->direction = zd::normalize(this->direction);
	vec3_t up{ 0.0f, 1.0f, 0.0f };

	vec3_t cross = zd::cross(this->direction, up);

	this->position.x -= t * cross.x;
	this->position.y -= t * cross.y;
	this->position.z -= t * cross.z;
}

void Camera::update_direction(float x, float y) {
	//printf("X,Y input mouse coord = {%.2f, %.2f}\n", rot.x, rot.y);
	float normalized_coord_x = ((x - (static_cast<float>(this->dims.x) * 0.5f)) / static_cast<float>(this->dims.x));
	float normalized_coord_y = ((y - (static_cast<float>(this->dims.y) * 0.5f)) / static_cast<float>(this->dims.y));
	//printf("X,Y normalized input mouse coord = {%.2f, %.2f}\n", normalized_coord_x, normalized_coord_y);

	float aspect_ratio = static_cast<float>(this->dims.x) / static_cast<float>(this->dims.y);

	float fov_hori_rad = this->hori_fov;
	float fov_vert_rad = this->hori_fov * (static_cast<float>(this->dims.y) / static_cast<float>(this->dims.x));
	float half_fov_hori_rad = fov_hori_rad * 0.5f;
	float half_fov_vert_rad = fov_vert_rad * 0.5f;

	float view_x = normalized_coord_x * half_fov_hori_rad * aspect_ratio;
	float view_y = normalized_coord_y * half_fov_vert_rad;

	this->rotation.x += view_x * 3.0f * aspect_ratio; //* (static_cast<float>(this->dims.x) / this->dims.y);
	this->rotation.y -= view_y * 3.0f;
	//this->rotation.z = 0.0f;

	if (this->rotation.y > 88.0f) {
		this->rotation.y = 88.0f;
	}
	if (this->rotation.y < -88.0f) {
		this->rotation.y = -88.0f;
	}

	float yaw = this->rotation.x * (PI / 180.0f),
		pitch = this->rotation.y * (PI / 180.0f);

	this->direction.x = cosf(yaw) * cosf(pitch);
	this->direction.y = sinf(pitch);
	this->direction.z = sinf(yaw) * cosf(pitch);

	this->direction = zd::normalize(this->direction);
}

void Camera::debug_print() {
	std::cout << "Camera Debug:" << std::endl;
	std::cout << std::setw(15) << "Res: " << this->dims.x << "x" << this->dims.y << "p" << std::endl;
	std::cout << std::setw(15) << "Loc: { " << this->position.x << ", " << this->position.y << ", " << this->position.z << " }" << std::endl;
	std::cout << std::setw(15) << "Dir: { " << this->direction.x << ", " << this->direction.y << ", " << this->direction.z << " }" << std::endl;
}