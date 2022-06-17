// Quad Shader

#type vertex
#version 450 core

#include "assets/editor/shaders/Uniforms.glsl"
#include "assets/editor/shaders/Common.glsl"

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec2 a_TilingFactor;
layout (location = 4) in vec2 a_UvOffset;
layout (location = 5) in float a_TexIndex;
layout (location = 6) in int a_EntityID;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	vec2 TilingFactor;
	vec2 UvOffset;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat float v_TexIndex;
layout (location = 5) out flat int v_EntityID;

void main()
{
	Output.Color = SrgbToLinear(a_Color);
	Output.TexCoord = a_TexCoord;
	Output.TilingFactor = a_TilingFactor;
	Output.UvOffset = a_UvOffset;
	v_TexIndex = a_TexIndex;
	v_EntityID = a_EntityID;

	gl_Position = u_Camera.Projection * u_Camera.View * vec4(a_Position, 1.0f);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_EntityID;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	vec2 TilingFactor;
	vec2 UvOffset;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat float v_TexIndex;
layout (location = 5) in flat int v_EntityID;

layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
	vec4 texColor = Input.Color;
	switch(int(v_TexIndex))
	{
		case  0: texColor *= texture(u_Textures[ 0], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  1: texColor *= texture(u_Textures[ 1], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  2: texColor *= texture(u_Textures[ 2], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  3: texColor *= texture(u_Textures[ 3], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  4: texColor *= texture(u_Textures[ 4], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  5: texColor *= texture(u_Textures[ 5], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  6: texColor *= texture(u_Textures[ 6], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  7: texColor *= texture(u_Textures[ 7], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  8: texColor *= texture(u_Textures[ 8], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case  9: texColor *= texture(u_Textures[ 9], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 10: texColor *= texture(u_Textures[10], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 11: texColor *= texture(u_Textures[11], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 12: texColor *= texture(u_Textures[12], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 13: texColor *= texture(u_Textures[13], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 14: texColor *= texture(u_Textures[14], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 15: texColor *= texture(u_Textures[15], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 16: texColor *= texture(u_Textures[16], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 17: texColor *= texture(u_Textures[17], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 18: texColor *= texture(u_Textures[18], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 19: texColor *= texture(u_Textures[19], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 20: texColor *= texture(u_Textures[20], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 21: texColor *= texture(u_Textures[21], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 22: texColor *= texture(u_Textures[22], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 23: texColor *= texture(u_Textures[23], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 24: texColor *= texture(u_Textures[24], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 25: texColor *= texture(u_Textures[25], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 26: texColor *= texture(u_Textures[26], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 27: texColor *= texture(u_Textures[27], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 28: texColor *= texture(u_Textures[28], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 29: texColor *= texture(u_Textures[29], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 30: texColor *= texture(u_Textures[30], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
		case 31: texColor *= texture(u_Textures[31], vec2(Input.TexCoord.x + Input.UvOffset.x, Input.TexCoord.y + Input.UvOffset.y) * Input.TilingFactor); break;
	}
	if (texColor.a < 0.5f) discard; // Due to GL_LINEAR filtering, we should not discard 0 alpha pixels only

	o_Color = texColor;

	// We do not want to override ID buffer value if current pixel is transparent
	o_EntityID = vec4(v_EntityID, 0.0f, 0.0f, o_Color.a == 0.0f ? 0.0f : 1.0f);
}
