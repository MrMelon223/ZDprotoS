	// Renderin.cpp
#include "../include/Camera.h"
#include "../include/App.h"

bool intersects_ray_triangle(vec3_t& r_pos, vec3_t& r_dir, vec3_t& verta, vec3_t& vertb, vec3_t& vertc, uv_t& uv, float& distance) {
	constexpr float epsilon = std::numeric_limits<float>::epsilon();

	vec3_t edge1, edge2, ray_cross_edge2 = zd::cross(r_dir, edge2);

	edge1.x = vertb.x - verta.x;
	edge1.y = vertb.y - verta.y;
	edge1.z = vertb.z - verta.z;

	edge2.x = vertc.x - verta.x;
	edge2.y = vertc.y - verta.y;
	edge2.z = vertc.z - verta.z;

	float det = zd::dot(edge1, ray_cross_edge2);
	if (det > -epsilon && det < epsilon) {
		return false;	// Ray parallel to tri
	}

	float inv_det = 1.0 / det;
	vec3_t s;

	s.x = r_pos.x - verta.x;
	s.y = r_pos.y - verta.y;
	s.z = r_pos.z - verta.z;

	float u = inv_det * zd::dot(s, ray_cross_edge2);

	if (u < 0.0f || u > 1.0f) {
		return false;
	}

	vec3_t s_cross_edge1 = zd::cross(s, edge1);
	float v = inv_det * zd::dot(r_pos, s_cross_edge1);
	if (v < 0.0f || v > 1.0f) {
		return false;
	}

	float t = inv_det * zd::dot(edge2, s_cross_edge1);
	if (t > epsilon) {
		uv = uv_t{ u,v };
		distance = t;
		return true;
	}
	else {
		return false;
	}
}

void Camera::capture(d_ModelInstance* models, unsigned int model_count) {

	d_Model* d_DEV_MODELS = d_DEVICE_MODELS;
	d_ModelInstance* d_models = sycl::malloc_device<d_ModelInstance>(model_count, *this->queue);

	try {
		this->queue->memcpy(this->d_dims, &this->dims, sizeof(dim_t));
		this->queue->memcpy(this->d_hori_fov, &this->hori_fov, sizeof(float));
		this->queue->memcpy(this->d_cam_position, &this->position, sizeof(vec3_t));
		this->queue->memcpy(this->d_cam_direction, &this->direction, sizeof(vec3_t));
		this->queue->memcpy(d_models, models, sizeof(d_ModelInstance) * model_count);
		this->queue->wait();
	}
	catch (sycl::exception& e) {
		std::cerr << "UPLOAD_TO_GPU::ERROR: " << e.what() << std::endl;
	}

	ray_t* rays = this->rays;
	dim_t* dims = d_dims;
	float* hori_fov = d_hori_fov;
	vec3_t* direction = d_cam_direction;
	vec3_t* position = d_cam_position;
	try {
		this->queue->submit([&](sycl::handler& h) {
			h.parallel_for<class ray_setup>(sycl::range<1>(this->dims.y * this->dims.x), [=](sycl::id<1> idx) {
				int index = idx[0];

				ray_t* ray = &rays[index];

				int x = index % dims->x,
					y = (index - x) / dims->x;

				float ratio = static_cast<float>(dims->x) / static_cast<float>(dims->y);
				float norm_x = (x - (static_cast<float>(dims->x) * 0.5f)) / (static_cast<float>(dims->x) * 0.5f);
				float norm_y = (y - (static_cast<float>(dims->y) * 0.5f)) / (static_cast<float>(dims->y) * 0.5f);
				float fov_rad = *hori_fov * (PI / 180.0f);
				float half_fov = fov_rad * 0.5f;

				vec3_t upward{ 0.0f, 1.0f, 0.0f };

				vec3_t right = zd::cross(*direction, upward);

				vec3_t up = zd::cross(right, *direction);

				ray->direction.x = direction->x + norm_x * half_fov * ratio * right.x + norm_y * half_fov * up.x;
				ray->direction.y = direction->y + norm_x * half_fov * ratio * right.y + norm_y * half_fov * up.y;
				ray->direction.z = direction->z + norm_x * half_fov * ratio * right.z + norm_y * half_fov * up.z;
			});
		});

		this->queue->wait();
	}
	catch (sycl::exception& e) {
		std::cerr << "SETUP_RAY::ERROR: " << e.what() << std::endl;
	}

	unsigned int* d_mod_count = (unsigned int*)sycl::malloc_device(sizeof(unsigned int), *this->queue);
	try {
		this->queue->memcpy(d_mod_count, &model_count, sizeof(unsigned int));
		this->queue->wait();
	}
	catch (sycl::exception& e) {
		std::cerr << "MODEL_COUNT::COPY_TO_GPU::ERROR: " << e.what() << std::endl;
	}

	try {
		this->queue->submit([&](sycl::handler& h) {
			h.parallel_for<class ray_capture>(sycl::range<1>(this->dims.y * this->dims.x), [=](sycl::id<1> idx) {
				int index = idx[0];

				ray_t* ray = &rays[index];

				int x = index % dims->x,
					y = (index - x) / dims->x;
				float closest = 100.0f;
				tri_t* intersected;

				for (unsigned int i = 0; i < *d_mod_count; i++) {
					d_Model* model = &d_DEV_MODELS[d_models[i].model_index];

					for (unsigned int j = 0; j < *model->triangle_count; j++) {

						tri_t* tri = &model->triangle_indices[j];
						uv_t uv;
						float distance;

						if (intersects_ray_triangle(ray->position, ray->direction, model->vertex_positions[tri->a], model->vertex_positions[tri->b], model->vertex_positions[tri->c], uv, distance)) {
							if (distance < closest) {
								ray->payload = { color_t{1.0f, 1.0f, 1.0f, 1.0f}, uv, i, j, 0, 0 };
								ray->has_hit = true;
							}
							else {
								ray->has_hit = false;
							}
						}
					}
				}
				});
			});

		this->queue->wait();

	}
	catch (sycl::exception& e) {
		std::cerr << "RAY_CAPTURE::ERROR: " << e.what() << std::endl;
	}
	sycl::free(d_mod_count, *this->queue);
}