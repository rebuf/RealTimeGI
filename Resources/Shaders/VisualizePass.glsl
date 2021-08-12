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
	rect = vec4(350, 0, 500, 250) * gscale * 1.4;
	if ( gl_FragCoord.x > rect.x && gl_FragCoord.x < (rect.x + rect.z)
		&& gl_FragCoord.y > rect.y && gl_FragCoord.y < (rect.y + rect.w) )
	{
		vec2 ruv = (gl_FragCoord.xy - rect.xy) / (rect.zw);
		FragColor.rgb = VisualizeCubeMap(Radiance, ruv).rgb;
		isDiscard = false;
	}

	// 
	rect = vec4(350, 260, 500, 250) * gscale * 1.4;
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

