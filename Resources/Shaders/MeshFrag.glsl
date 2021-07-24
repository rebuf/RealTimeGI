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



// Input...
layout(location = 0) in VERTEX_OUT
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;

} inFrag;




// Input for Shadow Pass.
#if defined(PIPELINE_STAGE_DIR_SHADOW) || defined(PIPELINE_STAGE_OMNI_SHADOW)
layout( push_constant ) uniform Constant
{
	// Shadow Transform.
	mat4 ShadowMatrix;

	// Light Position, for Omni-Shadows.
	vec4 LightPos;

} inShadow;
#endif





// Output...
#if defined(PIPELINE_STAGE_SHADOW)
// Nothing for Shadow...
#else
layout(location = 0) out vec4 FragAlbedo;
layout(location = 1) out vec4 FragBRDF;
layout(location = 2) out vec4 FragNormal;
#endif




void main()
{
#if defined(PIPELINE_STAGE_DIR_SHADOW)
// Nothing for Shadow...
#elif defined(PIPELINE_STAGE_OMNI_SHADOW)
	float LDist = length(inShadow.LightPos.xyz - inFrag.Position);
	gl_FragDepth = LDist;
#else
	FragAlbedo = vec4(1.0);
	FragBRDF = vec4(1.0);
	FragNormal = vec4(inFrag.Normal, 1.0);
#endif
}

