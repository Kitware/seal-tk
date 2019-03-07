/* This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
 * 3-Clause License. See top-level LICENSE file or
 * https://github.com/Kitware/seal-tk/blob/master/LICENSE for details. */

#version 150

uniform mat4 homography;
uniform mat4 viewHomography;

layout(lines_adjacency) in;
layout(triangle_strip) out;

vec4 corner(vec4 point1, vec4 point2, vec4 point3, float multiplier)
{
  vec4 diff1 = point1 - point2;
  vec4 diff2 = point3 - point2;
  return point2 + multiplier * normalize(normalize(diff1) + normalize(diff2));
}

void emitVertex(int index, float multiplier)
{
  gl_Position = viewHomography * homography * corner(
    gl_in[index - 1].gl_Position, gl_in[index].gl_Position,
    gl_in[index + 1].gl_Position, multiplier);
  EmitVertex();
}

void main()
{
  /*emitVertex(1, -10);
  emitVertex(1, 10);
  emitVertex(2, -10);
  emitVertex(2, 10);*/
  gl_Position = viewHomography * homography * gl_in[0].gl_Position;
  EmitVertex();
  gl_Position = viewHomography * homography * gl_in[1].gl_Position;
  EmitVertex();
  gl_Position = viewHomography * homography * gl_in[2].gl_Position;
  EmitVertex();
  EndPrimitive();
}
