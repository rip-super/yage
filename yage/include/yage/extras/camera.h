#pragma once
#include <yage/utils.h>
#include <yage/window.h>
#include <glm/glm.hpp>

namespace yage
{
    class Camera
    {
    public:
        void SetPosition(glm::vec2 pos);
        glm::vec2 GetPosition() const;

        void SetZoom(float zoom);
        float GetZoom() const;

        void Follow(glm::vec2 target, float sharpness, float dt);

        glm::mat4 GetViewProjection(const Window &window) const;

        glm::vec2 ScreenToWorld(glm::vec2 screen_pos, const Window &window) const;
        glm::vec2 WorldToScreen(glm::vec2 world_pos, const Window &window) const;

    private:
        glm::vec2 position = {0.0f, 0.0f};
        float zoom = 1.0f;
    };
}