
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform float colorRatio;

void main()
{
    FragColor = mix(texture(ourTexture, TexCoord), vec4(ourColor, 1.0), colorRatio);
}
