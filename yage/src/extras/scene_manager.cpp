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

    void SceneManager::RegisterScene(const std::string &name, Scene *scene)
    {
        scenes[name] = std::unique_ptr<Scene>(scene);
    }

    void SceneManager::Run(const std::string &initial)
    {
        if (scenes.find(initial) == scenes.end())
            return;

        Window window(scenes[initial]->window_config);
        Renderer renderer(window);
        Input input(window);

        this->window = &window;
        this->renderer = &renderer;
        this->input = &input;

        current_name = initial;
        Inject(scenes[current_name].get());
        window.SetTitle(scenes[current_name]->window_config.title);
        scenes[current_name]->OnStart();

        while (!window.ShouldClose() && !quit)
        {
            window.Poll(input);
            float dt = window.GetDt();

            if (!next_name.empty())
            {
                while (!overlay_names.empty())
                {
                    scenes[overlay_names.top()]->OnEnd();
                    overlay_names.pop();
                }
                while (!pending_overlays.empty())
                    pending_overlays.pop();

                scenes[current_name]->OnEnd();
                current_name = next_name;
                next_name.clear();
                Inject(scenes[current_name].get());
                window.SetTitle(scenes[current_name]->window_config.title);
                scenes[current_name]->OnStart();
            }

            while (!pending_overlays.empty())
            {
                std::string overlay = pending_overlays.front();
                pending_overlays.pop();
                Inject(scenes[overlay].get());
                scenes[overlay]->OnStart();
                overlay_names.push(overlay);
                window.SetTitle(scenes[overlay]->window_config.title);
            }

            renderer.BeginFrame();
            if (overlay_names.empty())
                scenes[current_name]->OnUpdate(dt);
            else
                scenes[overlay_names.top()]->OnUpdate(dt);

            renderer.EndFrame();
            window.SwapBuffers();
        }

        while (!overlay_names.empty())
        {
            scenes[overlay_names.top()]->OnEnd();
            overlay_names.pop();
        }

        scenes[current_name]->OnEnd();
    }

    void SceneManager::Set(const std::string &next)
    {
        next_name = next;
    }

    void SceneManager::Push(const std::string &overlay)
    {
        pending_overlays.push(overlay);
    }

    void SceneManager::Pop()
    {
        if (!overlay_names.empty())
        {
            scenes[overlay_names.top()]->OnEnd();
            overlay_names.pop();

            const std::string &active = overlay_names.empty() ? current_name : overlay_names.top();
            window->SetTitle(scenes[active]->window_config.title);
        }
    }

    void SceneManager::Quit()
    {
        quit = true;
    }
}