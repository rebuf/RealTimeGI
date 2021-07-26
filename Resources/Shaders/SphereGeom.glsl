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
layout (triangle_strip, max_vertices=18) out;



// VERTEX OUTPUT...
layout(location = 0) in VERTEX_OUT
{
	vec3 Position;
} inGeom[];




// GEOMETRY OUTPUT...
layout(location = 0) out GEOM_OUT
{
	vec3 Position;
} outGeom;



// 
layout(binding = 1) uniform SphereTransfrom
{
  mat4 Proj;
  mat4 View[6];
} inSphere;



void main()
{
  for (int f = 0; f < 6; ++f)
  {
    gl_Layer = f;

    for (int i = 0; i < 3; ++i)
    {
      outGeom.Position = inGeom[i].Position;
      gl_Position = inSphere.Proj * inSphere.View[f] * gl_in[i].gl_Position;
      EmitVertex();
    }

    
    EndPrimitive();
  }

}

