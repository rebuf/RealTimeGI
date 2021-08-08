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



// ....
layout(binding=3) uniform MaterailBlock
{
	// The Base Color.
	vec4 Color;

	// The Emission Color.
	vec4 Emission;

	// x[Roughness], y[Metallic].
	vec4 BRDF;

} inMaterial;


layout(binding=4) uniform sampler2D ColorTexture;
layout(binding=5) uniform sampler2D MetallicRoughnessTexture; // Metallic (B), Roughness(G)




// Output...
#if defined(PIPELINE_STAGE_DIR_SHADOW) || defined(PIPELINE_STAGE_OMNI_SHADOW)
// Nothing for Shadow...
#else
layout(location = 0) out vec4 FragAlbedo;
layout(location = 1) out vec4 FragBRDF;
layout(location = 2) out vec4 FragNormal;
layout(location = 3) out vec4 FragEmission;
#endif




void main()
{
#if defined(PIPELINE_STAGE_DIR_SHADOW)
// Nothing for Shadow...
#elif defined(PIPELINE_STAGE_OMNI_SHADOW)
	float LDist = length(inShadow.LightPos.xyz - inFrag.Position);
	gl_FragDepth = LDist;
#else
	FragAlbedo = texture(ColorTexture, inFrag.TexCoord) * inMaterial.Color;
	FragBRDF.rg = texture(MetallicRoughnessTexture, inFrag.TexCoord).gb * inMaterial.BRDF.xy;
	FragNormal = vec4(inFrag.Normal, 1.0);
	FragEmission = vec4(inMaterial.Emission);
#endif
}

