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
#include "glm/vec3.hpp"
#include "glm/common.hpp"





// Box:
//  - Axis aligned bounding box.
//
class Box
{
public:
	// Construct
	Box()
		: mValid(false)
		, mMin( 1.0f)
		, mMax(-1.0f)
	{

	}

	// Construct box using min & max.
	Box(const glm::vec3& min, const glm::vec3& max)
		: mValid(true)
		, mMin(min)
		, mMax(max)
	{

	}

	// Add point/position to be included by the bounding box.
	inline void Add(const glm::vec3& p)
	{
		if (!mValid)
		{
			mMin = mMax = p;
			mValid = true;
		}
		else
		{
			mMin = glm::min(mMin, p);
			mMax = glm::max(mMax, p);
		}
	}

	// Add another box to be included by the bounding box.
	inline void Add(const Box& box)
	{
		if (!box.mValid)
			return;

		Add(box.mMin);
		Add(box.mMax);
	}

	// Return the bounding box center.
	inline glm::vec3 Center() const { return (mMin + mMax) * 0.5f; }

	// Return the bounding box extent.
	inline glm::vec3 Extent() const { return (mMax - mMin) * 0.5f; }

	// Return the box minimum point.
	inline const glm::vec3& GetMin() const { return mMin; }

	// Return the box maximum point.
	inline const glm::vec3& GetMax() const { return mMax; }

	// Reset Box to invalid.
	inline void Reset()
	{
		mValid = false;
	}

	// Return true if the box has valid data.
	inline bool IsValid()
	{
		return mValid;
	}


private:
	// True if the box has valid data.
	bool mValid;

	// Box minimum.
	glm::vec3 mMin;

	// Box maximum.
	glm::vec3 mMax;

};


