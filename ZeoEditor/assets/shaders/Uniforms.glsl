// Shared uniform buffers

layout (std140, binding = 0) uniform Global
{
    vec2 ScreenSize;
}u_Global;

layout (std140, binding = 1) uniform Camera
{
    mat4 View;
    mat4 Projection;
    vec3 Position;
}u_Camera;

layout (std140, binding = 2) uniform Model
{
	mat4 Transform;
	mat4 NormalMatrix;
	int EntityID;
}u_Model;
