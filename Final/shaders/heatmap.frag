#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D priceData; 

uniform float maxPrice; 

void main()
{
    float price = texture(priceData, TexCoords).r;

    float normalizedValue = clamp(price / maxPrice, 0.0, 1.0);

    vec3 color;
    if (normalizedValue < 0.5) {
        color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), normalizedValue * 2.0);
    } else {
        color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (normalizedValue - 0.5) * 2.0);
    }

    FragColor = vec4(color, 1.0);
}