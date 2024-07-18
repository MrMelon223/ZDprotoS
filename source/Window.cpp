	// Window.cpp

#include "../include/Window.h"

void Window::resize_viewport(dim_t dimensions) {
    this->dims = dimensions;

    sycl::free(this->frame_buffer, *this->queue);

    this->frame_buffer = sycl::malloc_device<color_t>(this->dims.y * this->dims.x, *this->queue);
}

Window::Window(sycl::queue* q, int width, int height, bool fullscreen) {
    this->dims = dim_t{ width, height };
    this->is_fullscreen = fullscreen;
    this->running = true;

    this->queue = q;

    this->frame_buffer = sycl::malloc_device<color_t>(this->dims.y * this->dims.x, *this->queue);
    if (glfwInit() == -1) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    // Create a window
    this->window = glfwCreateWindow(this->dims.x, this->dims.y, "ZD-sycl", NULL, NULL);

}