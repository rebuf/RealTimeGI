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







#include "Transform.h"


#include "glm/common.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"





Transform Transform::IDENTITY = Transform(glm::mat4(1.0f));
glm::vec3 Transform::FORWARD = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 Transform::RIGHT = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Transform::UP = glm::vec3(0.0f, 0.0f, 1.0f);





Transform::Transform()
	: mIsDirty(true)
{

}


Transform::Transform(const glm::mat4& mtx)
	: mIsDirty(false)
{
	mMtx = mtx;
	SetMatrix(mtx);
}


void Transform::SetMatrix(const glm::mat4& mtx)
{
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(mtx, mScale, mRotate, mTranslate, skew, perspective);
}


const glm::mat4& Transform::GetMatrix() const
{
	if (mIsDirty)
	{
		UpdateMatrix();
	}

	return mMtx;
}


void Transform::UpdateMatrix() const
{
	mIsDirty = false;

	glm::translate(glm::mat4(1.0f), mTranslate);
	mMtx = mMtx * glm::mat4(mRotate);
	mMtx = glm::scale(mMtx, mScale);
}



Transform::Transform(const Transform& other)
	: mTranslate(other.mTranslate)
	, mScale(other.mScale)
	, mRotate(other.mRotate)
	, mMtx(other.mMtx)
	, mIsDirty(other.mIsDirty)
{

}


Transform& Transform::operator=(const Transform& other)
{
	mTranslate = other.mTranslate;
	mScale = other.mScale;
	mRotate = other.mRotate;
	mMtx = other.mMtx;
	mIsDirty = other.mIsDirty;

	return *this;
}


glm::mat4 Transform::LookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up)
{
	return glm::lookAtLH(pos, target, up);
}


glm::mat4 Transform::Perspective(float fov, float aspect, float near, float far)
{
	glm::mat4 mtx = glm::perspectiveLH_ZO(fov, aspect, near, far);
	mtx[1][1] *= -1.0f; // Vulkan NDC is RH.
	return mtx;
}


glm::mat4 Transform::Ortho(float left, float right, float bottom, float top, float near, float far)
{
	return glm::orthoLH(left, right, bottom, top, near, far);
}
