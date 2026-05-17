#include <yage/extras/scene.h>
#include <yage/extras/scene_manager.h>
#include <yage/window.h>
#include <yage/renderer.h>
#include <yage/input.h>

namespace yage
{
    void SceneManager::Inject(Scene *scene)
    {
        scene->window = window;
        scene->renderer = renderer;
        scene->input = input;
        scene->scenes = this;
    }

    void SceneManager::Run(std::unique_ptr<Scene> initial)
    {
        Window window(initial->window_config);
        Renderer renderer(window);
        Input input(window);

        this->window = &window;
        this->renderer = &renderer;
        this->input = &input;

        Inject(initial.get());
        initial->OnStart();

        std::unique_ptr<Scene> current = std::move(initial);

        while (!window.ShouldClose() && !quit)
        {
            window.Poll(input);
            float dt = window.GetDt();

            if (next)
            {
                current->OnEnd();
                Inject(next.get());
                next->OnStart();
                current = std::move(next);
            }

            renderer.BeginFrame();

            current->OnUpdate(dt);

            if (!overlays.empty())
                overlays.top()->OnUpdate(dt);

            renderer.EndFrame();
            window.SwapBuffers();
        }

        while (!overlays.empty())
        {
            overlays.top()->OnEnd();
            overlays.pop();
        }

        current->OnEnd();
    }

    void SceneManager::Set(std::unique_ptr<Scene> next)
    {
        this->next = std::move(next);
    }

    void SceneManager::Push(std::unique_ptr<Scene> overlay)
    {
        Inject(overlay.get());
        overlay->OnStart();
        overlays.push(std::move(overlay));
    }

    void SceneManager::Pop()
    {
        if (!overlays.empty())
        {
            overlays.top()->OnEnd();
            overlays.pop();
        }
    }

    void SceneManager::Quit()
    {
        quit = true;
    }
}