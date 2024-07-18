	// Renderin.cpp
#include "../include/Camera.h"
#include "../include/App.h"

class ray_setup;
class ray_capture;

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
	dim_t* dims = this->d_dims;
	float* hori_fov = this->d_hori_fov;
	vec3_t* direction = this->d_cam_direction;
	vec3_t* position = this->d_cam_position;
	try {
		this->queue->submit([&](sycl::handler& h) {
			h.parallel_for<class ray_setup>(sycl::nd_range<1>{this->dims.y * this->dims.x, 1}, [=](sycl::nd_item<1> idx) {
				int index = idx.get_global_id(0);

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
				up = zd::normalize(up);
				*direction = zd::normalize(*direction);
				right = zd::normalize(right);

				ray->direction.x = direction->x + norm_x * half_fov * ratio * right.x + norm_y * half_fov * up.x;
				ray->direction.y = direction->y + norm_x * half_fov * ratio * right.y + norm_y * half_fov * up.y;
				ray->direction.z = direction->z + norm_x * half_fov * ratio * right.z + norm_y * half_fov * up.z;

				ray->position = *position;
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
			h.parallel_for<class ray_capture>(sycl::nd_range<1>{this->dims.y * this->dims.x, 1}, [=](sycl::nd_item<1> idx) {
				int index = idx.get_global_id(0);

				ray_t* ray = &rays[index];

				int x = index % dims->x,
					y = (index - x) / dims->x;
				float closest = 1000.0f;
				tri_t* intersected;
				bool tried = false, intersect = false;

				for (unsigned int i = 0; i < *d_mod_count; i++) {
					d_Model* model = &d_DEV_MODELS[d_models[i].model_index];
					if (d_models[i].show) {
						vec3_t rot = d_models[i].rotation;

						mat4_t rot_x = glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1.0f, 0.0f, 0.0f)),
							rot_y = glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0.0f, 1.0f, 0.0f)),
							rot_z = glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0.0f, 0.0f, 1.0f)),
							rotate = rot_x * rot_y;

						for (unsigned int j = 0; j < *model->triangle_count; j++) {

							tri_t* tri = &model->triangle_indices[j];
							vec3_t tri_norm = model->triangle_normals[j];
							uv_t uv;
							float distance;
							float scale = d_models[i].scale;

							vec3_t offset = d_models[i].position;

							vec3_t verta = model->vertex_positions[tri->a];
							vec3_t vertb = model->vertex_positions[tri->b];
							vec3_t vertc = model->vertex_positions[tri->c];

							verta = ((rotate * glm::vec4(verta, 0.0f)) + glm::vec4(offset, 0.0f))* scale;

							vertb = ((rotate * glm::vec4(vertb, 0.0f)) + glm::vec4(offset, 0.0f)) * scale;

							vertc = ((rotate * glm::vec4(vertc, 0.0f)) + glm::vec4(offset, 0.0f)) * scale;

							// HERE !!!!

							bool test = glm::intersectRayTriangle(ray->position, ray->direction, verta, vertb, vertc, uv, distance);

							if (test) {
								if (distance < closest) {
									ray->payload = payload_t{ distance, color_t{1.0f, 1.0f, 1.0f, 1.0f}, uv, i, j, 0, 0, tri_norm };
									intersect = true;
								}
								else {
								}
							}
							tried = true;
						}
					}
				}
				if (tried && intersect) {
					ray->has_hit = true;
				}
				else {
					ray->has_hit = false;
				}
			});
		});

		this->queue->wait();

	}
	catch (sycl::exception& e) {
		std::cerr << "RAY_CAPTURE::ERROR: " << e.what() << std::endl;
	}
	sycl::free(d_models, *this->queue);
	sycl::free(d_mod_count, *this->queue);
}