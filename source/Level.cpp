	// Level.cpp
#include "../include/Level.h"
#include "../include/App.h"

void Level::load_objects_from_file() {
	std::ifstream objs_in(object_list_path);
	if (!objs_in) {
		std::cerr << "Cannot Find Main Objects File!" << std::endl;
		return;
	}

	std::string line;
	while (std::getline(objs_in, line)) {
		this->loaded_objects.push_back(Object(line));
	}
}

unsigned int Level::find_object_index(std::string name) {
	unsigned int count = 0;
	for (Object o : this->loaded_objects) {
		if (o.get_name() == name) {
			return count;
		}
		count++;
	}
	return 0;
}

void Level::load_from(std::string path) {

	//error_check(cudaMalloc((void**)&d_materials, sizeof(d_Material) * DEVICE_MATERIALS.size()));
	//error_check(cudaMemcpy(d_materials, DEVICE_MATERIALS.data(), sizeof(d_Material) * DEVICE_MATERIALS.size(), cudaMemcpyHostToDevice));

	std::ifstream in;
	in.open(path, std::ios::in);
	if (!in) {
		std::cout << "Cannot find Level: " << path << std::endl;
		return;
	}

	std::cout << "Loading Level: " << path << std::endl;

	this->loaded_objects = *new std::vector<Object>();

	std::string line;
	std::getline(in, line);
	std::istringstream parse(line);
	size_t leng = 0;

	parse >> leng;
	std::cout << leng << " static models detected!" << std::endl;
	std::string model;

	this->d_DEVICE_MODELS = sycl::malloc_device<d_Model>(DEVICE_MODELS.size(), *this->queue);
	uint32_t model_count = 0;

	this->queue->memcpy(this->d_DEVICE_MODELS, DEVICE_MODELS.data(), sizeof(d_Model) * DEVICE_MODELS.size());
	this->queue->wait();

	for (size_t i = 0; i < leng; i++) {
		std::getline(in, line);
		std::istringstream in0(line);

		float x, y, z, x_r, y_r, z_r, scale;

		in0 >> x >> y >> z >> x_r >> y_r >> z_r >> scale >> model;
		//std::cout << model << std::endl;

		vec3_t position = vec3_t{ x, y, z };
		vec3_t rotation = vec3_t{ x_r, y_r, z_r };

		//glModel* h_model = Runtime::find_model(model);

		this->instances.push_back(create_instance(Runtime::find_model(model), position, rotation, HOST_MODELS[Runtime::find_model(model)].get_tri_count(), true, scale));

		std::cout << "d_model = " << this->instances.back().model_index << std::endl;

		//d_Model d_model2 = Runtime::find_host_model(model)->to_gpu();

		//d_models.push_back(DEVICE_MODELS.at(Runtime::find_host_model_index(model)));	// -> Dis piece o shite

	}
	size_t light_leng;
	std::string line2;
	std::getline(in, line2);
	std::istringstream parse2(line2);
	parse2 >> light_leng;
	std::cout << light_leng << " lights detected!" << std::endl;
	std::vector<point_light_t> point_lights;
	for (size_t i = 0; i < light_leng; i++) {
		std::getline(in, line);
		std::istringstream in1(line);

		float x, y, z, r, g, b, a, s_r, s_g, s_b, s_a, intensity, falloff, range;

		in1 >> x >> y >> z >> r >> g >> b >> a >> s_r >> s_g >> s_b >> s_a >> intensity >> falloff >> range;
		//std::cout << model << std::endl;

		vec3_t position = vec3_t{ x, y, z };
		color_t color = color_t{ r, g, b, a };
		color_t s_color = color_t{ s_r, s_g, s_b, s_a };

		point_lights.push_back(point_light_t{ position, color, s_color, intensity, falloff, range });
	}

	std::getline(in, line);
	std::istringstream in2(line);

	float r, g, b, a, s_r, s_g, s_b, s_a, intensity;

	in2 >> r >> g >> b >> a >> s_r >> s_g >> s_b >> s_a >> intensity;

	ambient_light_t amb_light = ambient_light_t{ color_t{r, g, b, a}, color_t{s_r, s_g, s_b, s_a}, intensity };

	this->d_model_instance_count = static_cast<uint32_t>(this->instances.size());

	this->d_point_lights_count = static_cast<uint32_t>(point_lights.size());

	std::getline(in, line);
	std::istringstream in3(line);
	size_t object_count;
	in3 >> object_count;
	std::cout << std::setw(10) << object_count << " objects detected!" << std::endl;
	bool has_player = false;
	for (size_t i = 0; i < object_count; i++) {
		std::getline(in, line);
		std::istringstream in_obj(line);
		uint32_t type = 0;
		float x2, y2, z2, x_d, y_d, z_d, scale;
		std::string visual_model, rigid_model;

		in_obj >> x2 >> y2 >> z2 >> x_d >> y_d >> z_d >> scale >> visual_model;

		ObjectType obj_type = ObjectType::AI;
		//std::cout << "Type " << type << std::endl;

		d_ModelInstance instance;
		uint32_t model_idx = 0, instance_idx = 0, hitbox_index = 0;

		this->loaded_objects.push_back(Runtime::objects[this->find_object_index(visual_model)]);
		this->loaded_objects.back().set_position(vec3_t{ x2, y2, z2 });
		this->loaded_objects.back().set_rotation(vec3_t{ x_d, y_d, z_d });

		this->add_model_instance(create_instance(this->loaded_objects.back().get_model_index(), this->loaded_objects.back().get_position(), this->loaded_objects.back().get_rotation(), HOST_MODELS[this->loaded_objects.back().get_model_index()].get_tri_count(), false, scale));
		this->loaded_objects.back().set_model_index(static_cast<uint32_t>(this->instances.size() - 1));
		this->loaded_objects.back().set_spawn_point(vec3_t{ x2, y2, z2 });

		if (this->loaded_objects.back().get_object_type() == ObjectType::Player) {
			this->loaded_objects.back().attach_camera(this->camera_ptr);
		}
		std::cout << "Object added: " << &this->loaded_objects.back() << " @ " << this->loaded_objects.back().get_model_index() << " index with model " << visual_model << " of type " << this->loaded_objects.back().get_object_type() << std::endl;
	}

	this->d_DEVICE_MODELS = sycl::malloc_device<d_Model>(DEVICE_MODELS.size(), *this->queue);
	this->queue->memcpy(this->d_DEVICE_MODELS, DEVICE_MODELS.data(), sizeof(d_Model) * DEVICE_MODELS.size());
	this->d_DEVICE_MODEL_COUNT = static_cast<uint32_t>(DEVICE_MODELS.size());


	this->d_ambient_light = sycl::malloc_device<ambient_light_t>(1, *this->queue);
	this->queue->memcpy(this->d_ambient_light, &amb_light, sizeof(ambient_light_t));

	this->d_point_lights = sycl::malloc_device<point_light_t>(point_lights.size(), *this->queue);
	this->queue->memcpy(this->d_point_lights, point_lights.data(), sizeof(point_light_t)* point_lights.size());

	this->d_model_instances = sycl::malloc_device<d_ModelInstance>(this->instances.size(), *this->queue);
	this->queue->memcpy(this->d_model_instances, this->instances.data(), sizeof(d_ModelInstance)* this->instances.size());
	this->d_point_lights_count = static_cast<unsigned int>(this->instances.size());

	this->queue->wait();
}

Level::Level() {

}


Level::Level(std::string path, Camera* cam) {
	std::cout << "Initializing level from: " << path << std::endl;
	this->camera_ptr = cam;
	this->queue = cam->get_queue_ptr();
	this->d_objects = nullptr;
	this->load_from(path);
}

void Level::add_object(Object obj) {
	this->loaded_objects.push_back(obj);
}

void Level::clean_d_objects() {
	sycl::free(this->d_objects, *this->queue);
	sycl::free(this->d_model_instances, *this->queue);
}

void Level::upload_objects() {
	this->d_objects = sycl::malloc_device<Object>(this->loaded_objects.size(), *this->queue);
	try {
		this->queue->memcpy(this->d_objects, this->loaded_objects.data(), sizeof(Object)* this->loaded_objects.size());
		this->queue->wait();
	}
	catch (sycl::exception& e) {
		std::cerr << "ERROR::UPLOAD_OBJECTS_TO_GPU: " << e.what() << std::endl;
	}
	this->d_object_count = static_cast<uint32_t>(this->loaded_objects.size());
}