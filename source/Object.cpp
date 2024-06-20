	// Object.cpp
#include "../include/Object.h"
#include "../include/App.h"

void Object::load_from(std::string path) {
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Cannot Load Object from: " << path << std::endl;
		return;
	}

	std::string name, vis_model, hit_model;
	float mass_kg;
	int type, visible;

	std::string line;
	std::getline(file, line);
	std::istringstream name_in(line);
	name_in >> name;

	std::getline(file, line);
	std::istringstream vis_mod_in(line);
	vis_mod_in >> vis_model;

	std::getline(file, line);
	std::istringstream hit_mod_in(line);
	hit_mod_in >> hit_model;

	std::getline(file, line);
	std::istringstream mass_in(line);
	mass_in >> mass_kg;

	std::getline(file, line);
	std::istringstream type_in(line);
	type_in >> type;

	std::getline(file, line);
	std::istringstream vis_in(line);
	vis_in >> visible;

	this->name = name;

	this->model_index = Runtime::find_model(vis_model);
	if (this->model_index < 0) {
		std::cerr << "Visible Model: " << vis_model << ", not found for glObject: " << path << std::endl;
	}
	this->hitbox_model_index = Runtime::find_model(hit_model);
	if (this->hitbox_model_index < 0) {
		std::cerr << "Hitbox Model: " << hit_model << ", not found for glObject: " << path << std::endl;
	}

	if (visible == 0) {
		this->is_visible = false;
	}
	else {
		this->is_visible = true;
	}

	this->mass_kg = mass_kg;

	if (type == 1) {
		this->object_type = ObjectType::AI;
	}
	else if (type == 2) {
		this->object_type = ObjectType::Physics;
	}
	else if (type == 3) {
		this->object_type = ObjectType::Player;
	}
	else if (type == 4) {
		this->object_type = ObjectType::Weapon;
	}
	else if (type == 5) {
		this->object_type = ObjectType::Static;
	}
}

Object::Object(std::string path) {
	this->filepath = path;
	this->load_from(this->filepath);

	this->position = vec3_t(0.0f, 0.0f, 0.0f);
	this->rotation = vec3_t(90.0f, 90.0f, 90.0f);
}