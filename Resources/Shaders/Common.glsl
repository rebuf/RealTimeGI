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





#define COMMON_MODE_NONE 0
#define COMMON_MODE_REF_CAPTURE 1




// General uniform data that can be used by all shaders
layout(binding = 0) uniform CommonBlock
{
	// The View & Projection Matrix of the scene.
	mat4 ViewProjMatrix;
	
	// The Inverse of View & Projection Matrix of the scene.
	mat4 ViewProjMatrixInverse;
	
	// The View Direction of the scene.
	vec3 ViewDir;
	
	// The View Position of the scene.
	vec3 ViewPos;
	
	// The render viewport postion and size, X,Y: Position, and Z,W: Size.
	//     - Values Ranges [0, width], [0, Height]
	vec4 Viewport;
	
	// The size of the render target used for the current pass.
	vec4 TargetSize;

	// The environment sun direction.
	vec4 SunDir;
	
	// The environment sun color(RGB) & power(A).
	vec4 SunColorAndPower;
	
	// X: Near Plane, Y: Far Plane.
	vec2 NearFar;
	
	// Application Time.
	float Time;

	// Mode used to identify the current rendering stage.
	int mode;
	
} inCommon;




#define NEAR_VALUE inCommon.NearFar.x
#define FAR_VALUE inCommon.NearFar.y


#define PI 3.14159265
#define TWO_PI 6.2831853
#define HALF_PI 1.5707963
#define ONE_OVER_PI 0.3183099
#define SMALL_NUM 0.000001





// Compute World Position form depth.
float DepthToLinaer(float Depth) 
{
	return (-1.0 * NEAR_VALUE * FAR_VALUE) / (Depth * (FAR_VALUE - NEAR_VALUE) - FAR_VALUE);	
}




// Compute World Position form depth texture.
vec3 ComputeWorldPos(float Depth, vec2 TexCoord)
{
	TexCoord = TexCoord * 2.0 - 1.0;
	
	vec4 WorldPos = inCommon.ViewProjMatrixInverse * vec4(TexCoord, Depth, 1.0);
	return WorldPos.xyz / WorldPos.w;
}



// Visualize CubeMap from screen coordiante.
vec3 VisualizeCubeMap(samplerCube Map, vec2 Coord)
{
	Coord.y = 1.0 - Coord.y;

	float Theta  = Coord.x * PI * 2.0;
	float Phi    = (Coord.y - 0.5) * PI;
	float CosPhi = cos(Phi);	

	vec3 n = vec3(CosPhi * cos(Theta), CosPhi * sin(Theta), sin(Phi));
	return texture(Map, n.xyz).rgb;
}
