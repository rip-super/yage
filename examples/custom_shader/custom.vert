#version 330 core
layout (location = 0) in vec2  a_pos;
layout (location = 1) in vec4  a_color;
layout (location = 2) in vec2  a_uv;
layout (location = 3) in float a_shape;
layout (location = 4) in float a_param;

uniform mat4 u_view_proj;

out vec4  v_color;
out vec2  v_pos;
out vec2  v_uv;
out float v_shape;
out float v_param;

void main()
{
    gl_Position = u_view_proj * vec4(a_pos, 0.0, 1.0);
    v_color = a_color;
    v_pos = a_pos;
    v_uv = a_uv;
    v_shape = a_shape;
    v_param = a_param;
}