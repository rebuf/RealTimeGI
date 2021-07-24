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




#include "Core.h"
#include "Box.h"

#include "glm/common.hpp"
#include "glm/vec4.hpp"
#include "glm/matrix.hpp"







// Frustum:
//    - 
//
class Frustum
{
public:
	// Default Construct.
	Frustum();

	// Create frustum from View, Projection matrix.
	static Frustum FromVPMatrix(const glm::mat4& mtx);

	// Test if a bounding sphere inside this frustum in 2D.
	bool IsInFrustum2D(const glm::vec3& center, float radius);

private:
	// Normalize the plane.
	void Normalize(uint32_t i);

	// Test if the sphere inside the positive half space of the plane.
	bool TestPlane(uint32_t idx, const glm::vec3& center, float radius);

private:
	// The frustum planes in ax+by+cz+d=0 form.
	glm::vec4 mPlanes[6];
};

