#version 330 core

layout(location = 0) in vec3 a_Position;    // position
layout(location = 1) in vec2 a_TexCoord;    // uv
layout(location = 2) in float a_Brightness; // brightness

uniform mat4 u_ViewProj;
uniform mat4 u_Model;

out vec2 v_TexCoord;
out float v_Brightness;

void main()
{
    gl_Position = u_ViewProj * u_Model * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
    v_Brightness = a_Brightness;
}
