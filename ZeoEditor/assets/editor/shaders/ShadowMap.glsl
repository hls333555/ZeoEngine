// Shadow Map Shader

#type vertex
#version 450 core

#include "assets/editor/shaders/Uniforms.glsl"

layout (location = 0) in vec3 a_Position;

void main()
{
	gl_Position = u_Model.Transform * vec4(a_Position, 1.0f);
}

#type geometry
#version 450 core

// TODO:
#define MAX_CASCADE_COUNT 4

layout(triangles, invocations = MAX_CASCADE_COUNT) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 4) uniform ShadowCamera
{
    mat4 ViewProjection[MAX_CASCADE_COUNT];
}u_ShadowCamera;

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        gl_Position = u_ShadowCamera.ViewProjection[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}

#type fragment
#version 450 core

void main()
{
}
