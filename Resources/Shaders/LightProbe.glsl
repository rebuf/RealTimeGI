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







//
//
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
//
vec4 ComputeLightProbe(in SurfaceData Surface, in vec3 Pos, in float Radius,
	in samplerCube Irradiance, in samplerCube Radiance)
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

	if ((inCommon.Mode & COMMON_MODE_REF_CAPTURE) != 0)
		return vec4(Kd * 0.5, Falloff * Falloff);

	return vec4(Kd, Falloff * Falloff);
}


