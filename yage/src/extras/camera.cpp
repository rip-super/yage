#include <yage/extras/camera.h>
#include <glm/gtc/matrix_transform.hpp>

namespace yage
{
    void Camera::SetPosition(glm::vec2 pos)
    {
        position = pos;
    }

    glm::vec2 Camera::GetPosition() const
    {
        return position;
    }

    void Camera::SetZoom(float z)
    {
        zoom = z;
    }

    float Camera::GetZoom() const
    {
        return zoom;
    }

    void Camera::Follow(glm::vec2 target, float sharpness, float dt)
    {
        position = glm::mix(position, target, sharpness * dt);
    }

    glm::mat4 Camera::GetViewProjection(const Window &window) const
    {
        glm::ivec2 size = window.GetSize();
        float width = size.x / zoom;
        float height = size.y / zoom;

        glm::mat4 projection = glm::ortho(-width / 2.0f, width / 2.0f, height / 2.0f, -height / 2.0f, -1.0f, 1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));

        return projection * view;
    }

    glm::vec2 Camera::ScreenToWorld(glm::vec2 screen_pos, const Window &window) const
    {
        glm::ivec2 size = window.GetSize();
        float width = size.x / zoom;
        float height = size.y / zoom;

        float ndc_x = (screen_pos.x / size.x) * 2.0f - 1.0f;
        float ndc_y = (screen_pos.y / size.y) * 2.0f - 1.0f;

        return position + glm::vec2(ndc_x * width / 2.0f, ndc_y * height / 2.0f);
    }

    glm::vec2 Camera::WorldToScreen(glm::vec2 world_pos, const Window &window) const
    {
        glm::ivec2 size = window.GetSize();
        float width = size.x / zoom;
        float height = size.y / zoom;

        glm::vec2 relative = world_pos - position;
        float ndc_x = (relative.x / (width / 2.0f)) * 0.5f + 0.5f;
        float ndc_y = (relative.y / (height / 2.0f)) * 0.5f + 0.5f;

        return {ndc_x * size.x, ndc_y * size.y};
    }
}
