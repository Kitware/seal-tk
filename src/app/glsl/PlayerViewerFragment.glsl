#version 130

uniform sampler2D image;

in vec2 v_textureCoords;

out vec4 color;

void main()
{
  color = texture(image, v_textureCoords);
}
