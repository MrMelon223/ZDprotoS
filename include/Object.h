#ifndef OBJECT_H
#define OBJECT_H

#include "Model.h"

enum ObjectType {
	AI,
	Physics,
	Player,
	Weapon,
	Static
};

class Object {
private:
	void load_from(std::string);
protected:
	std::string name;
	std::string filepath;

	vec3_t spawn_position;

	vec3_t position;
	vec3_t rotation;
	vec3_t direction;
	vec3_t velocity;
	float mass_kg;

	int model_index;
	int hitbox_model_index;

	bool is_visible;

	ObjectType object_type;
public:
	Object(std::string);

	void set_visibility(bool b) { this->is_visible = b; }

	vec3_t get_position() { return this->position; }
	void set_position(vec3_t p) { this->position = p; }
};

#endif
