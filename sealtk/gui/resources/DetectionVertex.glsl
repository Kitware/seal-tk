/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#version 130

in vec2 a_vertexCoords;

uniform mat4 homography;
uniform mat4 viewHomography;

void main()
{
  gl_Position = viewHomography * homography * vec4(a_vertexCoords, 0.0, 1.0);
}
