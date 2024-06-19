	// Model.cpp
#include "../include/Model.h"

void glModel::load_from(std::string path) {
	this->filepath = path;
	//this->name = extract_name(this->filepath);

	std::ifstream in;
	in.open(this->filepath, std::ios_base::in);

	if (!in) {
		std::cout << "Cannot find model " << path << std::endl;
		return;
	}

	size_t len;
	std::string line2;
	std::getline(in, line2);
	std::istringstream in_s(line2);
	in_s >> len;
	for (int i = 0; i < len; i++) {

		std::string type;
		std::getline(in, type);
		std::istringstream in2(type);

		float x, y, z;
		float r, g, b, a;
		float u, v;
		in2 >> x >> y >> z;
		in2 >> r >> g >> b >> a;
		in2 >> u >> v;

		this->vertex_positions.push_back(vec3_t(x, y, z));
		this->vertex_normals.push_back(glm::normalize(vec3_t(x, y, z)));
		this->vertex_colors.push_back(color_t(r, g, b, a));
		this->vertex_uvs.push_back(uv_t(u, v));
	}
	std::string line;
	std::getline(in, line);
	std::istringstream in_s2(line);
	in_s2 >> len;

	for (int i = 0; i < len; i++) {
		std::string type;
		std::getline(in, type);
		std::istringstream in3(type);

		uint32_t x, y, z;
		in3 >> x >> y >> z;

		//std::cout << "Triangle idxs = { " << x << ", " << y << ", " << z << " }" << std::endl;

		vec3_t va = this->vertex_positions[x],
			vb = this->vertex_positions[y],
			vc = this->vertex_positions[z];

		glm::vec3 t_norm = glm::cross(va - vb, va - vc),
			norm_w = (this->vertex_normals[x] + this->vertex_normals[y] + this->vertex_normals[z]) / 3.0f;
		if (glm::dot(t_norm, norm_w) < 0.0f) {
			t_norm = -1.0f * t_norm;
		}

		this->triangle_indices.push_back(tri_t{ x, y, z });
		this->triangle_normals.push_back(t_norm);
	}

	std::string shader_name;
	std::getline(in, line);
	std::istringstream in_s3(line);
	in_s3 >> shader_name;

	//this->shader_program = load_shader_program(shader_name);


	std::cout << std::setw(10) << this->vertex_positions.size() << " Vertices loaded" << std::endl;
	std::cout << std::setw(10) << this->triangle_indices.size() << " Triangles loaded" << std::endl;

	in.close();
}

glModel::glModel(std::string filepath, std::string name) {
	this->load_from(filepath);
	this->name = name;
}

using namespace sycl;

d_Model glModel::to_gpu(sycl::queue* q) {
	d_Model ret{};

	ret.queue = q;

	ret.vertex_positions = malloc_device<vec3_t>(this->vertex_positions.size(), *q);
	ret.vertex_normals = malloc_device<vec3_t>(this->vertex_normals.size(), *q);
	ret.vertex_colors = malloc_device<color_t>(this->vertex_colors.size(), *q);
	ret.vertex_uvs = malloc_device<uv_t>(this->vertex_uvs.size(), *q);
	ret.vertex_count = malloc_device<unsigned int>(1, *q);

	ret.triangle_indices = malloc_device<tri_t>(this->triangle_indices.size(), *q);
	ret.triangle_normals = malloc_device<vec3_t>(this->triangle_normals.size(), *q);
	ret.triangle_count = malloc_device<unsigned int>(1, *q);

	q->memcpy(ret.vertex_positions, this->vertex_positions.data(), sizeof(vec3_t) * this->vertex_positions.size());
	q->memcpy(ret.vertex_normals, this->vertex_normals.data(), sizeof(vec3_t) * this->vertex_normals.size());
	q->memcpy(ret.vertex_colors, this->vertex_colors.data(), sizeof(color_t) * this->vertex_colors.size());
	q->memcpy(ret.vertex_uvs, this->vertex_uvs.data(), sizeof(uv_t) * this->vertex_uvs.size());

	q->memcpy(ret.triangle_indices, this->triangle_indices.data(), sizeof(tri_t) * this->triangle_indices.size());
	q->memcpy(ret.triangle_normals, this->triangle_normals.data(), sizeof(vec3_t) * this->triangle_normals.size());

	unsigned int v_count = static_cast<unsigned int>(this->vertex_positions.size()),
		t_count = static_cast<unsigned int>(this->triangle_indices.size());

	q->memcpy(ret.vertex_count, &v_count, sizeof(unsigned int));
	q->memcpy(ret.triangle_count, &t_count, sizeof(unsigned int));

	q->wait();
}