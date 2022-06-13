// Post processing Quad Shader

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

layout(location = 0) in vec2 v_Uv;
layout (binding = 4) uniform sampler2D u_SceneTexture;

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = texture(u_SceneTexture, v_Uv);
}
