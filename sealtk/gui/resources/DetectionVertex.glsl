/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifdef GL_ES
#undef highp
precision highp float;
#endif

attribute vec2 a_vertexCoords;

uniform mat4 transform;

void main()
{
  gl_Position = transform * vec4(a_vertexCoords, 0.0, 1.0);
}
