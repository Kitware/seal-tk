/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifdef GL_ES
#undef highp
precision highp float;
#endif

uniform vec4 color;

void main()
{
  gl_FragColor = color;
}
