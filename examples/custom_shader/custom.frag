#version 330 core

in vec4 v_color;
in vec2 v_pos;
in vec2 v_uv;
in float v_shape;
in float v_param;

uniform float u_time;

out vec4 frag_color;

void main()
{
    vec4 color = v_color;

    if (v_shape > 0.5)
    {
        float dist = length(v_uv);
        if (dist > 1.0) discard;
        if (v_param > 0.0 && dist < v_param) discard;

        float fw = fwidth(dist);
        float outer = 1.0 - smoothstep(1.0 - fw, 1.0, dist);
        float inner = (v_param > 0.0) ? smoothstep(v_param - fw, v_param, dist) : 1.0;
        color.a *= outer * inner;
    }

    float wave = sin(v_pos.x * 0.02 + u_time * 2.0) * 0.5 + 0.5;
    vec3 shifted = vec3(
        color.r * wave + color.g * (1.0 - wave),
        color.g * wave + color.b * (1.0 - wave),
        color.b * wave + color.r * (1.0 - wave)
    );

    frag_color = vec4(shifted, color.a);
}