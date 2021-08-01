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




layout(binding = 10) uniform samplerCube Irradiance;
layout(binding = 11) uniform samplerCube Radiance;




// Vertex Input...
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec2 TargetTexCoord;



// Output...
layout(location = 0) out vec4 FragColor;





vec2 signNotZero(vec2 v) 
{
	return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}


vec2 octEncode(in vec3 v) 
{
	v.xyz = -v.xzy;

	float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
	vec2 result = v.xy * (1.0 / l1norm);
	if (v.z < 0.0) {
	    result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
	}

	return result;
}


vec3 octDecode(vec2 o) 
{
	vec3 v = vec3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
	if (v.z < 0.0) {
	    v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
	}

	return normalize(-v.xzy);
}



void main()
{
	// --- - --- - ---- ---- --- ----- ----- -
	//             VISUALIZE
	// --- - --- - ---- ---- --- ----- ----- -

	bool isDiscard = true;
	vec2 gsize = inCommon.Viewport.zw;
	float gscale = mix(1.0, 1.5, gsize.x / 1024.0 - 1.0);
	vec4 rect;

	// 
	rect = vec4(650, 50, 500, 250) * gscale;
	if ( gl_FragCoord.x > rect.x && gl_FragCoord.x < (rect.x + rect.z)
		&& gl_FragCoord.y > rect.y && gl_FragCoord.y < (rect.y + rect.w) )
	{
		vec2 ruv = (gl_FragCoord.xy - rect.xy) / (rect.zw);
		FragColor.rgb = VisualizeCubeMap(Radiance, ruv).rgb;
		isDiscard = false;
	}

	// 
	rect = vec4(650, 350, 500, 250) * gscale;
	if ( gl_FragCoord.x > rect.x && gl_FragCoord.x < (rect.x + rect.z)
		&& gl_FragCoord.y > rect.y && gl_FragCoord.y < (rect.y + rect.w) )
	{
		vec2 ruv = (gl_FragCoord.xy - rect.xy) / (rect.zw);
		FragColor.rgb = VisualizeCubeMap(Irradiance, ruv).rgb;
		isDiscard = false;
	}


	if (isDiscard)
	{
		discard;
	}

	FragColor.a = 1.0;
}

