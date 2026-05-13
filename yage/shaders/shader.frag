#version 330 core

in vec4  v_color;
in vec2  v_uv;
in float v_shape;
in float v_param;

out vec4 frag_color;

void main()
{
    if (v_shape < 0.5)
    {
        frag_color = v_color;
        return;
    }

    float dist = length(v_uv);

    if (dist > 1.0) discard;

    if (v_param > 0.0 && dist < v_param) discard;

    float fw = fwidth(dist);
    float outer_alpha = 1.0 - smoothstep(1.0 - fw, 1.0, dist);
    float inner_alpha = (v_param > 0.0)
        ? smoothstep(v_param - fw, v_param, dist)
        : 1.0;

    frag_color = vec4(v_color.rgb, v_color.a * outer_alpha * inner_alpha);
}
