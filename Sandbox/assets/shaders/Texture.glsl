// Basic Texture Shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.f);
	v_TexCoord = a_TexCoord;
}

#type fragment
#version 330 core

in vec2 v_TexCoord;

layout(location = 0) out vec4 color;

uniform vec4 u_Color;
uniform vec2 u_TilingFactor;
uniform vec2 u_UVOffset;
uniform sampler2D u_Texture;

void main()
{
	color = texture(u_Texture, vec2(v_TexCoord.x + u_UVOffset.x, v_TexCoord.y + u_UVOffset.y) * u_TilingFactor) * u_Color;
}
