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

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	vec3 CameraPosition;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.WorldPosition = vec3(u_Model.Transform * vec4(a_Position, 1.0f));
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
	bool bCastShadow;
	int ShadowType;
	float DepthBias;
	float NormalBias;
	int PcfLevel;
	float LightSize;
	float NearPlane;
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

layout (std140, binding = 3) uniform LightSpace
{
    mat4 ViewProjection;
}u_LightSpace;

layout (std140, binding = 4) uniform Material
{
	float Shininess;
}u_Material;

struct VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	vec3 CameraPosition;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_ShadowMap;
layout (binding = 1) uniform sampler2DShadow u_PcfShadowMap;
layout (binding = 2) uniform sampler2D u_DiffuseTexture;
layout (binding = 3) uniform sampler2D u_SpecularTexture;

int numBlockerSearchSamples = 64;
int numPCFSamples = 64;
const vec2 Poisson25[25] = vec2[](
    vec2(-0.978698, -0.0884121),
    vec2(-0.841121, 0.521165),
    vec2(-0.71746, -0.50322),
    vec2(-0.702933, 0.903134),
    vec2(-0.663198, 0.15482),
    vec2(-0.495102, -0.232887),
    vec2(-0.364238, -0.961791),
    vec2(-0.345866, -0.564379),
    vec2(-0.325663, 0.64037),
    vec2(-0.182714, 0.321329),
    vec2(-0.142613, -0.0227363),
    vec2(-0.0564287, -0.36729),
    vec2(-0.0185858, 0.918882),
    vec2(0.0381787, -0.728996),
    vec2(0.16599, 0.093112),
    vec2(0.253639, 0.719535),
    vec2(0.369549, -0.655019),
    vec2(0.423627, 0.429975),
    vec2(0.530747, -0.364971),
    vec2(0.566027, -0.940489),
    vec2(0.639332, 0.0284127),
    vec2(0.652089, 0.669668),
    vec2(0.773797, 0.345012),
    vec2(0.968871, 0.840449),
    vec2(0.991882, -0.657338)
);

const vec2 Poisson32[32] = vec2[](
    vec2(-0.975402, -0.0711386),
    vec2(-0.920347, -0.41142),
    vec2(-0.883908, 0.217872),
    vec2(-0.884518, 0.568041),
    vec2(-0.811945, 0.90521),
    vec2(-0.792474, -0.779962),
    vec2(-0.614856, 0.386578),
    vec2(-0.580859, -0.208777),
    vec2(-0.53795, 0.716666),
    vec2(-0.515427, 0.0899991),
    vec2(-0.454634, -0.707938),
    vec2(-0.420942, 0.991272),
    vec2(-0.261147, 0.588488),
    vec2(-0.211219, 0.114841),
    vec2(-0.146336, -0.259194),
    vec2(-0.139439, -0.888668),
    vec2(0.0116886, 0.326395),
    vec2(0.0380566, 0.625477),
    vec2(0.0625935, -0.50853),
    vec2(0.125584, 0.0469069),
    vec2(0.169469, -0.997253),
    vec2(0.320597, 0.291055),
    vec2(0.359172, -0.633717),
    vec2(0.435713, -0.250832),
    vec2(0.507797, -0.916562),
    vec2(0.545763, 0.730216),
    vec2(0.56859, 0.11655),
    vec2(0.743156, -0.505173),
    vec2(0.736442, -0.189734),
    vec2(0.843562, 0.357036),
    vec2(0.865413, 0.763726),
    vec2(0.872005, -0.927)
);

const vec2 Poisson64[64] = vec2[](
    vec2(-0.934812, 0.366741),
    vec2(-0.918943, -0.0941496),
    vec2(-0.873226, 0.62389),
    vec2(-0.8352, 0.937803),
    vec2(-0.822138, -0.281655),
    vec2(-0.812983, 0.10416),
    vec2(-0.786126, -0.767632),
    vec2(-0.739494, -0.535813),
    vec2(-0.681692, 0.284707),
    vec2(-0.61742, -0.234535),
    vec2(-0.601184, 0.562426),
    vec2(-0.607105, 0.847591),
    vec2(-0.581835, -0.00485244),
    vec2(-0.554247, -0.771111),
    vec2(-0.483383, -0.976928),
    vec2(-0.476669, -0.395672),
    vec2(-0.439802, 0.362407),
    vec2(-0.409772, -0.175695),
    vec2(-0.367534, 0.102451),
    vec2(-0.35313, 0.58153),
    vec2(-0.341594, -0.737541),
    vec2(-0.275979, 0.981567),
    vec2(-0.230811, 0.305094),
    vec2(-0.221656, 0.751152),
    vec2(-0.214393, -0.0592364),
    vec2(-0.204932, -0.483566),
    vec2(-0.183569, -0.266274),
    vec2(-0.123936, -0.754448),
    vec2(-0.0859096, 0.118625),
    vec2(-0.0610675, 0.460555),
    vec2(-0.0234687, -0.962523),
    vec2(-0.00485244, -0.373394),
    vec2(0.0213324, 0.760247),
    vec2(0.0359813, -0.0834071),
    vec2(0.0877407, -0.730766),
    vec2(0.14597, 0.281045),
    vec2(0.18186, -0.529649),
    vec2(0.188208, -0.289529),
    vec2(0.212928, 0.063509),
    vec2(0.23661, 0.566027),
    vec2(0.266579, 0.867061),
    vec2(0.320597, -0.883358),
    vec2(0.353557, 0.322733),
    vec2(0.404157, -0.651479),
    vec2(0.410443, -0.413068),
    vec2(0.413556, 0.123325),
    vec2(0.46556, -0.176183),
    vec2(0.49266, 0.55388),
    vec2(0.506333, 0.876888),
    vec2(0.535875, -0.885556),
    vec2(0.615894, 0.0703452),
    vec2(0.637135, -0.637623),
    vec2(0.677236, -0.174291),
    vec2(0.67626, 0.7116),
    vec2(0.686331, -0.389935),
    vec2(0.691031, 0.330729),
    vec2(0.715629, 0.999939),
    vec2(0.8493, -0.0485549),
    vec2(0.863582, -0.85229),
    vec2(0.890622, 0.850581),
    vec2(0.898068, 0.633778),
    vec2(0.92053, -0.355693),
    vec2(0.933348, -0.62981),
    vec2(0.95294, 0.156896)
);

//坐标点转相机空间
//vec3 ShadowCoords(mat4 shadowMapViewProjection)
//{
//	vec4 projectedCoords = shadowMapViewProjection * vec4(vWorldPosition, 1);
//	vec3 shadowCoords = projectedCoords.xyz / projectedCoords.w;
//	shadowCoords = shadowCoords * 0.5 + 0.5;
//	return shadowCoords;
//}

//遮挡范围计算
float SearchWidth(float uvLightSize, float receiverDistance, LightBase base)
{
	return uvLightSize * (receiverDistance - base.NearPlane) / receiverDistance;
}

//遮挡物平均深度查询
float FindBlockerDistance_DirectionalLight(vec3 shadowCoords, float uvLightSize, LightBase base)
{
	int blockers = 0;
	float avgBlockerDistance = 0.0f;
	float searchWidth = SearchWidth(uvLightSize, shadowCoords.z, base);
	float texelSize = 1.0f / textureSize(u_ShadowMap, 0).x;
	for (int i = 0; i < numBlockerSearchSamples; ++i)
	{
		float z = texture(u_ShadowMap, shadowCoords.xy + Poisson64[i] * searchWidth).r;
		if (z < shadowCoords.z - base.DepthBias * texelSize)
		{
			++blockers;
			avgBlockerDistance += z;
		}
	}
	if (blockers > 0)
	{
		return avgBlockerDistance / blockers;
	}
	else
	{
		return -1.0f;
	}
}

//PCF
float PCF_DirectionalLight(vec3 shadowCoords, float uvRadius, LightBase base)
{
	float sum = 0.0f;
	float texelSize = 1.0f / textureSize(u_PcfShadowMap, 0).x;
	for (int i = 0; i < numPCFSamples; ++i)
	{
		float z = 1.0f - texture(u_PcfShadowMap, vec3(shadowCoords.xy + Poisson64[i] * uvRadius, shadowCoords.z - base.DepthBias * texelSize)).r;
		sum += z;
	}
	return sum / numPCFSamples;
}

//软阴影计算
float PCSS_DirectionalLight(vec3 shadowCoords, LightBase base)
{
	// 遮挡物平均深度计算
	float blockerDistance = FindBlockerDistance_DirectionalLight(shadowCoords, base.LightSize, base);
	if (blockerDistance == -1.0f)
	{
		return 0.0f;
	}

	// 半影区域计算
	float penumbraWidth = ((shadowCoords.z - blockerDistance) / blockerDistance) * base.LightSize;

	// PCF
	float uvRadius = penumbraWidth * base.NearPlane / shadowCoords.z;
	return PCF_DirectionalLight(shadowCoords, uvRadius, base);
}

float PCF_DirectionalLight(vec3 projCoords, LightBase base)
{
	float shadow = 0.0f;
	vec2 texelSize = 1.0f / textureSize(u_PcfShadowMap, 0);
	// Hardware PCF
	float pcfBase = (base.PcfLevel - 1.0f) / 2.0f;
	for (float x = -pcfBase; x <= pcfBase; x += 1.0f)
	{
		for (float y = -pcfBase; y <= pcfBase; y += 1.0f)
		{
			shadow += 1.0f - texture(u_PcfShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z - base.DepthBias * texelSize.x));
		}
	}
	shadow /= base.PcfLevel * base.PcfLevel;
	return shadow;
}

float CalculateShadow(LightBase base, vec3 lightDirection, vec3 vertexNormal)
{
	float dotRes = clamp(dot(vertexNormal, normalize(-lightDirection)), 0.0f, 1.0f);
	// Normal bias offset applied to world position before shadow evaluation
    vec3 posOffset = base.NormalBias * (1.0f - dotRes) * vertexNormal;
	vec4 lightSpacePosition = u_LightSpace.ViewProjection * vec4(Input.WorldPosition + posOffset, 1.0f);
	// Perform perspective divide, now in [-1,1] range
    vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
	// Transform to [0,1] range
    projCoords = projCoords * 0.5f + 0.5f;
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	float shadow = 0.0f;
	if (currentDepth <= 1.0f)
	{
		switch (base.ShadowType)
		{
			case 0: break;
			case 1: shadow = PCF_DirectionalLight(projCoords, base); break;
			case 2: shadow = PCSS_DirectionalLight(projCoords, base); break;
		}
	}

    return shadow;
}

vec4 CalculateLightInternal(LightBase base, vec3 lightDirection, vec3 vertexNormal)
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
	float shadow = base.bCastShadow ? CalculateShadow(base, lightDirection, vertexNormal) : 0.0f;

	return (ambientColor + (1.0f - shadow) * (diffuseColor + specularColor)) * base.Color * base.Intensity;
}

vec4 CalculateDirectionalLight(vec3 vertexNormal)
{
	return CalculateLightInternal(u_DirectionalLight.Base, u_DirectionalLight.Direction, vertexNormal);
}

vec4 CalculatePointLight(PointLight pointLight, vec3 vertexNormal)
{
	vec3 lightDirection = Input.WorldPosition - pointLight.Position;
	float lightDistance = length(lightDirection);
	lightDirection = normalize(lightDirection);

	vec4 color = CalculateLightInternal(pointLight.Base, lightDirection, vertexNormal);
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
