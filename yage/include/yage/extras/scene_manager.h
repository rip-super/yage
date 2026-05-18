#pragma once
#include <yage/extras/scene.h>
#include <memory>
#include <stack>
#include <queue>
#include <unordered_map>
#include <string>

namespace yage
{
    class Window;
    class Renderer;
    class Input;

    class SceneManager
    {
    public:
        void RegisterScene(const std::string& name, Scene* scene);
        void Run(const std::string& initial);
        void Set(const std::string& next);

        void Push(const std::string& overlay);
        void Pop();

        void Quit();

    private:
        void Inject(Scene *scene);

        Window *window = nullptr;
        Renderer *renderer = nullptr;
        Input *input = nullptr;

        std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
        std::string current_name;
        std::string next_name;
        std::stack<std::string> overlay_names;
        std::queue<std::string> pending_overlays;
        bool quit = false;
    };
}