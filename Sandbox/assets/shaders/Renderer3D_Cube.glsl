// 3D Cube Renderer Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec2 a_TexCoord;
layout(location = 4) in float a_TexIndex;

uniform mat4 u_ViewProjection;

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat float v_TexIndex;

void main()
{
	Output.Position = a_Position;
	Output.Normal = a_Normal;
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;

	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat float v_TexIndex;

uniform sampler2D u_Textures[32];

void main()
{
	// Simple directional lighting
	vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
	vec3 normal = normalize(Input.Normal);
	float diff = max(dot(normal, lightDir), 0.0);
	
	// Ambient + diffuse
	float ambient = 0.3;
	float lighting = ambient + (1.0 - ambient) * diff;
	
	// Sample texture
	vec4 texColor = texture(u_Textures[int(v_TexIndex)], Input.TexCoord);
	
	// Final color
	color = vec4(texColor.rgb * Input.Color.rgb * lighting, texColor.a * Input.Color.a);
}
