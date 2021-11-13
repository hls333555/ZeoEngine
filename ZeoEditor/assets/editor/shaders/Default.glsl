// Basic Texture Shader

#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in float a_TexIndex;

layout (std140, binding = 0) uniform CameraBlock
{
	mat4 u_ViewProjection;
};

layout (std140, binding = 1) uniform ModelBlock
{
	mat4 u_Transform;
	mat4 u_NormalMatrix;
	int u_EntityID;
};

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;
layout (location = 3) out flat float v_TexIndex; // flat variables cannot be in a struct
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0f));
	Output.Normal = mat3(u_NormalMatrix) * a_Normal;
	Output.TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_EntityID = u_EntityID;

	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct DirectionalLight
{
	vec4 Color;
	vec3 Direction;
	float Intensity;
};

layout (std140, binding = 2) uniform DirectionalLightBlock // Uniform block name should not conflict with struct name
{
	DirectionalLight u_DirectionalLight;
};

struct Material
{
	vec4 AmbientColor;
};

layout (std140, binding = 3) uniform MaterialBlock
{
	Material u_Material;
};

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
};

layout (location = 0) in VertexOutput Input;
layout (location = 3) in flat float v_TexIndex;
layout (location = 4) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

vec4 CalculateDirectionalLight(vec3 vertexPosition, vec3 vertexNormal, DirectionalLight light)
{
	float intensity = light.Intensity * max(dot(vertexNormal, normalize(-light.Direction)), 0.0f);
	return light.Color * intensity;
}

void main()
{
	vec4 texColor = vec4(1.0f);
	switch(int(v_TexIndex))
	{
		case  0: texColor *= texture(u_Textures[ 0], Input.TexCoord); break;
		case  1: texColor *= texture(u_Textures[ 1], Input.TexCoord); break;
		case  2: texColor *= texture(u_Textures[ 2], Input.TexCoord); break;
		case  3: texColor *= texture(u_Textures[ 3], Input.TexCoord); break;
		case  4: texColor *= texture(u_Textures[ 4], Input.TexCoord); break;
		case  5: texColor *= texture(u_Textures[ 5], Input.TexCoord); break;
		case  6: texColor *= texture(u_Textures[ 6], Input.TexCoord); break;
		case  7: texColor *= texture(u_Textures[ 7], Input.TexCoord); break;
		case  8: texColor *= texture(u_Textures[ 8], Input.TexCoord); break;
		case  9: texColor *= texture(u_Textures[ 9], Input.TexCoord); break;
		case 10: texColor *= texture(u_Textures[10], Input.TexCoord); break;
		case 11: texColor *= texture(u_Textures[11], Input.TexCoord); break;
		case 12: texColor *= texture(u_Textures[12], Input.TexCoord); break;
		case 13: texColor *= texture(u_Textures[13], Input.TexCoord); break;
		case 14: texColor *= texture(u_Textures[14], Input.TexCoord); break;
		case 15: texColor *= texture(u_Textures[15], Input.TexCoord); break;
		case 16: texColor *= texture(u_Textures[16], Input.TexCoord); break;
		case 17: texColor *= texture(u_Textures[17], Input.TexCoord); break;
		case 18: texColor *= texture(u_Textures[18], Input.TexCoord); break;
		case 19: texColor *= texture(u_Textures[19], Input.TexCoord); break;
		case 20: texColor *= texture(u_Textures[20], Input.TexCoord); break;
		case 21: texColor *= texture(u_Textures[21], Input.TexCoord); break;
		case 22: texColor *= texture(u_Textures[22], Input.TexCoord); break;
		case 23: texColor *= texture(u_Textures[23], Input.TexCoord); break;
		case 24: texColor *= texture(u_Textures[24], Input.TexCoord); break;
		case 25: texColor *= texture(u_Textures[25], Input.TexCoord); break;
		case 26: texColor *= texture(u_Textures[26], Input.TexCoord); break;
		case 27: texColor *= texture(u_Textures[27], Input.TexCoord); break;
		case 28: texColor *= texture(u_Textures[28], Input.TexCoord); break;
		case 29: texColor *= texture(u_Textures[29], Input.TexCoord); break;
		case 30: texColor *= texture(u_Textures[30], Input.TexCoord); break;
		case 31: texColor *= texture(u_Textures[31], Input.TexCoord); break;
	}
	vec4 directionalLight = CalculateDirectionalLight(Input.WorldPosition, normalize(Input.Normal), u_DirectionalLight);
	o_Color = vec4(1.0f) * directionalLight;

	o_EntityID = v_EntityID;
}
