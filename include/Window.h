#ifndef WINDOW_H
#define WINDOW_H

#include "Helper.h"

class Window {
private:
protected:
	bool running;
	GLFWwindow* window;

	color_t* frame_buffer;
public:
	dim_t dims;
	bool is_fullscreen;

	Window(int, int, bool);

	GLFWwindow* get_window_ptr() { return this->window; }

	void resize_viewport(dim_t);

	bool is_running() { return this->running; }
	void set_running(bool b) { this->running = b; }

	color_t* get_frame_buffer_ptr() { return this->frame_buffer; }
};

#endif
