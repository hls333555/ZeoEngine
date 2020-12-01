// Basic Texture Shader

#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in vec2 a_TilingFactor;
layout(location = 5) in vec2 a_UVOffset;

out vec4 v_Color;
out vec2 v_TexCoord;
out flat float v_TexIndex;
out vec2 v_TilingFactor;
out vec2 v_UVOffset;

uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.f);
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	v_UVOffset = a_UVOffset;
}

#type fragment
#version 450

in vec4 v_Color;
in vec2 v_TexCoord;
in flat float v_TexIndex;
in vec2 v_TilingFactor;
in vec2 v_UVOffset;

layout(location = 0) out vec4 color;

uniform sampler2D u_Textures[32];

void main()
{
	color = texture(u_Textures[int(v_TexIndex)], vec2(v_TexCoord.x + v_UVOffset.x, v_TexCoord.y + v_UVOffset.y) * v_TilingFactor) * v_Color;
}
