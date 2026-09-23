#version 330 core

in vec2 v_TexCoord;
in float v_Brightness;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    FragColor = vec4(texColor.rgb * v_Brightness, texColor.a);
}
