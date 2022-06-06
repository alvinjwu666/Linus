#version 400
out vec4 FragColor;

in vec2 TexCoord;


void main()
{
    FragColor = vec4(1, 0, 0, 1);
    // FragColor = texture(ourTexture, TexCoord);
}