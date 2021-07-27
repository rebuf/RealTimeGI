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


//
vec3 ComputeSunLight(in SurfaceData Surface, in sampler2D SunShadow, in mat4 SunTransform)
{
	float shadow = 1.0 - ComputeDirShadow(Surface.P, SunShadow, SunTransform);
	vec3 l = -inCommon.SunDir.xyz;

	float df = max(dot(l, Surface.N), 0.0);

	return Surface.Albedo * df * shadow;
}




// Image Based Lighting
vec4 ComputeIBLight(in SurfaceData Surface, in vec3 Pos, in float Radius, in samplerCube Irradiance)
{
	vec3 dir = Surface.P - Pos;
	float dist = length(Surface.P - Pos);
	float falloff = 1.0 - smoothstep(Radius * 0.45, Radius, dist);

//	dir /= dist;
//	float offset = dist / Radius;
//	vec3 SampleNormal = normalize(Surface.N + dir * offset);

	vec3 DiffuseIrradiance = texture(Irradiance, Surface.N).rgb * Surface.Albedo;

	return vec4(DiffuseIrradiance, falloff);
}


