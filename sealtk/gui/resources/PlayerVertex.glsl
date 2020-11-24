/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifdef GL_ES
#undef highp
precision highp float;
#endif

uniform mat4 transform;

attribute vec2 a_vertexCoords;
attribute vec2 a_textureCoords;

varying vec2 v_textureCoords;

void main()
{
  gl_Position = transform * vec4(a_vertexCoords, 0.0, 1.0);
  v_textureCoords = a_textureCoords;
}
