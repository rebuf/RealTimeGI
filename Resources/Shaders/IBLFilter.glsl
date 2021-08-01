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


precision highp float;



#include "Common.glsl"



// GEOMETRY OUTPUT...
layout(location = 0) in GEOM_OUT
{
	vec3 Position;
} inFrag;


#if defined(PIPELINE_IBL_SPECULAR)

layout( push_constant ) uniform Constant
{
	float Roughness;

} inSpecularFilter;

#endif



// Input...
layout(binding = 2) uniform samplerCube Environment;


// Output...
layout(location = 0) out vec4 FragColor;




#if defined(PIPELINE_IBL_IRRADIANCE)

// Pre-fitler Irradiance:
//    -
//    - http://www.codinglabs.net/article_physically_based_rendering.aspx
//
vec3 ComputeIrradiance(vec3 Normal)
{
	vec3 irradiance = vec3(0.0);
	float NumSamples = 0.0;

	// Compute Basis 
	vec3 up = abs(Normal.z) > 0.999 ? vec3(0.0, sign(-Normal.z), 0.0) : vec3(0.0, 0.0, 1.0);
	vec3 right = normalize( cross(up, Normal) );
	up = normalize( cross(Normal, right) );
	
	// Delta
	float phidlt = 0.05;
	float thetadlt = 0.015;
	
	// Compute Irradiance from by sampling Environment Texture Hemisphere...
	for (float phi = 0; phi < TWO_PI; phi += phidlt)
	{
		for (float theta = 0; theta < HALF_PI; theta += thetadlt)
		{
			float sinTheta = sin(theta);
			float cosTheta = cos(theta);
			
			vec3 base0 = cos(phi) * right + sin(phi) * up;
			vec3 sv = cosTheta * Normal + sinTheta * base0; // Sample Vector in the hemisphere

			irradiance += texture(Environment, sv).rgb * cosTheta * sinTheta;
			NumSamples += 1.0;
		}
	}
	
	return PI * irradiance / NumSamples;




//	// Compute Basis 
//	vec3 UpVector = abs(Normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
//	vec3 TangentX = normalize( cross( UpVector, Normal ) );
//	vec3 TangentY = cross( Normal, TangentX );
//
//	// Delta
//	float ThetaDt = 0.01;
//	float PhiDt = 0.025;
//
//	// Compute Irradiance by sampling Environment Texture Hemisphere...
//	for (float Theta = 0; Theta < TWO_PI; Theta += ThetaDt)
//	{
//		for (float Phi = 0; Phi < HALF_PI; Phi += PhiDt)
//		{
//			float CosTheta = cos(Theta);
//			float SinTheta = sin(Theta);
//			float CosPhi = cos(Phi);
//			float SinPhi = sin(Phi);
//			
//			vec3 Base = CosPhi * (CosTheta * TangentX + SinTheta * TangentY);
//			vec3 Sample = SinPhi * Normal + Base;
//
//			irradiance += texture(Environment, Sample).rgb * CosPhi * SinPhi;
//			NumSamples += 1.0;
//		}
//	}
//	
//	
//	return PI * irradiance / NumSamples;
}

#endif


#if defined(PIPELINE_IBL_SPECULAR)

//	Hammersley Points on the Hemisphere:
//	   - http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
 
 
 
 // Points on the Hemisphere
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}




// Importance Sampling - skew the samples in the direction of the normal based on roughness.
//		- https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//
vec3 ImportanceSampleGGX(vec2 xi, float roughness, vec3 normal)
{
	float alpha = roughness * roughness;
	

	float phi = TWO_PI * xi.x;
	float cosTheta = sqrt( (1.0 - xi.y) / (1.0 + (alpha * alpha - 1.0) * xi.y)  );
	float sinTheta = sqrt( 1.0 - cosTheta * cosTheta );
	

	vec3 H;
	H.x = sinTheta * cos(phi);
	H.y = sinTheta * sin(phi);
	H.z = cosTheta;
	

	vec3 up = abs(normal.y) > 0.001 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
	vec3 Tx = normalize( cross(up, normal) );
	vec3 Ty = normalize( cross(normal, Tx) );
	

	return Tx * H.x + Ty * H.y + H.z * normal;
}



// Pefilter Specular:
//  - https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//
vec3 FilterSpecularIBL(float roughness, vec3 normal)
{
	vec3 filterColor = vec3(0.0);
	
	const uint numSamples = 2000;
	float weights = 0.0;
	
	for (uint i = 0; i < numSamples; ++i)
	{
		vec2 xi = Hammersley(i, numSamples);
		vec3 H = ImportanceSampleGGX(xi, roughness, normal);
		vec3 L = 2 * dot(normal, H) * H - normal;
		float NDotL = max(dot(normal, L), 0.0);
		
		if (NDotL > 0.0)
		{
			filterColor += texture(Environment, H).rgb * NDotL;
			weights += NDotL;
		}
	}
	
	return filterColor / weights;
}


#endif



void main()
{
	vec3 Normal = normalize(inFrag.Position);

#if defined(PIPELINE_IBL_IRRADIANCE)
	FragColor.rgb = ComputeIrradiance(Normal);
#endif

#if defined(PIPELINE_IBL_SPECULAR)
	FragColor.rgb = FilterSpecularIBL(inSpecularFilter.Roughness, Normal);
#endif

	FragColor.a = 1.0;
}
