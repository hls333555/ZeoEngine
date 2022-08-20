// Horizontal Blur Quad Shader

#type vertex
#version 450 core

layout(location = 0) out vec2 v_Uv;

void main() 
{
    float x = float(((uint(gl_VertexIndex) + 2u) / 3u) % 2u);
    float y = float(((uint(gl_VertexIndex) + 1u) / 3u) % 2u);

    gl_Position = vec4(-1.0f + x * 2.0f, -1.0f + y * 2.0f, 0.0f, 1.0f);
    v_Uv = vec2(x, y);
}

#type fragment
#version 450 core

#include "assets/shaders/Uniforms.glsl"

layout(location = 0) in vec2 v_Uv;
layout (binding = 0) uniform sampler2D u_ScreenSpaceShadowMap;

layout(location = 0) out vec4 o_Color;

void main()
{
    float shadow = 0;
    float weight = 0;
    int r = 3;

    for (int i = -r; i <= r; ++i)
    {
        vec2 offset = vec2(i, 0) / vec2(u_Global.ScreenSize.y, u_Global.ScreenSize.y);
        vec2 uv_sample = v_Uv + offset;
        shadow += texture(u_ScreenSpaceShadowMap, uv_sample).r;
        weight += 1;
    }
    shadow /= weight;

    o_Color = vec4(vec3(shadow), 1.0f);
}
