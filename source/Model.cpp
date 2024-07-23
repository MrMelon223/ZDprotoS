	// Model.cpp
#include "../include/Model.h"

bool ray_intersects_box(vec3_t pos, vec3_t dir, vec3_t min, vec3_t max, int& towards) {
	vec3_t invDirection = vec3_t(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);

	// Calculate intersection distances
	vec3_t min_pos = min - pos;
	vec3_t max_pos = max - pos;


	vec3_t t1 = zd::cross(min_pos, invDirection);
	vec3_t t2 = zd::cross(max_pos, invDirection);

	// Find the maximum and minimum of these intersection distances
	vec3_t tMin = zd::min(t1, t2);
	vec3_t tMax = zd::max(t1, t2);

	float tNear = zd::maxf(zd::maxf(tMin.x, tMin.y), tMin.z);
	float tFar = zd::minf(zd::minf(tMax.x, tMax.y), tMax.z);

	if (tFar >= tNear) {
		if (tNear >= 0.0f) {
			towards = 1;
		}
		else {
			towards = -1;
		}
	}
	else {
		towards = 0;
	}

	return tFar >= tNear;
}

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

		this->vertex_positions.push_back(vec3_t{ x, y, z });
		vec3_t position{ x,y,z };
		position = zd::normalize(position);
		this->vertex_normals.push_back(position);
		this->vertex_colors.push_back(color_t{ r, g, b, a });
		this->vertex_uvs.push_back(uv_t{ u, v });
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

		vec3_t op_a = vb - va,
			op_b = vb - vc;

		vec3_t t_norm = zd::cross(op_a, op_b),
			pos_a = this->vertex_normals[x],
			pos_b = this->vertex_normals[y],
			pos_c = this->vertex_normals[z],
			op_c = pos_a + pos_b,
			norm_w = op_c + pos_c;

		norm_w.x = norm_w.x / 3.0f;
		norm_w.y = norm_w.y / 3.0f;
		norm_w.z = norm_w.z / 3.0f;

		if (zd::dot(t_norm, norm_w) < 0.0f) {
			t_norm.x = -t_norm.x;
			t_norm.y = -t_norm.y;	// This is gonna be a major floating point rounding headache in the future, goodluck homie
			t_norm.z = -t_norm.z;
		}

		this->triangle_indices.push_back(tri_t{ x, y, z});
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
	this->bvh = new BVH(this, q);

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

	ret.d_bvh = malloc_device<d_BVH>(1, *q);

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

	d_BVH d_bvh = this->bvh->to_gpu();

	q->memcpy(ret.d_bvh, &d_bvh, sizeof(d_BVH));

	q->wait();

	return ret;
}

// BVH

int32_t bvh_layer_count(size_t volume_count) {
	int32_t layer_count = 1;
	int32_t v_count = static_cast<int32_t>(volume_count);
	int32_t last = v_count;
	while (v_count > 2) {
		v_count -= last / 2;
		last = v_count;
		layer_count++;
	}
	return layer_count;
}

BVH::BVH() {

}

uint32_t find_closest(tri_t* t, std::vector<vec3_t> verts, std::vector<tri_t> tris, bool* used, int cur_idx) {
	vec3_t center = (verts[t->a] + verts[t->b]) + verts[t->c];
	center.x /= 3.0f;
	center.y /= 3.0f;
	center.z /= 3.0f;

	float closest = 100000.0f;
	int32_t tracker = 0, ret = -1;
	for (size_t i = cur_idx; i < tris.size(); i++) {
		/*if (used[i && i != cur_idx]) {
			ret = i;
			break;
		}*/
		tri_t tr = tris[i];
		if (!used[i] && i != cur_idx) {
			vec3_t center_test = (verts[tr.a] + verts[tr.b] + verts[tr.c]);
			center_test.x /= 3.0f;
			center_test.y /= 3.0f;
			center_test.z /= 3.0f;

			if (ret == -1) {
				ret == i;
			}
			if (zd::length(center_test - center) < closest) {
				ret = i;
			}
		}
	}
	return ret;
}

BVH::BVH(glModel* model, sycl::queue* q) {
	this->queue = q;
	std::vector<vec3_t> verts = model->get_vertex_positions();
	std::vector<tri_t> tris = model->get_triangle_indices();
	//if (tris.size() < BVH_TRIANGLE_COUNT) {
	BoundingVolume vol = {};
	uint32_t* tri_idxs = (uint32_t*)malloc(tris.size() * sizeof(uint32_t));
	for (uint32_t j = 0; j < tris.size(); j++) {
		tri_idxs[j] = j;
	}

	vol.triangles = sycl::malloc_device<uint_t>(tris.size(), *this->queue);
	this->queue->memcpy(vol.triangles, tri_idxs, tris.size() & sizeof(uint_t));
	vol.triangle_count = static_cast<uint32_t>(tris.size());

	vec3_t min, max = min = verts.at(tris.at(tri_idxs[0]).a);
	for (uint32_t j = 1; j < vol.triangle_count; j++) {
		tri_t* t = &tris.at(tri_idxs[j]);
		vec3_t* va = &verts.at(t->a);
		vec3_t* vb = &verts.at(t->b);
		vec3_t* vc = &verts.at(t->c);
		min = zd::min(min, zd::min(zd::min(*va, *vb), *vc));
		max = zd::max(min, zd::max(zd::max(*va, *vb), *vc));
	}

	free(tri_idxs);

	vol.vertices[0] = min;
	vol.vertices[1] = max;
	vol.is_base = true;

	BVHNode node = { vol, -1, -1 };

	this->nodes.push_back(node);
	//}
	/*else {
		bool* used = new bool[tris.size()];


		for (size_t i = 0; i < tris.size(); i++) {
			used[i] = false;
		}

		int32_t idx = 0;
		size_t count = 0;
		int32_t remaining = static_cast<uint32_t>(tris.size());

		//std::cout << "Model Vertex Count: " << verts.size() << std::endl;
		//std::cout << "Model Triangle Count: " << tris.size() << std::endl;
		while (remaining > 1) {
			//std::cout << "Remaining: " << remaining << std::endl;
			BoundingVolume vol = {};


			bool none_found = false;
			int cap = -1;
			for (uint32_t j = 0; j < BVH_TRIANGLE_COUNT; j++) {
				used[idx] = true;
				int found = find_closest(&tris[idx], verts, tris, used, idx);
				if (found >= 0) {
					vol.triangles[j] = static_cast<uint32_t>(found);
					used[vol.triangles[j]] = true;
					idx = found;
					//printf("Triangle found = %i\n", vol.triangles[j]);
				}
				else {
					int32_t next = -1;
					for (int32_t k = 0; k < tris.size(); k++) {
						if (!used[k]) {
							next = k;
							idx = next;
						}
					}
					if (next == -1) {
						none_found = true;
						cap = j;
						break;
					}
					vol.triangles[j] = next;
					used[vol.triangles[j]] = true;
				}
				//printf("Triangle found = %i\n", vol.triangles[j]);
				//used[vol.triangles[j]] = true;
			}
			if (none_found) {
				vol.triangle_count = cap;
			}
			else {
				vol.triangle_count = BVH_TRIANGLE_COUNT;
			}
			remaining -= vol.triangle_count;
			bool made_it = true;

			glm::vec3 min, max = min = verts.at(tris.at(vol.triangles[0]).a).position;
			for (uint8_t j = 1; j < vol.triangle_count; j++) {
				//std::cout << "Triangle " << vol.triangles[j] << std::endl;
				Tri* t = &tris.at(vol.triangles[j]);
				Vertex* va = &verts.at(t->a);
				Vertex* vb = &verts.at(t->b);
				Vertex* vc = &verts.at(t->c);

				min = glm::min(min, glm::min(glm::min(va->position, vb->position), vc->position));
				max = glm::max(min, glm::max(glm::max(va->position, vb->position), vc->position));
			}

			vol.vertices[0] = glm::vec3(min);
			vol.vertices[1] = glm::vec3(max);
			vol.is_base = true;

			BVHNode node = { vol, -1, -1 };

			this->nodes.push_back(node);

			count++;
		}
		delete[] used;
	}
	this->layers = bvh_layer_count(this->nodes.size());
	//std::cout << "Need Layers #" << this->layers << std::endl;
	int32_t last = static_cast<int32_t>(this->nodes.size());
	int32_t running_total = 0;
	if (this->nodes.size() > 1) {
		while(static_cast<float>(last) * 0.5f > 1.0f) {
			//std::cout << "Layering tree for iteration " << n << std::endl;
			last = static_cast<int32_t>(static_cast<float>(last) * 0.5f);
			for (int32_t i = 1; i < last; i++) {
				BVHNode* n1 = &this->nodes[running_total],
					* n2 = &this->nodes[running_total + 1];

				glm::vec3 n1_min = n1->volume.vertices[0],
					n1_max = n1->volume.vertices[1];
				glm::vec3 n2_min = n2->volume.vertices[0],
					n2_max = n2->volume.vertices[1];

				glm::vec3 min = glm::min(n1_min, n2_min), max = glm::max(n1_max, n2_max);

				BoundingVolume vol{};
				vol.is_base = false;
				vol.vertices[0] = min;
				vol.vertices[1] = max;

				BVHNode n3 = {};
				n3.volume = vol;
				vol.is_top = false;
				n3.left_child_index = static_cast<int32_t>(running_total);
				n3.right_child_index = static_cast<int32_t>(running_total + 1);

				printf("Children indexs = %i,%i\n", n3.left_child_index, n3.right_child_index);

				this->nodes.push_back(n3);
				running_total += 2;
			}
			BVHNode* n1 = &this->nodes[running_total],
				* n2 = &this->nodes[running_total + 1];

			glm::vec3 n1_min = n1->volume.vertices[0],
				n1_max = n1->volume.vertices[1];
			glm::vec3 n2_min = n2->volume.vertices[0],
				n2_max = n2->volume.vertices[1];

			glm::vec3 min = glm::min(n1_min, n2_min), max = glm::max(n1_max, n2_max);

			BoundingVolume vol{};
			vol.is_base = false;
			vol.vertices[0] = min;
			vol.vertices[1] = max;

			BVHNode n3 = {};
			n3.volume = vol;
			vol.is_top = false;
			n3.left_child_index = static_cast<int32_t>(running_total);
			n3.right_child_index = static_cast<int32_t>(running_total + 1);

			this->nodes.push_back(n3);

		}
		this->nodes.back().volume.is_top = true;
	}*/
	this->debug_print();
}

d_BVH BVH::to_gpu() {
	d_BVH d_bvh{};
	d_bvh.nodes = sycl::malloc_device<BVHNode>(this->nodes.size(), *this->queue);
	this->queue->memcpy(d_bvh.nodes, this->nodes.data(), sizeof(BVHNode) * this->nodes.size());
	printf("BVH Node count = %i\n", this->nodes.size());
	d_bvh.initial = 0;//static_cast<uint32_t>(this->nodes.size() - 1);
	//d_bvh.layers = this->layers;
	d_bvh.layers = 1;
	d_bvh.node_size = static_cast<uint32_t>(this->nodes.size());

	return d_bvh;
}

void BVH::debug_print() {
	for (BVHNode n : this->nodes) {
		printf("Node left = %i, Node right = %i\n", n.left_child_index, n.right_child_index);
		/*for (uint32_t i = 0; i < n.volume.triangle_count; i++) {
			printf("     Tri %i = %i\n", i, n.volume.triangles[i]);
		}*/
	}
}