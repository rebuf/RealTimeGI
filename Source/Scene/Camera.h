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
#include "Core/Transform.h"



class Camera
{
public:
	// Construct.
	Camera()
		: mNear(1.0f)
		, mFar(320000.0f)
		, mViewPos(0.0f)
		, mViewTarget(Transform::FORWARD)
		, mUp(Transform::UP)
		, mIsPerspective(true)
		, mDistance(1.0f)
		, mFOV(PI * 0.25f)
	{
		mFrustum[0] = 0.0f;
		mFrustum[1] = 0.0f;
		mFrustum[2] = 0.0f;
		mFrustum[3] = 0.0f;
	}

	// Set The View position.
	inline void SetViewPos(const glm::vec3& pos)
	{
		mViewPos = pos;
	}

	// Set The View Target.
	inline void SetViewTarget(const glm::vec3& pos)
	{
		mViewTarget = pos;
	}

	inline void SetUp(const glm::vec3& up)
	{
		mUp = up;
	}

	// Set the distance between view position & target.
	inline void SetViewDistance(float distance)
	{
		mDistance = distance > SMALL_NUM ? distance : SMALL_NUM;
	}

	// Change the view target to be in a direction.
	inline void SetViewDir(const glm::vec3& dir, float distance)
	{
		mViewTarget = mViewPos + dir * mDistance;
		RecomputeUp();
	}

	// Recompute up to be orthographic with the new view pos & target.
	inline void RecomputeUp()
	{
		const glm::vec3 dir = glm::normalize(mViewTarget - mViewPos);
		const glm::vec3 right = glm::normalize( glm::cross(Transform::UP, dir) );
		mUp = glm::cross(dir, right);
	}

	// Enable/Disable Perspective projection.
	inline void SetPerspective(bool value)
	{
		mIsPerspective = value;
	}

	// Set Z-Clip plane for projection.
	inline void SetZPlane(float near, float far)
	{
		mNear = near;
		mFar = far;
	}

	// Set field of view.
	inline void SetFOV(float fov)
	{
		mFOV = fov;
	}

	// Set Aspect Ratio.
	inline void SetAspect(float aspect)
	{
		mAspect = aspect;
	}

	// Set Frustum for projection.
	inline void SetFrustum(float left, float right, float bottom, float top)
	{
		mFrustum[0] = left;
		mFrustum[1] = right;
		mFrustum[2] = bottom;
		mFrustum[3] = top;
	}

	// Return true if the camera projection is perspective.
	inline bool IsPerspective() const { return mIsPerspective; }

	// Return camera view transformation.
	inline glm::mat4 GetViewTransform() const
	{
		return Transform::LookAt(mViewPos, mViewTarget, mUp);
	}

	// Return camera projection transform.
	inline glm::mat4 GetProjection() const
	{
		if (mIsPerspective)
		{
			return Transform::Perspective(mFOV, mAspect, mNear, mFar);
		}
		else
		{
			return Transform::Ortho(mFrustum[0], mFrustum[1], mFrustum[2], mFrustum[3], mNear, mFar);
		}
	}

	// Return Z-Clip plane used in projection.
	inline void GetZPlane(float& outNear, float& outFar)
	{
		outNear = mNear;
		outFar = mFar;
	}

private:
	// The View Position.
	glm::vec3 mViewPos;

	// The View Target Position.
	glm::vec3 mViewTarget;

	// The View Up.
	glm::vec3 mUp;

	// The distance between view position & target.
	float mDistance;

	// Near clip plane.
	float mNear;

	// Far clip plane.
	float mFar;

	// Scene projection field of view.
	float mFOV;

	// Aspect Ratio.
	float mAspect;

	// Othographic Frustum.
	float mFrustum[4];

	// If true then the projection is perspective otherwise orthographic.
	bool mIsPerspective;

};