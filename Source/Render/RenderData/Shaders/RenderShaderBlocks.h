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



#pragma once




#include "Core/Core.h"
#include "RenderUniform.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/matrix.hpp"






#define COMMON_MODE_NONE 0
#define COMMON_MODE_REF_CAPTURE 1








namespace GUniform
{

	// General uniform data that can be used by all shaders
	struct CommonBlock
	{
		// The View & Projection Matrix of the scene.
		glm::mat4 viewProjMatrix;

		// The Inverse of View & Projection Matrix of the scene.
		glm::mat4 viewProjMatrixInverse;

		// The View Direction of the scene.
		alignas(16) glm::vec3 viewDir;

		// The View Position of the scene.
		alignas(16) glm::vec3 viewPos;

		// The render viewport postion and size, X,Y: Position, and Z,W: Size.
		//     - Values Ranges [0, width], [0, Height]
		alignas(16) glm::vec4 viewport;

		// The size of the render target used for the current pass.
		glm::vec4 targetSize;

		// The environment sun direction.
		glm::vec4 sunDir;

		// The environment sun color(RGB) & power(A).
		glm::vec4 sunColorAndPower;

		// X: Near Plane, Y: Far Plane.
		alignas(8) glm::vec2 nearFar;

		// Application Time.
		float time;

		// Mode used to identify the current rendering stage.
		int32_t mode;
	};



	// Data used for shadow pass.
	struct ShadowConstantBlock
	{
		// Shadow Transform.
		glm::mat4 shadowMatrix;

		// Light Position, for Omni-Shadows.
		glm::vec4 lightPos;
	};


	// Data used for shadow pass.
	struct SphereHelperBlock
	{
		glm::vec4 position;
		glm::vec4 scale;
		glm::vec4 color;
	};


}











