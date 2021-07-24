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




#include "Frustum.h"




Frustum::Frustum()
{
	memset(mPlanes, 0, sizeof(glm::vec4) * 6);
}


Frustum Frustum::FromVPMatrix(const glm::mat4& mtx)
{
	Frustum f;

	// Left clipping plane   
	f.mPlanes[0].x = mtx[0][3] + mtx[0][0];
	f.mPlanes[0].y = mtx[1][3] + mtx[1][0];
	f.mPlanes[0].z = mtx[2][3] + mtx[2][0];
	f.mPlanes[0].w = mtx[3][3] + mtx[3][0];

	// Right clipping plane   
	f.mPlanes[1].x = mtx[0][3] - mtx[0][0];
	f.mPlanes[1].y = mtx[1][3] - mtx[1][0];
	f.mPlanes[1].z = mtx[2][3] - mtx[2][0];
	f.mPlanes[1].w = mtx[3][3] - mtx[3][0];
	
	// Top clipping plane   
	f.mPlanes[2].x = mtx[0][3] - mtx[0][1];
	f.mPlanes[2].y = mtx[1][3] - mtx[1][1];
	f.mPlanes[2].z = mtx[2][3] - mtx[2][1];
	f.mPlanes[2].w = mtx[3][3] - mtx[3][1];
	
	// Bottom clipping plane   
	f.mPlanes[3].x = mtx[0][3] + mtx[0][1];
	f.mPlanes[3].y = mtx[1][3] + mtx[1][1];
	f.mPlanes[3].z = mtx[2][3] + mtx[2][1];
	f.mPlanes[3].w = mtx[3][3] + mtx[3][1];
	
	// Near clipping plane    
	f.mPlanes[4].x = mtx[0][3] - mtx[0][2];
	f.mPlanes[4].y = mtx[1][3] - mtx[1][2];
	f.mPlanes[4].z = mtx[2][3] - mtx[2][2];
	f.mPlanes[4].w = mtx[3][3] - mtx[3][2];
	
	// Far clipping plane    
	f.mPlanes[5].x = mtx[0][3] + mtx[0][2];
	f.mPlanes[5].y = mtx[1][3] + mtx[1][2];
	f.mPlanes[5].z = mtx[2][3] + mtx[2][2];
	f.mPlanes[5].w = mtx[3][3] + mtx[3][2];
	

	// Normalize Planes.
	f.Normalize(0);
	f.Normalize(1);
	f.Normalize(2);
	f.Normalize(3);
	f.Normalize(4);
	f.Normalize(5);

	return f;
}


void Frustum::Normalize(uint32_t i)
{
	float len = mPlanes[i].x * mPlanes[i].x + mPlanes[i].y * mPlanes[i].y + mPlanes[i].z * mPlanes[i].z;

	// Zero?
	if (len < SMALL_NUM)
	{
		mPlanes[i] = glm::vec4(0.0f);
		return;
	}

	len = sqrt(len);
	mPlanes[i].x /= len;
	mPlanes[i].y /= len;
	mPlanes[i].z /= len;
	mPlanes[i].w /= len;
}


bool Frustum::IsInFrustum2D(const glm::vec3& center, float radius)
{
	// Left 
	if (TestPlane(0, center, radius))
		return false;

	// Right
	if (TestPlane(1, center, radius))
		return false;

	// Near
	if (TestPlane(4, center, radius))
		return false;

	// Far
	if (TestPlane(5, center, radius))
		return false;

	return true;
}


bool Frustum::TestPlane(uint32_t idx, const glm::vec3& center, float radius)
{
	glm::vec3 n = glm::vec3(mPlanes[idx].x, mPlanes[idx].y, mPlanes[idx].z);

	float dr = glm::dot(n, center) + mPlanes[idx].w;

	if (-(dr + radius) > SMALL_NUM)
		return true;

	return false;
}
