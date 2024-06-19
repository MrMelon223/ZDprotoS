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

    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    // Configure GLFW
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    this->window = glfwCreateWindow(this->dims.x, this->dims.y, "ZD-sycl", NULL, NULL);
    if (!this->window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(this->window);
}