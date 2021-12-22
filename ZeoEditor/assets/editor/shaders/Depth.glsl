// Simple Depth Shader

#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;

layout (std140, binding = 3) uniform LightSpace
{
    mat4 ViewProjection;
}u_LightSpace;

layout (std140, binding = 1) uniform Model
{
	mat4 Transform;
	mat4 NormalMatrix;
	int EntityID;
}u_Model;

void main()
{
	gl_Position = u_LightSpace.ViewProjection * u_Model.Transform * vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

void main()
{
}
