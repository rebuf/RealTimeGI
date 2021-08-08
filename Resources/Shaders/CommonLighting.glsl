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



//  CommonLighting:
//     - Contain common lighting operations used by the lighting passes.
//
//  References Used for PBR & IBL:
//     - https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//     - https://www.trentreed.net/blog/physically-based-shading-and-image-based-lighting/
//





// Data used to light a surface.
struct SurfaceData
{
	// The Surface Position in World Space.
	vec3 P;
	
	// The Surface Normal in World Space.
	vec3 N;
	
	// The direction from the surface to the view.
	vec3 V;
	
	// Base Reflective Index
	vec3 F0;
	
	// The Surface Albedo(Color)
	vec3 Albedo;
	
	// The roughness factor [0, 1] 0:Rough - 1:Smooth
	float Roughness;
	
	// The metallic factor [0, 1] 0:Plastic - 1:Metallic
	float Metallic;
	
	// Specular Factor used to control overall specular reflection.
	float Specular;
	
	// Dot product between surface normal and view vector.
	float NDotV;
	
	// Ambient Occlusion.
	float AO;
};





// Normal Distribution Function - GGX [Trowbridge-Reitz]
float ComputeDistributionGGX(float Roughness, float NDotH)
{
	float Alpha = Roughness * Roughness;
	float D = (NDotH * NDotH) * (Alpha - 1.0) + 1.0;
	return Alpha / (PI * D * D);
}


// Geometry Function - GGX [Schlick model] / Smith
float ComputeGeomGGX(float Roughness, float NDotV, float NDotL)
{
	float k = (Roughness + 1.0);
	k = (k * k) * 0.125;
	
	float G1 = NDotV / (NDotV * (1.0 - k) + k);
	float G2 = NDotL / (NDotL * (1.0 - k) + k);
	
	return G1 * G2;
}


// Fresnel - Schlick approximation + Spherical Gaussian approximation
vec3 ComputeFresnel(vec3 F0, float VDotH)
{
	return F0 + (1.0 - F0) * pow(2, (-5.55473 * VDotH - 6.98316) * VDotH);
}




// Compute Physically Based Rendering BRDF:
// 		- Cook-Torrance microfacet specular shading model.
//
vec3 ComputeBRDFLighting(in SurfaceData Surface, float NDotH, float NDotV, float NDotL, float VDotH)
{
	// BRDF specular funtion DGF.
	float Dggx = ComputeDistributionGGX(Surface.Roughness, NDotH);
	float Gggx = ComputeGeomGGX(Surface.Roughness, NDotV, NDotL);
	vec3 Fr = ComputeFresnel(Surface.F0, VDotH);
	
	// Specular & Diffuse Terms
	vec3 Ks = (Dggx * Gggx * Fr) / (4.0 * NDotL * NDotV);
	Ks *= Surface.Specular;
	
	vec3 Kd = Surface.Albedo * ONE_OVER_PI;
	
	// Incoming Light == Outgoing Light
	Kd *= (1.0 - Ks);
	
	// Non-Dielectric Surface - No Diffuse.
	Kd *= (1.0 - Surface.Metallic);

	// In capture mode we only need diffuse.
	if ((inCommon.Mode & COMMON_MODE_REF_CAPTURE) != 0)
		Ks *= 0.0;
	
	return (Kd + Ks) * NDotL;
}




// Compute Direction Light Shadows.
float ComputeDirShadow(vec3 P, in sampler2D ShadowMap, in mat4 LightTransform)
{
	vec4 LP = LightTransform * vec4(P, 1.0);
	LP.xy = LP.xy * 0.5 + 0.5;
	
	float Bias = 0.01;
	float ShadowValue = 0.0;
	
	// PCF...
	vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
	
	for(float x = -1.5; x < 1.49; x+=1.0)
	{
		for(float y = -1.5; y < 1.49; y+=1.0)
		{
				float S_Depth = texture(ShadowMap, LP.xy + vec2(x * texelSize.x, y * texelSize.y)).r;
				ShadowValue += S_Depth > (LP.z - Bias) ? 0.0 : 1.0;
		}    
	}
	
	ShadowValue /= 9.0;

	return ShadowValue;
}


// Compute a sample ray by perfroming ray sphere intersection.
vec3 LightProbeSampleRay(in vec3 Center, float Radius, in vec3 RayOrg, in vec3 RayDir)
{
	RayDir = -RayDir;

	vec3 V =  RayOrg - Center;
	float A = dot(RayDir, RayDir);
	float B = 2.0 * dot(V, RayDir);
	float C = dot(V, V) - Radius * Radius;
	float Disc = B*B - 4*A*C;
	
	// Fail?
	if (Disc < 0.0)
		return vec3(0.0);
	
	float T = (-B - sqrt(Disc)) / (2.0 * A);
	return normalize((RayOrg + RayDir * T) - Center);
}





// Compuate Sun from Common Data.
vec3 ComputeSunLight(in SurfaceData Surface, in sampler2D SunShadow, in mat4 SunTransform)
{
	// The Light Data
	vec3 L = -inCommon.SunDir.xyz;
	vec3 H = normalize(L + Surface.V);
	float NDotL = dot(Surface.N, L);
	
	NDotL = max(NDotL, 0.0001);
	float NDotH = max(dot(Surface.N, H), 0.0001);
	float VDotH = max(dot(Surface.V, H), 0.00001);
	
	// BRDF
	vec3 BRDFValue = ComputeBRDFLighting(Surface, NDotH, Surface.NDotV, NDotL, VDotH);
	
	// SUN SHADOW.
	float ShadowValue = ComputeDirShadow(Surface.P, SunShadow, SunTransform);
	
	// Return the sun lighting
	return BRDFValue * inCommon.SunColorAndPower.rgb * inCommon.SunColorAndPower.a * (1.0 - ShadowValue);
}


