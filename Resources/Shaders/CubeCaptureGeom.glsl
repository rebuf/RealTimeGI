// Copyright (c) 2021 Ammar Herzallah
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.



#version 450
#extension GL_ARB_separate_shader_objects : enable



#include "Common.glsl"


layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;


// VERTX OUTPUT...
layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec2 inTargetTexCoord[];


// GEOM OUTPUT...
layout(location = 0) out vec2 TexCoord;
layout(location = 1) out vec2 TargetTexCoord; // The texture coordinate for sampling render targets.


layout( push_constant ) uniform Constant
{
	// Layer
	int CubeLayer;

} inCubeCapture;




void main()
{
  gl_Layer = inCubeCapture.CubeLayer;

  for (int i = 0; i < 3; ++i)
  {
    gl_Position = gl_in[i].gl_Position;
    TexCoord = inTexCoord[i];
    TargetTexCoord = inTargetTexCoord[i];
    EmitVertex();
  }

  EndPrimitive();
}

