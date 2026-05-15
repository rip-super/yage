#version 330 core

in vec4  v_color;
in vec2  v_uv;
in float v_shape;
in float v_param;

uniform sampler2D u_tex;

out vec4 frag_color;

void main()
{
    vec2 tex_uv = (v_shape > 0.5) ? v_uv * 0.5 + 0.5 : v_uv;
    vec4 tex_color = texture(u_tex, tex_uv) * v_color;

    if (v_shape < 0.5)
    {
        frag_color = tex_color;
        return;
    }

    float dist = length(v_uv);

    if (dist > 1.0) discard;

    if (v_param > 0.0 && dist < v_param) discard;

    float fw = fwidth(dist);
    float outer_alpha = 1.0 - smoothstep(1.0 - fw, 1.0, dist);
    float inner_alpha = (v_param > 0.0) ? smoothstep(v_param - fw, v_param, dist) : 1.0;

    frag_color = vec4(tex_color.rgb, tex_color.a * outer_alpha * inner_alpha);
}