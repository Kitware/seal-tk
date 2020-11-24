/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#ifdef GL_ES
#undef highp
precision highp float;
#endif

uniform sampler2D image;
uniform float levelShift;
uniform float levelScale;

varying vec2 v_textureCoords;

void main()
{
  vec4 color = texture2D(image, v_textureCoords);
  gl_FragColor = clamp((color - levelShift) * levelScale, 0.0, 1.0);
}
