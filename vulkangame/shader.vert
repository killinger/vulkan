#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uni {
	mat4 mvp;
} unis;

layout(binding = 2) uniform texas {
	vec4 textrans;
} tex;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	vec2 nej;
	nej.x = (inTexCoord.x * tex.textrans.x) + (tex.textrans.z * tex.textrans.x);
	nej.y = (inTexCoord.y * tex.textrans.y) + (tex.textrans.w * tex.textrans.y);


	gl_Position = unis.mvp * vec4(inPosition, 1.0, 1.0);
	fragColor = inColor;
	fragTexCoord = nej;
}