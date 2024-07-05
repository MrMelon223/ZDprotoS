	// Window.cpp

#include "../include/Window.h"

void Window::resize_viewport(dim_t dimensions) {
    this->dims = dimensions;

    delete this->frame_buffer;

    this->frame_buffer = new color_t[this->dims.y * this->dims.x];
}

Window::Window(int width, int height, bool fullscreen) {
    this->dims = dim_t{ width, height };
    this->is_fullscreen = fullscreen;
    this->running = true;

    this->frame_buffer = new color_t[this->dims.y * this->dims.x];
    if (glfwInit() == -1) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    // Create a window
    this->window = glfwCreateWindow(this->dims.x, this->dims.y, "ZD-sycl", NULL, NULL);

}