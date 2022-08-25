// Screen Space Shadow Shader

#type vertex
#version 450 core

#include "assets/shaders/Uniforms.glsl"

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

struct VertexOutput
{
	vec3 PositionWS;
	vec3 NormalWS;
	vec2 TexCoord;
};

struct CameraOutput
{
	vec3 Position;
	mat4 View;
};

layout (location = 0) out VertexOutput v_VertexOutput;
layout (location = 3) out CameraOutput v_CameraOutput;

void main()
{
	v_VertexOutput.PositionWS = vec3(u_Model.Transform * vec4(a_Position, 1.0f));
	v_VertexOutput.NormalWS = mat3(u_Model.NormalMatrix) * a_Normal;
	v_VertexOutput.TexCoord = a_TexCoord;

	v_CameraOutput.Position = u_Camera.Position;
	v_CameraOutput.View = u_Camera.View;

	gl_Position = u_Camera.Projection * u_Camera.View * vec4(v_VertexOutput.PositionWS, 1.0f);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

[macro](3)SHADOW_TYPE

#define MAX_POINT_LIGHTS 32
#define MAX_SPOT_LIGHTS 32

#define MAX_CASCADE_COUNT 4
#define USE_RECEIVER_PLANE_DEPTH_BIAS 1
#define VISUALIZE_CASCADES 0
#define BLEND_CASCADES 1
#define USE_OPTIMIZED_PCF 0

#define POISSON_SAMPLE_COUNT 64
const vec2 Poisson64[] = {
    vec2(-0.5119625f, -0.4827938f),
    vec2(-0.2171264f, -0.4768726f),
    vec2(-0.7552931f, -0.2426507f),
    vec2(-0.7136765f, -0.4496614f),
    vec2(-0.5938849f, -0.6895654f),
    vec2(-0.3148003f, -0.7047654f),
    vec2(-0.42215f, -0.2024607f),
    vec2(-0.9466816f, -0.2014508f),
    vec2(-0.8409063f, -0.03465778f),
    vec2(-0.6517572f, -0.07476326f),
    vec2(-0.1041822f, -0.02521214f),
    vec2(-0.3042712f, -0.02195431f),
    vec2(-0.5082307f, 0.1079806f),
    vec2(-0.08429877f, -0.2316298f),
    vec2(-0.9879128f, 0.1113683f),
    vec2(-0.3859636f, 0.3363545f),
    vec2(-0.1925334f, 0.1787288f),
    vec2(0.003256182f, 0.138135f),
    vec2(-0.8706837f, 0.3010679f),
    vec2(-0.6982038f, 0.1904326f),
    vec2(0.1975043f, 0.2221317f),
    vec2(0.1507788f, 0.4204168f),
    vec2(0.3514056f, 0.09865579f),
    vec2(0.1558783f, -0.08460935f),
    vec2(-0.0684978f, 0.4461993f),
    vec2(0.3780522f, 0.3478679f),
    vec2(0.3956799f, -0.1469177f),
    vec2(0.5838975f, 0.1054943f),
    vec2(0.6155105f, 0.3245716f),
    vec2(0.3928624f, -0.4417621f),
    vec2(0.1749884f, -0.4202175f),
    vec2(0.6813727f, -0.2424808f),
    vec2(-0.6707711f, 0.4912741f),
    vec2(0.0005130528f, -0.8058334f),
    vec2(0.02703013f, -0.6010728f),
    vec2(-0.1658188f, -0.9695674f),
    vec2(0.4060591f, -0.7100726f),
    vec2(0.7713396f, -0.4713659f),
    vec2(0.573212f, -0.51544f),
    vec2(-0.3448896f, -0.9046497f),
    vec2(0.1268544f, -0.9874692f),
    vec2(0.7418533f, -0.6667366f),
    vec2(0.3492522f, 0.5924662f),
    vec2(0.5679897f, 0.5343465f),
    vec2(0.5663417f, 0.7708698f),
    vec2(0.7375497f, 0.6691415f),
    vec2(0.2271994f, -0.6163502f),
    vec2(0.2312844f, 0.8725659f),
    vec2(0.4216993f, 0.9002838f),
    vec2(0.4262091f, -0.9013284f),
    vec2(0.2001408f, -0.808381f),
    vec2(0.149394f, 0.6650763f),
    vec2(-0.09640376f, 0.9843736f),
    vec2(0.7682328f, -0.07273844f),
    vec2(0.04146584f, 0.8313184f),
    vec2(0.9705266f, -0.1143304f),
    vec2(0.9670017f, 0.1293385f),
    vec2(0.9015037f, -0.3306949f),
    vec2(-0.5085648f, 0.7534177f),
    vec2(0.9055501f, 0.3758393f),
    vec2(0.7599946f, 0.1809109f),
    vec2(-0.2483695f, 0.7942952f),
    vec2(-0.4241052f, 0.5581087f),
    vec2(-0.1020106f, 0.6724468f)
};

struct LightBase
{
	vec4 Color;
	float Intensity;
	bool bCastShadow;
	float DepthBias;
	float NormalBias;
	float FilterSize;
	float LightSize;
};

struct DirectionalLight
{
	LightBase Base;
	vec3 Direction;
	int CascadeCount;
	vec4 CascadeSplits; // DO NOT USE FLOAT ARRAY as float[4] will become vec4[4] due to padding!
	mat4 CascadeReferenceMatrix;
	vec4 CascadeOffsets[MAX_CASCADE_COUNT];
	vec4 CascadeScales[MAX_CASCADE_COUNT];
	float CascadeBlendThreshold;
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

layout (std140, binding = 3) uniform Light
{
	DirectionalLight u_DirectionalLight;
	PointLight u_PointLights[MAX_POINT_LIGHTS];
	SpotLight u_SpotLights[MAX_SPOT_LIGHTS];
	int u_NumPointLights;
	int u_NumSpotLights;
};

struct VertexInput
{
	vec3 PositionWS;
	vec3 NormalWS;
	vec2 TexCoord;
};

struct CameraInput
{
	vec3 Position;
	mat4 View;
};

layout (location = 0) in VertexInput v_VertexInput;
layout (location = 3) in CameraInput v_CameraInput;

layout (binding = 0) uniform sampler2DArray u_ShadowMap;
layout (binding = 1) uniform sampler2DArrayShadow u_ShadowMapPcf;

float Random(vec4 seed)
{
    float dotProduct = dot(seed, vec4(12.9898, 78.233, 45.164, 94.673));
    return fract(sin(dotProduct) * 43758.5453);
}

vec3 GetShadowPosNormalOffset(vec3 normal)
{
    float nDotL = clamp(dot(normal, -u_DirectionalLight.Direction), 0.0f, 1.0f);
    float texelSize = 2.0f / textureSize(u_ShadowMapPcf, 0).x;
    float nmlOffsetScale = 1.0f - nDotL;
    return texelSize * u_DirectionalLight.Base.NormalBias * nmlOffsetScale * normal;
}

#if USE_RECEIVER_PLANE_DEPTH_BIAS
vec2 ComputeReceiverPlaneDepthBias(vec3 texCoordDX, vec3 texCoordDY)
{
    // See: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
    // Chain rule to compute dz/du and dz/dv
    // |dz/du|   |du/dx du/dy|^-T   |dz/dx|
    // |dz/dv| = |dv/dx dv/dy|    * |dz/dy|
    vec2 biasUV = inverse(transpose(mat2(texCoordDX.xy, texCoordDY.xy))) * vec2(texCoordDX.z, texCoordDY.z);
    return biasUV;
}

float GetBiasedZ(float z0, vec2 receiverPlaneDepthBias, vec2 offset)
{
	return z0 + dot(abs(receiverPlaneDepthBias), offset);
}
#endif

float PenumbraRadius(float zReceiver, float zBlocker, float lightSize)
{
    //return abs(lightSize * (zReceiver - zBlocker) / zBlocker);
    return abs(lightSize * (zReceiver - zBlocker));
}

float FindBlockerDistance_DirectionalLight(vec3 shadowCoords, mat2 rotateMatrix, vec2 sampleScale, int cascadeIndex, vec2 receiverPlaneDepthBias, out int blockers)
{
	float avgBlockerDistance = 0.0f;
	for (int i = 0; i < POISSON_SAMPLE_COUNT; ++i)
	{
		vec2 offset = rotateMatrix * Poisson64[i] * sampleScale;
        vec2 uv = shadowCoords.xy + offset;
		float z = texture(u_ShadowMap, vec3(uv, cascadeIndex)).r;
        #if USE_RECEIVER_PLANE_DEPTH_BIAS
            float biasedZ = GetBiasedZ(shadowCoords.z, receiverPlaneDepthBias, offset);
        #else
            float biasedZ = shadowCoords.z;
        #endif
		if (z < biasedZ)
		{
			++blockers;
			avgBlockerDistance += z;
		}
	}
	return avgBlockerDistance;
}

float SampleShadowMapPCFInternal_DirectionalLight(vec3 shadowCoords, float filterRadiusUV, mat2 rotateMatrix, vec2 sampleScale, int cascadeIndex, vec2 receiverPlaneDepthBias)
{
    float sum = 0.0f;
    for (int i = 0; i < POISSON_SAMPLE_COUNT; ++i)
    {
        vec2 offset = rotateMatrix * Poisson64[i] * filterRadiusUV * sampleScale;
        vec2 uv = shadowCoords.xy + offset;
        #if USE_RECEIVER_PLANE_DEPTH_BIAS
            float biasedZ = GetBiasedZ(shadowCoords.z, receiverPlaneDepthBias, offset);
        #else
            float biasedZ = shadowCoords.z;
        #endif
        float z = texture(u_ShadowMapPcf, vec4(uv, cascadeIndex, biasedZ));
        sum += z;
    }
    return sum / POISSON_SAMPLE_COUNT;
}

float SampleShadowMapHardShadow_DirectionalLight(vec3 shadowCoords, int cascadeIndex)
{
	return texture(u_ShadowMapPcf, vec4(shadowCoords.xy, cascadeIndex, shadowCoords.z));
}

#if USE_OPTIMIZED_PCF
float SampleShadowMap_DirectionalLight(vec2 baseUV, float u, float v, vec2 texelSizeInv, int cascadeIndex, float depth, vec2 receiverPlaneDepthBias)
{
    vec2 uv = baseUV + vec2(u, v) * texelSizeInv;

    #if USE_RECEIVER_PLANE_DEPTH_BIAS
        float z = depth + dot(vec2(u, v) * texelSizeInv, receiverPlaneDepthBias);
    #else
        float z = depth;
    #endif

    return texture(u_ShadowMapPcf, vec4(uv, cascadeIndex, z));
}

float SampleShadowMapOptimizedPCF_DirectionalLight(vec3 shadowCoords, int cascadeIndex, const vec2 texelSize, vec2 receiverPlaneDepthBias)
{
    const vec2 texelSizeInv = 1.0 / texelSize;

	vec2 uv = shadowCoords.xy * texelSize; // 1 unit - 1 texel

    vec2 baseUV;
    baseUV.x = floor(uv.x + 0.5f);
    baseUV.y = floor(uv.y + 0.5f);

    float s = (uv.x + 0.5f - baseUV.x);
    float t = (uv.y + 0.5f - baseUV.y);

    baseUV -= vec2(0.5f, 0.5f);
    baseUV *= texelSizeInv;

    float sum = 0;

	float uw0 = (4 - 3 * s);
    float uw1 = 7;
    float uw2 = (1 + 3 * s);

    float u0 = (3 - 2 * s) / uw0 - 2;
    float u1 = (3 + s) / uw1;
    float u2 = s / uw2 + 2;

    float vw0 = (4 - 3 * t);
    float vw1 = 7;
    float vw2 = (1 + 3 * t);

    float v0 = (3 - 2 * t) / vw0 - 2;
    float v1 = (3 + t) / vw1;
    float v2 = t / vw2 + 2;

    sum += uw0 * vw0 * SampleShadowMap_DirectionalLight(baseUV, u0, v0, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);
    sum += uw1 * vw0 * SampleShadowMap_DirectionalLight(baseUV, u1, v0, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);
    sum += uw2 * vw0 * SampleShadowMap_DirectionalLight(baseUV, u2, v0, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);

    sum += uw0 * vw1 * SampleShadowMap_DirectionalLight(baseUV, u0, v1, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);
    sum += uw1 * vw1 * SampleShadowMap_DirectionalLight(baseUV, u1, v1, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);
    sum += uw2 * vw1 * SampleShadowMap_DirectionalLight(baseUV, u2, v1, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);

    sum += uw0 * vw2 * SampleShadowMap_DirectionalLight(baseUV, u0, v2, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);
    sum += uw1 * vw2 * SampleShadowMap_DirectionalLight(baseUV, u1, v2, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);
    sum += uw2 * vw2 * SampleShadowMap_DirectionalLight(baseUV, u2, v2, texelSizeInv, cascadeIndex, shadowCoords.z, receiverPlaneDepthBias);

    return sum * 1.0f / 144;
}
#endif

float SampleShadowMapRandomDiscPCF_DirectionalLight(vec3 shadowCoords, int cascadeIndex, mat2 rotateMatrix, const vec2 texelSize, vec2 receiverPlaneDepthBias)
{
    const vec2 filterSize = u_DirectionalLight.Base.FilterSize.xx * abs(u_DirectionalLight.CascadeScales[cascadeIndex].xy);
    const vec2 sampleScale = max(vec2(0.0f), 0.5f * abs(filterSize) / texelSize);

	return SampleShadowMapPCFInternal_DirectionalLight(shadowCoords, 1.0f, rotateMatrix, sampleScale, cascadeIndex, receiverPlaneDepthBias);
}

float SampleShadowMapPCSS_DirectionalLight(vec3 shadowCoords, int cascadeIndex, mat2 rotateMatrix, const vec2 texelSize, vec2 receiverPlaneDepthBias)
{
	const vec2 filterSize = u_DirectionalLight.Base.FilterSize.xx * abs(u_DirectionalLight.CascadeScales[cascadeIndex].xy);
    const vec2 sampleScale = max(vec2(0.0f), 0.5f * abs(filterSize) / texelSize);

	int blockers = 0;
	float avgBlockerDistance = FindBlockerDistance_DirectionalLight(shadowCoords, rotateMatrix, sampleScale, cascadeIndex, receiverPlaneDepthBias, blockers);
	if (blockers == 0)
	{
		return 1.0f;
	}
	else if (blockers == POISSON_SAMPLE_COUNT)
	{
		return 0.0f;
	}

	avgBlockerDistance /= blockers;
	float penumbraRadius = PenumbraRadius(shadowCoords.z, avgBlockerDistance, u_DirectionalLight.Base.LightSize);
	float filterRadiusUV = clamp(penumbraRadius / POISSON_SAMPLE_COUNT, 0.1f, 1.0f);

	return SampleShadowMapPCFInternal_DirectionalLight(shadowCoords, filterRadiusUV, rotateMatrix, sampleScale, cascadeIndex, receiverPlaneDepthBias);
}

float SampleShadowCascade(vec3 shadowCoords, vec3 texCoordDX, vec3 texCoordDY, int cascadeIndex, mat2 rotateMatrix)
{
    shadowCoords += u_DirectionalLight.CascadeOffsets[cascadeIndex].xyz;
    shadowCoords *= u_DirectionalLight.CascadeScales[cascadeIndex].xyz;

    texCoordDX *= u_DirectionalLight.CascadeScales[cascadeIndex].xyz;
    texCoordDY *= u_DirectionalLight.CascadeScales[cascadeIndex].xyz;

    const vec2 texelSize = textureSize(u_ShadowMapPcf, 0).xy;

    #if USE_RECEIVER_PLANE_DEPTH_BIAS
        vec2 receiverPlaneDepthBias = ComputeReceiverPlaneDepthBias(texCoordDX, texCoordDY);
	    const vec2 texelSizeInv = 1.0f / texelSize;
	    float fractionalSamplingError = 2 * dot(texelSizeInv, abs(receiverPlaneDepthBias));
	    const float RECEIVER_PLANE_MIN_FRACTIONAL_ERROR = 0.025f;
	    fractionalSamplingError = min(fractionalSamplingError, RECEIVER_PLANE_MIN_FRACTIONAL_ERROR);
	    float bias = fractionalSamplingError;
    #else
	    vec2 receiverPlaneDepthBias = vec2(0.0f);
        float bias = u_DirectionalLight.Base.DepthBias;
    #endif

    shadowCoords.z = clamp(shadowCoords.z - bias, 0.0f, 1.0f); // clamp to avoid buggy results at infinity

    #if SHADOW_TYPE == 1
        #if USE_OPTIMIZED_PCF
			return SampleShadowMapOptimizedPCF_DirectionalLight(shadowCoords, cascadeIndex, texelSize, receiverPlaneDepthBias);
        #else
			return SampleShadowMapRandomDiscPCF_DirectionalLight(shadowCoords, cascadeIndex, rotateMatrix, texelSize, receiverPlaneDepthBias);
        #endif
    #elif SHADOW_TYPE == 2
        return SampleShadowMapPCSS_DirectionalLight(shadowCoords, cascadeIndex, rotateMatrix, texelSize, receiverPlaneDepthBias);
    #else
        return SampleShadowMapHardShadow_DirectionalLight(shadowCoords, cascadeIndex);
    #endif
}

vec3 ShadowVisibility(vec3 normal)
{
	vec4 shadowPos = u_DirectionalLight.CascadeReferenceMatrix * vec4(v_VertexInput.PositionWS, 1.0f);
	vec4 posVS = v_CameraInput.View * vec4(v_VertexInput.PositionWS, 1.0f);

    int cascadeIndex = u_DirectionalLight.CascadeCount - 1;
	// Projection base cascade selection
	for (int i = u_DirectionalLight.CascadeCount - 1; i >= 0; --i)
	{
		// Select based on whether or not the pixel is inside the projection
		vec3 uv = shadowPos.xyz + u_DirectionalLight.CascadeOffsets[i].xyz;
		uv *= u_DirectionalLight.CascadeScales[i].xyz;
		uv = abs(uv - 0.5f);
		if (uv.x <= 0.5f && uv.y <= 0.5f && uv.z <= 0.5f)
		{
			cascadeIndex = i;
		}
	}

    vec3 offset = GetShadowPosNormalOffset(normal);
	const vec3 shadowCoords = (u_DirectionalLight.CascadeReferenceMatrix * vec4(v_VertexInput.PositionWS + offset, 1.0f)).xyz;

    vec3 texCoordDX = dFdxFine(shadowCoords);
    vec3 texCoordDY = dFdyFine(shadowCoords);

	float theta = 2.0 * 3.1415926535 * Random(vec4(v_VertexInput.PositionWS, v_VertexInput.PositionWS.z));
    mat2 rotateMatrix = mat2(cos(theta), sin(theta), -sin(theta), cos(theta));

    float shadow = SampleShadowCascade(shadowCoords, texCoordDX, texCoordDY, cascadeIndex, rotateMatrix);

#if BLEND_CASCADES
    // Sample the next cascade, and blend between the two results to smooth the transition
    const float blendThreshold = u_DirectionalLight.CascadeBlendThreshold;

    float nextSplit = u_DirectionalLight.CascadeSplits[cascadeIndex];
    float splitSize = cascadeIndex == 0 ? nextSplit : nextSplit - u_DirectionalLight.CascadeSplits[cascadeIndex - 1];
    float fadeFactor = (nextSplit - posVS.z) / splitSize;

	vec3 cascadePos = shadowPos.xyz + u_DirectionalLight.CascadeOffsets[cascadeIndex].xyz;
	cascadePos *= u_DirectionalLight.CascadeScales[cascadeIndex].xyz;
	cascadePos = abs(cascadePos * 2.0f - 1.0f);
	float distToEdge = 1.0f - max(max(cascadePos.x, cascadePos.y), cascadePos.z);
	fadeFactor = max(distToEdge, fadeFactor);

	if (fadeFactor <= blendThreshold && cascadeIndex != u_DirectionalLight.CascadeCount - 1)
    {
		float nextShadow = SampleShadowCascade(shadowCoords, texCoordDX, texCoordDY, cascadeIndex + 1, rotateMatrix);
        float alpha = smoothstep(0.0f, blendThreshold, fadeFactor);
        shadow = mix(nextShadow, shadow, alpha);
    }
#endif

    vec3 cascadeColor = vec3(1.0f);

#if VISUALIZE_CASCADES
    const vec3 cascadeColors[MAX_CASCADE_COUNT] = {
        vec3(1.0f, 0.0f, 0.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(1.0f, 1.0f, 0.0f)
    };

    cascadeColor = cascadeColors[cascadeIndex];
#endif

    return cascadeColor * shadow;
}

vec4 CalculateDirectionalLight(vec3 normal)
{
	if (u_DirectionalLight.Base.bCastShadow)
	{
		return vec4(ShadowVisibility(normal), 1.0f);
	}

	return vec4(1.0f);
}

vec4 CalculatePointLight(PointLight pointLight, vec3 normal)
{
	return vec4(0.0f);
}

vec4 CalculateSpotLight(SpotLight spotLight, vec3 normal)
{
	return vec4(0.0f);
}

void main()
{
	// Normalize after interpolation
	vec3 normalWS = normalize(v_VertexInput.NormalWS);

	vec4 totalLight = CalculateDirectionalLight(normalWS);
	for (int i = 0; i < u_NumPointLights; ++i)
	{
		totalLight += CalculatePointLight(u_PointLights[i], normalWS);
	}
	for (int i = 0; i < u_NumSpotLights; ++i)
	{
		totalLight += CalculateSpotLight(u_SpotLights[i], normalWS);
	}

	o_Color = totalLight;
}
