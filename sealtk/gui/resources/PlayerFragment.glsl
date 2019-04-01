/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#version 130

uniform sampler2DArray image;

in vec2 v_textureCoords;

out vec4 color;

void main()
{
  color = texture(image, vec3(v_textureCoords, 0));
}
