#ifndef BVH_H
#define BVH_H

#include "Helper.h"
#include "Model.h"

const int BVH_TRIANGLE_COUNT = 8;

struct BoundingVolume {
	unsigned int* triangles;// [BVH_TRIANGLE_COUNT] ;
	unsigned int triangle_count;
	vec3_t vertices[2];
	bool is_base, is_top;
};

struct BVHNode {
	BoundingVolume volume;
	int left_child_index, right_child_index;
};

struct d_BVH {
	BVHNode* nodes;
	unsigned int initial;
	unsigned int layers;
	unsigned int node_size;
};

class BVH {
protected:
	std::vector<BVHNode> nodes;
	uint32_t layers;
public:
	BVH();
	BVH(glModel*);

	d_BVH to_gpu();

	void debug_print();

	std::vector<BVHNode>* get_nodes() { return &this->nodes; }
};

#endif