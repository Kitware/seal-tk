#version 130

in vec2 a_vertexCoords;
in vec2 a_textureCoords;

out vec2 v_textureCoords;

void main()
{
  gl_Position = vec4(a_vertexCoords, 0.0, 1.0);
  v_textureCoords = a_textureCoords;
}
