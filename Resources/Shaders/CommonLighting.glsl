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





struct SurfaceData
{
	// The Surface Position in World Space.
	vec3 P;
	
	// The Surface Normal in World Space.
	vec3 N;
	
	// The direction from the surface to the view.
	vec3 V;

	//
	vec3 Albedo;
};






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


// Geometry Function - GGX [Schlick model] / Smith
float ComputeGeomGGX(float roughness, float NDotV, float NDotL)
{
	float k = (roughness + 1.0);
	k = (k * k) * 0.125;
	
	float G1 = NDotV / (NDotV * (1.0 - k) + k);
	float G2 = NDotL / (NDotL * (1.0 - k) + k);
	
	return G1 * G2;
}


vec3 SpecularIBL(float Roughness, vec3 N, vec3 V, in samplerCube Radiance)
{
	vec3 SpecularLighting = vec3(0);
	
	const uint NumSamples = 32;

	for( uint i = 0; i < NumSamples; i++ )
	{
		vec2 Xi = Hammersley( i, NumSamples );

		vec3 H = ImportanceSampleGGX( Xi, Roughness, N );
		vec3 L = 2 * dot( V, H ) * H - V;
		float NoV = max( dot( N, V ), 0.001 );
		float NoL = max( dot( N, L ), 0.001 );
		float NoH = max( dot( N, H ), 0.001 );
		float VoH = max( dot( V, H ), 0.001 );

		if( NoL > 0.0 )
		{
			vec3 SampleColor = textureLod( Radiance, L, 0 ).rgb;
			float G = ComputeGeomGGX( Roughness, NoV, NoL );
			float Fc = pow( 1 - VoH, 5 );
			vec3 F = (1 - Fc) * SampleColor + Fc;

			SpecularLighting += F * G * VoH / (NoH * NoV);
		}
	}
	
	return SpecularLighting / NumSamples;
}






float ComputeDirShadow(vec3 p, in sampler2D SunShadow, in mat4 SunTransform)
{
	vec4 lp = SunTransform * vec4(p, 1.0);
	lp.xy = lp.xy * 0.5 + 0.5;
	
	float bias = 0.01;
	float shadow = 0.0;
	
	// PCF...
	vec2 texelSize = 1.0 / textureSize(SunShadow, 0);
	

	for(float x = -1.5; x < 1.49; x+=1.0)
	{
		for(float y = -1.5; y < 1.49; y+=1.0)
		{
				float s_depth = texture(SunShadow, lp.xy + vec2(x * texelSize.x, y * texelSize.y)).r;
				shadow += s_depth > (lp.z - bias) ? 0.0 : 1.0;
		}    
	}
	
	shadow /= 9.0;

	return shadow;
}



float ComputeRadianceOcclusion(vec3 v, float ld, in samplerCube Radiance) 
{ 
	float bias = 0.01; 
	
	float offset = 2.5;
	float numSamples = 3.0;
	float M1 = 0.0;
	float M2 = 0.0;
	
	for(float x = -offset; x < offset; x += offset / (numSamples * 0.5)) 
	{ 
		for(float y = -offset; y < offset; y += offset / (numSamples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (numSamples * 0.5)) 
			{
				float s_depth = texture(Radiance, v + vec3(x, y, z)).a; 

				M1 += s_depth;
				M2 += s_depth * s_depth;

				//Occlusion += s_depth > (ld - bias) ? 0.0 : 1.0;
			} 
		} 
	}

	//Occlusion /= 64.0;
	M1 /= 27.0;
	M2 /= 27.0;


	float Ver = M2 - M1 * M1;
	ld -= bias;
	float g = ld - M1;

	return  g > 0.0 ? (Ver / (Ver + g * g)) : 1.0;
}




//
vec3 ComputeSunLight(in SurfaceData Surface, in sampler2D SunShadow, in mat4 SunTransform)
{
	float shadow = 1.0 - ComputeDirShadow(Surface.P, SunShadow, SunTransform);
	vec3 l = -inCommon.SunDir.xyz;

	float df = max(dot(l, Surface.N), 0.0);

	return Surface.Albedo * df * shadow * inCommon.SunColorAndPower.rgb * inCommon.SunColorAndPower.a;
}




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



// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// Image Based Lighting from light probes.


vec4 ComputeIBLight(in SurfaceData Surface, in vec3 Pos, in float Radius, in samplerCube Irradiance,
	in samplerCube Radiance)
{
	vec3 V = Surface.P - Pos;
	float Dist = length(V);
	float Falloff = 1.0 - smoothstep(Radius * Radius * 0.25, Radius * Radius, Dist * Dist);

	vec3 Sample = LightProbeSampleRay(Pos, Radius, Surface.P, Surface.N);
	vec4 DiffuseIrradiance = texture(Irradiance, Sample);
	vec3 Kd = DiffuseIrradiance.rgb * Surface.Albedo;

	float Occlusion = ComputeRadianceOcclusion(V, Dist * 0.001, Radiance);
	Falloff *= Occlusion;


	if (dot(V, Surface.N) > 0.0)
		return vec4(0.0);

	if (inCommon.Mode == COMMON_MODE_REF_CAPTURE)
		return vec4(Kd * 0.5, Falloff * Falloff);

	return vec4(Kd, Falloff * Falloff);

//	vec3 ViewVector = normalize(inCommon.ViewPos - Surface.P);
//	vec3 R = -ViewVector;
//	R = reflect(R, Surface.N);
//	R = LightProbeSampleRay(Pos, Radius, Surface.P, R);
//	vec3 Ks = texture(Radiance, R).rgb;

	//vec3 Ks = SpecularIBL(0.0, Surface.N, ViewVector, Radiance);


	return vec4(Kd, Falloff);
}





// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// Irradiance Volume

struct IrradianceVolumeData
{
	vec3 Start;
	vec3 Extent;
	ivec3 Count;
	vec3 GridSize;
	float GridLen;
};




ivec3 GetGridCoord(in vec3 Pos, in IrradianceVolumeData IrVolume)
{
	return ivec3((Pos - IrVolume.Start + IrVolume.GridSize) / IrVolume.GridSize) - 1;
}


