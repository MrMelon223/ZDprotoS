	// App.cpp
#include "../include/App.h"

std::vector<glModel> HOST_MODELS;
std::vector<d_Model> DEVICE_MODELS;

d_Model* d_DEVICE_MODELS;

std::queue<KeyboardButtonUse> keyboard_button_uses;
std::queue<MouseButtonUse> mouse_button_uses;



static void keyboard_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {

	KeyboardButtonUse k{};

	k.key = key;
	k.scancode = scancode;
	k.action = action;
	k.mods = mods;

	keyboard_button_uses.push(k);
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods) {

	MouseButtonUse k{};

	k.button = button;
	k.action = action;
	k.mods = mods;

	mouse_button_uses.push(k);
}

void Game::load_models_from_file() {
	std::ifstream models_in(this->model_list_path);
	if (!models_in) {
		std::cerr << "Cannot Find Model List File!" << std::endl;
		return;
	}

	std::string line;
	while (std::getline(models_in, line)) {
		std::istringstream line_in(line);
		std::string path = line.substr(0, line.find(","));
		std::string name = line.substr(line.find(",") + 1, line.size());

		printf("Path:Name = %s \t %s\n", path.c_str(), name.c_str());

		HOST_MODELS.push_back(glModel(path, name));
		DEVICE_MODELS.push_back(HOST_MODELS.back().to_gpu(&this->gpu_queue));
	}

	d_DEVICE_MODELS = sycl::malloc_device<d_Model>(DEVICE_MODELS.size(), this->gpu_queue);

	this->gpu_queue.memcpy(d_DEVICE_MODELS, DEVICE_MODELS.data(), sizeof(d_Model) * DEVICE_MODELS.size());

	this->gpu_queue.wait();
}

int Runtime::find_model(std::string name) {
	for (int i = 0; i < HOST_MODELS.size(); i++) {
		glModel* current = &HOST_MODELS[i];

		if (current->get_name() == name) {
			return i;
		}
	}
	return -1;
}

/*void Game::resize_callback(GLFWwindow* win, int width, int height) {
	this->window->resize_viewport(dim_t{ width, height });
}*/

void Game::load_objects_from_file() {
	std::ifstream objs_in(object_list_path);
	if (!objs_in) {
		std::cerr << "Cannot Find Main Objects File!" << std::endl;
		return;
	}

	std::string line;
	while (std::getline(objs_in, line)) {
		this->objects.push_back(Object(line));
	}
}

void Game::empty_queues() {
	while (!keyboard_button_uses.empty()) {
		this->input_handle(keyboard_button_uses.front());
		keyboard_button_uses.pop();
	}

	while (!mouse_button_uses.empty()) {
		this->mouse_handle(mouse_button_uses.front());
		mouse_button_uses.pop();
	}
}

void Game::input_handle(KeyboardButtonUse& k) {

	int CURRENT_KEY = k.key;
	int CURRENT_ACTION = k.action;

	std::cout << CURRENT_KEY << std::endl;

	//this->is_walking = false, this->is_sprinting = false, this->trying_sprint = false;;
	switch (CURRENT_KEY) {
	case GLFW_KEY_W:
		if (CURRENT_ACTION == GLFW_RELEASE) {

		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {

		}
		break;
	case GLFW_KEY_S:
		if (CURRENT_ACTION == GLFW_RELEASE) {

		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {

		}
		break;
	case GLFW_KEY_A:
		if (CURRENT_ACTION == GLFW_RELEASE) {

		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {

		}
		break;
	case GLFW_KEY_D:
		if (CURRENT_ACTION == GLFW_RELEASE) {

		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {

		}
		break;
	case GLFW_KEY_R:
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {
			//this->cam->set_capture_mode(RT);
		}
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		break;
	case GLFW_KEY_F:
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {
			//this->camera->set_capture_mode(FULLBRIGHT);
		}
		break;
	case GLFW_KEY_ESCAPE:
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {
			this->window->set_running(false);
		}
		break;
	case GLFW_KEY_P:
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {
			//this->cam->get_rays(0)->debug_stats();
		}
		break;
	case GLFW_KEY_SPACE:
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {

		}
		break;
	case GLFW_MOUSE_BUTTON_LEFT:
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		if (CURRENT_ACTION == GLFW_REPEAT) {

		}
		break;
	}

	int CURRENT_MOD = k.mods;
	switch (CURRENT_MOD) {
	case GLFW_MOD_SHIFT:

		break;
	}
}

void Game::mouse_handle(MouseButtonUse& k) {

	int CURRENT_MOUSE = k.button;
	int CURRENT_ACTION = k.action;

	switch (CURRENT_MOUSE) {
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (CURRENT_ACTION == GLFW_RELEASE) {

		}
		if (CURRENT_ACTION == GLFW_PRESS) {

		}
		break;
	case GLFW_MOUSE_BUTTON_LEFT:
		if (CURRENT_ACTION == GLFW_RELEASE) {
		}
		if (CURRENT_ACTION == GLFW_PRESS || CURRENT_ACTION == GLFW_REPEAT) {

		}
		break;

	}
}

void Game::debug_print_device(sycl::device* d) {
	std::cout << std::setw(12) << "Device: " << d->get_info< sycl::info::device::name>() << std::endl;
	std::cout << std::setw(10) << "Cores: " << d->get_info<sycl::info::device::max_compute_units>() << std::endl;
}

Game::Game() {
	this->window = new Window(640, 480, false);

	this->cpu_device = sycl::device(sycl::cpu_selector_v);
	this->gpu_device = sycl::device(sycl::gpu_selector_v);

	this->cpu_queue = sycl::queue(this->cpu_device);
	this->gpu_queue = sycl::queue(this->gpu_device);

	debug_print_device(&this->cpu_device);
	debug_print_device(&this->gpu_device);

	this->load_models_from_file();

	this->load_objects_from_file();

	this->camera = new Camera(this->window->dims.x, this->window->dims.y, &this->gpu_queue);

	this->instances.push_back(d_ModelInstance{ 0, vec3_t{0.0f, 0.0f, 0.0f}, vec3_t{0.0f, 0.0f, 0.0f}});
	glfwMakeContextCurrent(this->window->get_window_ptr());
}

void Game::main_loop() {
	glfwSetKeyCallback(this->window->get_window_ptr(), keyboard_callback);
	glfwSetMouseButtonCallback(this->window->get_window_ptr(), mouse_callback);

	while (this->window->is_running()) {
		color_t* frame_buff = new color_t[this->window->dims.y * this->window->dims.x];

		this->empty_queues();

		this->camera->capture(this->instances.data(), static_cast<unsigned int>(this->instances.size()));

		this->camera->copy_data_out(this->window);

		glfwSwapBuffers(this->window->get_window_ptr());
		glfwPollEvents();

		delete frame_buff;
	}
}