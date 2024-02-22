#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

layout(std140) uniform MyBlock{
    float myArray[4];
};

void main()
{
    // Assuming myArray contains values between 0.0 and 1.0
    float brightness = myArray[int(gl_FragCoord.x - x)]; // Convert x coordinate to int

    // Set the output color based on brightness
    vec3 color = vec3(brightness, brightness, brightness);

    // Output the final color
    FragColor = vec4(color, 1.0);
}
