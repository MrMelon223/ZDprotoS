#ifndef LEVEL_H
#define LEVEL_H

#include "Model.h"
#include "Light.h"
#include "Object.h"
#include "Camera.h"

class Level {
protected:

	std::string object_list_path = "resources/objects.txt";

	sycl::queue* queue;

	std::string name;

	std::vector<Object> loaded_objects;
	Object* d_objects;
	uint32_t d_object_count;

	std::vector<d_ModelInstance> instances;

	d_ModelInstance* d_model_instances;
	uint32_t d_model_instance_count;

	d_Model* d_DEVICE_MODELS;
	uint32_t d_DEVICE_MODEL_COUNT;

	ambient_light_t* d_ambient_light;
	point_light_t* d_point_lights;
	uint32_t d_point_lights_count;

	//d_Material* d_materials;
	//uint32_t d_materials_count;

	Camera* camera_ptr;

	void load_from(std::string);

	Object* PLAYER_OBJECT;

	unsigned int find_object_index(std::string);
	void load_objects_from_file();

public:
	Level();
	Level(std::string, Camera*);

	void add_model_instance(d_ModelInstance d) { this->instances.push_back(d); }

	d_ModelInstance* get_d_model_instances() { return this->d_model_instances; }
	uint32_t get_d_model_instance_count() { return this->d_model_instance_count; }

	d_Model* get_d_device_models() { return this->d_DEVICE_MODELS; }

	ambient_light_t* get_d_ambient_light() { return this->d_ambient_light; }
	point_light_t* get_d_point_lights() { return this->d_point_lights; }
	uint32_t get_d_point_lights_size() { return this->d_point_lights_count; }

	void update_instance(uint32_t, d_ModelInstance);

	void upload_instances();

	void add_object(Object);
	void update_object(uint32_t, Object);
	Object* get_objects_ptr() { return d_objects; }
	uint32_t get_object_count() { return static_cast<uint32_t>(this->loaded_objects.size()); }
	void upload_objects();
	void clean_d_objects();
	Object* get_d_objects() { return this->d_objects; }
	uint32_t get_d_object_count() { return this->d_object_count; }

	Object* get_player_object() { return this->PLAYER_OBJECT; }

	//d_Material* get_d_materials() { return this->d_materials; }

	//uint32_t get_instance_index(d_ModelInstance);

};

#endif