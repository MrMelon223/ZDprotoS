#ifndef APP_H
#define APP_H

#include "Window.h"
#include "Model.h"
#include "Camera.h"
#include "Object.h"

static void keyboard_callback(GLFWwindow*, int, int, int, int);
static void mouse_callback(GLFWwindow*, int, int, int);

struct KeyboardButtonUse {
	int key, scancode, action, mods;
};
struct MouseButtonUse {
	int button, action, mods;
};

extern std::queue<KeyboardButtonUse> keyboard_button_uses;
extern std::queue<MouseButtonUse> mouse_button_uses;

extern std::vector<glModel> HOST_MODELS;
extern std::vector<d_Model> DEVICE_MODELS;

namespace Runtime {

	int find_model(std::string);

}

void resize_callback(GLFWwindow*, int, int);

class Game {
private:
	std::string config_path = "config.txt";
	std::string shader_program_path = "resources/shader_programs.txt";
	std::string model_list_path = "resources/models.txt";
	std::string object_list_path = "resources/objects.txt";

	void load_settings_from_cfg();
	void load_models_from_file();
	void load_objects_from_file();

	dim_t initial_window_size;
	bool initial_fullscreen;

	void empty_queues();

	void input_handle(KeyboardButtonUse&);
	void mouse_handle(MouseButtonUse&);

protected:
	Window* window;

	sycl::queue gpu_queue, cpu_queue;

	//std::vector<glShaderProgram> shader_programs;

	std::vector<Object> objects;
	std::vector<d_ModelInstance> instances;

	Camera* camera;

public:
	Game();

	void main_loop();
};

#endif
