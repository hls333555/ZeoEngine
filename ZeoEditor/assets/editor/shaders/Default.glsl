// Basic Texture Shader

#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in float a_TexIndex;

layout (std140, binding = 0) uniform Camera
{
    mat4 View;
    mat4 Projection;
    vec3 Position;
}u_Camera;

layout (std140, binding = 1) uniform Model
{
	mat4 Transform;
	mat4 NormalMatrix;
	int EntityID;
}u_Model;

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
	Output.WorldPosition = vec3(u_Model.Transform * vec4(a_Position, 1.0f));
	Output.Normal = mat3(u_Model.NormalMatrix) * a_Normal;
	Output.TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_EntityID = u_Model.EntityID;

	gl_Position = u_Camera.Projection * u_Camera.View * u_Model.Transform * vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout (std140, binding = 2) uniform DirectionalLight
{
	vec4 Color;
	vec3 Position; float Intensity;
	vec3 Direction;
}u_DirectionalLight;


layout (std140, binding = 3) uniform Material
{
	float Shininess;
}u_Material;

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

vec4 CalculateDirectionalLight(vec3 vertexPosition, vec3 vertexNormal)
{
	// Ambient
	vec4 ambientColor = texture(u_DiffuseTexture, Input.TexCoord) * 0.3f;

	// Diffuse
	float diffuse = max(dot(vertexNormal, normalize(-u_DirectionalLight.Direction)), 0.0f);
	vec4 diffuseColor = diffuse * texture(u_DiffuseTexture, Input.TexCoord);

	// Specular
	vec3 viewDirection = normalize(u_DirectionalLight.Position - vertexPosition);
	vec3 reflectDirection = reflect(u_DirectionalLight.Direction, vertexNormal);
	float specular = pow(max(dot(viewDirection, reflectDirection), 0.0f), u_Material.Shininess >= 1.0f ? u_Material.Shininess : 1.0f);
	vec4 specularColor = specular * texture(u_SpecularTexture, Input.TexCoord);

	return (ambientColor + diffuseColor + specularColor) * u_DirectionalLight.Color * u_DirectionalLight.Intensity;
}

void main()
{
	vec4 directionalLight = CalculateDirectionalLight(Input.WorldPosition, normalize(Input.Normal));
	o_Color = directionalLight;
	// Force set opaque mode due to rgb texture's alpha is 0
	o_Color.a = 1.0f;

	o_EntityID = v_EntityID;
}
