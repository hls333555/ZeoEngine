// Basic Texture Shader

#type vertex
#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

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

layout (std140, binding = 3) uniform LightSpace
{
    mat4 ViewProjection;
}u_LightSpace;

struct VertexOutput
{
	vec3 WorldPosition;
	vec4 LightSpacePosition;
	vec3 Normal;
	vec2 TexCoord;
	vec3 CameraPosition;
};

layout (location = 0) out VertexOutput Output;
layout (location = 5) out flat int v_EntityID;

void main()
{
	Output.WorldPosition = vec3(u_Model.Transform * vec4(a_Position, 1.0f));
	Output.LightSpacePosition = u_LightSpace.ViewProjection * vec4(Output.WorldPosition, 1.0f);
	Output.Normal = mat3(u_Model.NormalMatrix) * a_Normal;
	Output.TexCoord = a_TexCoord;
	Output.CameraPosition = u_Camera.Position;
	v_EntityID = u_Model.EntityID;

	gl_Position = u_Camera.Projection * u_Camera.View * vec4(Output.WorldPosition, 1.0f);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_EntityID;

const int MAX_POINT_LIGHTS = 32;
const int MAX_SPOT_LIGHTS = 32;

struct LightBase
{
	vec4 Color;
	float Intensity;
};

struct DirectionalLight
{
	LightBase Base;
	vec3 Direction;
};

struct PointLight
{
	LightBase Base;
	vec3 Position;
	float Radius;
};

struct SpotLight
{
	PointLight Base;
	vec3 Direction;
	float Cutoff;
};

layout (std140, binding = 2) uniform Light
{
	DirectionalLight u_DirectionalLight;
	PointLight u_PointLights[MAX_POINT_LIGHTS];
	SpotLight u_SpotLights[MAX_SPOT_LIGHTS];
	int u_NumPointLights;
	int u_NumSpotLights;
};

layout (std140, binding = 4) uniform Material
{
	float Shininess;
}u_Material;

struct VertexOutput
{
	vec3 WorldPosition;
	vec4 LightSpacePosition;
	vec3 Normal;
	vec2 TexCoord;
	vec3 CameraPosition;
};

layout (location = 0) in VertexOutput Input;
layout (location = 5) in flat int v_EntityID;

layout (binding = 0) uniform sampler2DShadow u_ShadowMap;
layout (binding = 1) uniform sampler2D u_DiffuseTexture;
layout (binding = 2) uniform sampler2D u_SpecularTexture;

float CalculateShadow()
{
	// Perform perspective divide, now in [-1,1] range
    vec3 projCoords = Input.LightSpacePosition.xyz / Input.LightSpacePosition.w;
	// Transform to [0,1] range
    projCoords = projCoords * 0.5f + 0.5f;
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	float shadow = 0.0f;
	if (currentDepth <= 1.0f)
	{
		vec2 texelSize = 1.0f / textureSize(u_ShadowMap, 0);
		// Hardware PCF
		for (float x = -1.5f; x <= 1.5f; x += 1.0f)
		{
			for (float y = -1.5f; y <= 1.5f; y += 1.0f)
			{
				// No bias needed! Front face culling solves those artifacts!
				shadow += 1.0f - texture(u_ShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z));
			}
		}
		shadow /= 16.0f;
	}

    return shadow;
}

vec4 CalculateLightInternal(LightBase base, vec3 lightDirection, vec3 vertexNormal, bool bCastShadow)
{
	// Ambient
	vec4 ambientColor = texture(u_DiffuseTexture, Input.TexCoord) * 0.3f;

	// Diffuse
	float diffuse = max(dot(vertexNormal, normalize(-lightDirection)), 0.0f);
	vec4 diffuseColor = diffuse * texture(u_DiffuseTexture, Input.TexCoord);

	// Specular
	vec3 viewDirection = normalize(Input.CameraPosition - Input.WorldPosition);
	vec3 reflectDirection = reflect(lightDirection, vertexNormal);
	float specular = pow(max(dot(viewDirection, reflectDirection), 0.0f), u_Material.Shininess >= 1.0f ? u_Material.Shininess : 1.0f);
	vec4 specularColor = specular * texture(u_SpecularTexture, Input.TexCoord);

	// Shadow
	float shadow = bCastShadow ? CalculateShadow() : 0.0f;

	return (ambientColor + (1.0f - shadow) * (diffuseColor + specularColor)) * base.Color * base.Intensity;
}

vec4 CalculateDirectionalLight(vec3 vertexNormal)
{
	return CalculateLightInternal(u_DirectionalLight.Base, u_DirectionalLight.Direction, vertexNormal, true);
}

vec4 CalculatePointLight(PointLight pointLight, vec3 vertexNormal)
{
	vec3 lightDirection = Input.WorldPosition - pointLight.Position;
	float lightDistance = length(lightDirection);
	lightDirection = normalize(lightDirection);

	vec4 color = CalculateLightInternal(pointLight.Base, lightDirection, vertexNormal, false);
	float normalizedDistance = lightDistance / pointLight.Radius;
	float attenuation = clamp(1.0f / (1.0f + 25.0f * normalizedDistance * normalizedDistance) *
								clamp((1.0f - normalizedDistance) * 5.0f, 0.0f, 1.0f)
							, 0.0f, 1.0f);
	return color * attenuation;
}

vec4 CalculateSpotLight(SpotLight spotLight, vec3 vertexNormal)
{
	vec3 lightDirection = normalize(Input.WorldPosition - spotLight.Base.Position);
	float spotFactor = dot(lightDirection, spotLight.Direction);

	if (spotFactor > spotLight.Cutoff)
	{
		vec4 color = CalculatePointLight(spotLight.Base, vertexNormal);
		float intensity = (1.0f - (1.0f - spotFactor) / (1.0f - spotLight.Cutoff));
		return color * intensity;
	}
	else
	{
		return vec4(0.0f);
	}
}

void main()
{
	vec3 normal = normalize(Input.Normal);
	vec4 totalLight = CalculateDirectionalLight(normal);
	for (int i = 0; i < u_NumPointLights; ++i)
	{
		totalLight += CalculatePointLight(u_PointLights[i], normal);
	}
	for (int i = 0; i < u_NumSpotLights; ++i)
	{
		totalLight += CalculateSpotLight(u_SpotLights[i], normal);
	}
	o_Color = totalLight;
	// Force set opaque mode due to rgb texture's alpha is 0
	o_Color.a = 1.0f;

	o_EntityID = vec4(v_EntityID, 0.0f, 0.0f, 1.0f);
}
