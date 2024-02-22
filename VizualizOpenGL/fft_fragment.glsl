#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

layout(std140) uniform MyBlock{
    float myArray[512];
};

uniform vec2 xy;

void main()
{
    float brightness = myArray[int(gl_FragCoord.x - xy[0])]; 

    vec3 color = vec3(brightness, brightness, brightness);

    FragColor = vec4(color, 1.0);
}
