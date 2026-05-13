#include <yage/yage.h>
using namespace yage;

int main()
{
    WindowConfig cfg;
    cfg.title = "Hello YAGE!";
    cfg.width = 800;
    cfg.height = 600;
    cfg.gl_version = GLVersion::GL_4_1;

    Window window(cfg);

    while (!window.ShouldClose())
    {
        window.Poll();

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.SwapBuffers();
    }
}