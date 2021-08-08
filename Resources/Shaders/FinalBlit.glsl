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


#define SCALE_UV_WITH_TARGET 1
#define FXAA_PC 1
#define FXAA_GLSL_130 1
#define FXAA_QUALITY__PRESET 12
#include "FXAA_3_11.glsl"



// Vertex Input...
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec2 TargetTexCoord; // The texture coordinate for sampling render targets.

// Input...
layout(binding = 1) uniform sampler2D FinalRender;


// Output...
layout(location = 0) out vec4 FragColor;





void main()
{
	vec2 fxaaQualityRcpFrame = 1.0 / textureSize(FinalRender, 0);


	// This used to be the FXAA_QUALITY__SUBPIX define.
	// It is here now to allow easier tuning.
	// Choose the amount of sub-pixel aliasing removal.
	// This can effect sharpness.
	//   1.00 - upper limit (softer)
	//   0.75 - default amount of filtering
	//   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
	//   0.25 - almost off
	//   0.00 - completely off
	float fxaaQualitySubpix = 0.75;
	
	
	// This used to be the FXAA_QUALITY__EDGE_THRESHOLD define.
	// It is here now to allow easier tuning.
	// The minimum amount of local contrast required to apply algorithm.
	//   0.333 - too little (faster)
	//   0.250 - low quality
	//   0.166 - default
	//   0.125 - high quality 
	//   0.063 - overkill (slower)
	float fxaaQualityEdgeThreshold = 0.166;
	
	
	// This used to be the FXAA_QUALITY__EDGE_THRESHOLD_MIN define.
	// It is here now to allow easier tuning.
	// Trims the algorithm from processing darks.
	//   0.0833 - upper limit (default, the start of visible unfiltered edges)
	//   0.0625 - high quality (faster)
	//   0.0312 - visible limit (slower)
	// Special notes when using FXAA_GREEN_AS_LUMA,
	//   Likely want to set this to zero.
	//   As colors that are mostly not-green
	//   will appear very dark in the green channel!
	//   Tune by looking at mostly non-green content,
	//   then start at zero and increase until aliasing is a problem.
	float fxaaQualityEdgeThresholdMin = 0.0833;
	
	
	
	// FXAA...
	FragColor = FxaaPixelShader(
		TargetTexCoord,                        // pos {xy} = center of pixel
		vec4(0.0),                             // fxaaConsolePosPos.
		FinalRender,                           // Input color texture.
		FinalRender,                           // fxaaConsole360TexExpBiasNegOne
		FinalRender,                           // fxaaConsole360TexExpBiasNegTwo
		fxaaQualityRcpFrame,
		vec4(0.0),                             // fxaaConsoleRcpFrameOpt
		vec4(0.0),                             // fxaaConsoleRcpFrameOpt2
		vec4(0.0),                             // fxaaConsole360RcpFrameOpt2,
		fxaaQualitySubpix,
		fxaaQualityEdgeThreshold,
		fxaaQualityEdgeThresholdMin,
		0.0,                                   // fxaaConsoleEdgeSharpness,
		0.0,                                   // fxaaConsoleEdgeThreshold,
		0.0,                                   // fxaaConsoleEdgeThresholdMin,
		vec4(0.0)                              // fxaaConsole360ConstDir
	);
	
	
	// Reset Alpha to 1.0.
	FragColor.a = 1.0;
}

