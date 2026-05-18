#pragma once
#include <yage/utils.h>

namespace yage
{

    class SceneManager;
    class Renderer;
    class Input;
    class Shader;
    class Texture;
    class Window;

    class Scene
    {
    public:
        WindowConfig window_config;

        virtual void OnStart() = 0;
        virtual void OnUpdate(float dt) = 0;
        virtual void OnEnd() = 0;
        virtual ~Scene() = default;

    protected:
        Window *window = nullptr;
        Renderer *renderer = nullptr;
        Input *input = nullptr;
        SceneManager *scenes = nullptr;

        friend class SceneManager;
    };
}