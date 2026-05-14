#include <yage/utils.h>
#include <yage/window.h>
#include <GLFW/glfw3.h>

static void applyGLVersionHints(yage::GLVersion version)
{
    int major, minor;

    switch (version)
    {
    case yage::GLVersion::GL_3_3:
        major = 3;
        minor = 3;
        break;
    case yage::GLVersion::GL_4_1:
        major = 4;
        minor = 1;
        break;
    case yage::GLVersion::GL_4_4:
        major = 4;
        minor = 4;
        break;
    case yage::GLVersion::GL_4_6:
        major = 4;
        minor = 6;
        break;
    default:
        major = 3;
        minor = 3;
        break;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

namespace yage
{
    Window::Window(const WindowConfig &cfg)
    {
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW");

        applyGLVersionHints(cfg.gl_version);
        glfwWindowHint(GLFW_RESIZABLE, cfg.resizable ? GLFW_TRUE : GLFW_FALSE);

        handle = cfg.fullscreen
                     ? glfwCreateWindow(cfg.width, cfg.height, cfg.title.c_str(), glfwGetPrimaryMonitor(), NULL)
                     : glfwCreateWindow(cfg.width, cfg.height, cfg.title.c_str(), NULL, NULL);

        if (!handle)
            throw std::runtime_error("Failed to create window");

        glfwMakeContextCurrent(handle);
        glfwSwapInterval(cfg.vsync ? 1 : 0);

        if (cfg.mouse_grab)
            glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("Failed to initalize GLAD");

        int fb_width, fb_height;
        glfwGetFramebufferSize(handle, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);
        this->fb_width = fb_width;
        this->fb_height = fb_height;

        int logical_width, logical_height;
        glfwGetWindowSize(handle, &logical_width, &logical_height);
        width = logical_width;
        height = logical_height;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        time = (float)glfwGetTime();
        dt = 0.0f;

        glfwSetWindowUserPointer(handle, this);

        // clang-format off
        glfwSetFramebufferSizeCallback(handle, 
            [](GLFWwindow *w, int width, int height) {
                auto *win = (Window*)glfwGetWindowUserPointer(w);
                win->width = width;
                win->height = height;
                glViewport(0, 0, width, height); 
            }
        );
        // clang-format on
    }

    Window::~Window()
    {
        glfwDestroyWindow(handle);
        glfwTerminate();
    }

    bool Window::ShouldClose() const { return glfwWindowShouldClose(handle); }
    void Window::SwapBuffers() { glfwSwapBuffers(handle); }
    void Window::SetTitle(const std::string &title) { glfwSetWindowTitle(handle, title.c_str()); }
    glm::ivec2 Window::GetSize() const { return glm::ivec2(width, height); }
    int Window::GetWidth() const { return width; }
    int Window::GetHeight() const { return height; }
    int Window::GetFramebufferWidth() const { return fb_width; }
    int Window::GetFramebufferHeight() const { return fb_height; }
    float Window::GetAspect() const { return (float)width / float(height); }

    void Window::Poll()
    {
        float now = (float)glfwGetTime();
        dt = now - time;
        time = now;
        glfwPollEvents();
    }

    float Window::GetDt() const { return dt; }
    float Window::GetTime() const { return time; }

    GLFWwindow *Window::GetHandle() const { return handle; }
}
