#pragma once
#include <yage/extras/scene.h>
#include <memory>
#include <stack>

namespace yage
{
    class Window;
    class Renderer;
    class Input;

    class SceneManager
    {
    public:
        void Run(std::unique_ptr<Scene> inital);
        void Set(std::unique_ptr<Scene> next);

        void Push(std::unique_ptr<Scene> overlay);
        void Pop();

        void Quit();

    private:
        void Inject(Scene *scene);

        Window *window = nullptr;
        Renderer *renderer = nullptr;
        Input *input = nullptr;

        std::unique_ptr<Scene> next;
        std::stack<std::unique_ptr<Scene>> overlays;
        bool quit = false;
    };
}