#ifndef MODEL_H
#define MODEL_H

#include "Helper.h"

const int BVH_TRIANGLE_COUNT = 8;

struct BoundingVolume {
	uint32_t* triangles;// [BVH_TRIANGLE_COUNT] ;
	uint32_t triangle_count;
	vec3_t vertices[2];
	bool is_base, is_top;
};

struct BVHNode {
	BoundingVolume volume;
	int32_t left_child_index, right_child_index;
};

struct d_BVH {
	BVHNode* nodes;
	uint32_t initial;
	uint32_t layers;
	uint32_t node_size;
};

class BVH;

extern SYCL_EXTERNAL bool ray_intersects_box(vec3_t, vec3_t, vec3_t, vec3_t, int&);

typedef struct d_Model {
	sycl::queue* queue;

	vec3_t* vertex_positions;
	vec3_t* vertex_normals;
	color_t* vertex_colors;
	uv_t* vertex_uvs;
	unsigned int* vertex_count;

	tri_t* triangle_indices;
	vec3_t* triangle_normals;
	unsigned int* triangle_count;

	unsigned int* material_index;

	d_BVH* d_bvh;
};

class glModel {
private:
	void load_from(std::string);
protected:
	std::string filepath;
	std::string name;

	std::vector<vec3_t> vertex_positions;
	std::vector<vec3_t> vertex_normals;
	std::vector<color_t> vertex_colors;
	std::vector<uv_t> vertex_uvs;

	std::vector<tri_t> triangle_indices;
	std::vector<vec3_t> triangle_normals;

	BVH* bvh;

public:
	glModel(std::string, std::string);

	d_Model to_gpu(sycl::queue*);

	std::string get_name() { return this->name; }

	unsigned int get_tri_count() { return static_cast<unsigned int>(this->triangle_indices.size()); }

	std::vector<vec3_t> get_vertex_positions() { return this->vertex_positions; }
	std::vector<tri_t> get_triangle_indices() { return this->triangle_indices; }
};

typedef struct d_ModelInstance {
	unsigned int model_index;
	vec3_t position;
	vec3_t rotation;
	unsigned int triangle_count;
	bool show;
	float scale;
};

inline d_ModelInstance create_instance(unsigned int m_idx, vec3_t pos, vec3_t rot, unsigned int t_count, bool show, float scale) {
	return d_ModelInstance{ m_idx, pos, rot, t_count, show, scale };
}

class BVH {
protected:
	sycl::queue* queue;
	std::vector<BVHNode> nodes;
	uint32_t layers;
public:
	BVH();
	BVH(glModel*, sycl::queue*);

	d_BVH to_gpu();

	void debug_print();

	std::vector<BVHNode>* get_nodes() { return &this->nodes; }
};

#endif
