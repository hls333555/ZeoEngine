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
	vec3 Position; float Intensity;
	vec3 Direction;
};

layout (std140, binding = 2) uniform DirectionalLightBlock // Uniform block name should not conflict with struct name
{
	DirectionalLight u_DirectionalLight;
};

struct Material
{
	float Shininess;
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

layout (binding = 0) uniform sampler2D u_DiffuseTexture;
layout (binding = 1) uniform sampler2D u_SpecularTexture;

vec4 CalculateDirectionalLight(vec3 vertexPosition, vec3 vertexNormal, DirectionalLight light, Material material)
{
	// Ambient
	vec4 ambientColor = texture(u_DiffuseTexture, Input.TexCoord);

	// Diffuse
	float diffuse = max(dot(vertexNormal, normalize(-light.Direction)), 0.0f);
	vec4 diffuseColor = diffuse * texture(u_DiffuseTexture, Input.TexCoord);

	// Specular
	vec3 viewDirection = normalize(light.Position - vertexPosition);
	vec3 reflectDirection = reflect(light.Direction, vertexNormal);
	float specular = pow(max(dot(viewDirection, reflectDirection), 0.0f), material.Shininess);
	vec4 specularColor = specular * texture(u_SpecularTexture, Input.TexCoord);

	return (ambientColor + diffuseColor + specularColor) * light.Color * light.Intensity;
}

void main()
{
	vec4 directionalLight = CalculateDirectionalLight(Input.WorldPosition, normalize(Input.Normal), u_DirectionalLight, u_Material);
	o_Color = directionalLight;

	o_EntityID = v_EntityID;
}
