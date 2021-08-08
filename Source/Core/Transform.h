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

#include "glm/common.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"



// Transform:
//   - Scale, Rotation & Translation.
//   - Order of transformation Scale -> Rotate -> Translate.
//
class Transform
{
public:
	// Construct Uninitialized.
	Transform();

	// Construct using transform matrix .
	Transform(const glm::mat4& mtx);

	// Copy Construct.
	Transform(const Transform& other);

	// Copy Assignment.
	Transform& operator=(const Transform& other);

	// Return transform matrix.
	void SetMatrix(const glm::mat4& mtx);

	// Set Transform Matrix.
	const glm::mat4& GetMatrix() const;

	// Set Transform...
	inline void SetTranslate(const glm::vec3& translate) { mTranslate = translate; }
	inline void SetScale(const glm::vec3& scale) { mScale = scale; }
	inline void SetRotate(const glm::quat& rotate) { mRotate = rotate; }
	void SetRotate(glm::vec3 euler); // Set Rotation using Euler angles.

	// Get Transform...
	inline const glm::vec3& GetTranslate() const { return mTranslate; }
	inline const glm::vec3& GetScale() const { return mScale; }
	inline const glm::quat& GetRotate() const { return mRotate; }
	glm::vec3 GetRotate(); // Return Rotation as Euler angles.

	// Create a view matrix from look at.
	static glm::mat4 LookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up);

	// Return perspective transform.
	static glm::mat4 Perspective(float fov, float aspect, float near, float far);

	// Return orthographic transform.
	static glm::mat4 Ortho(float left, float right, float bottom, float top, float near, float far);

	// Return cubemap view & projection matrix for specific face index.
	static glm::mat4 GetCubeView(uint32_t face, const glm::vec3& pos);
	static glm::mat4 GetCubeViewProj(uint32_t face, const glm::vec3& pos);

	// Compute polar angles Pitch & Yaw from direction vector.
	static void DirectionToPolar(const glm::vec3& dir, float& pitch, float& yaw);

	// Convert polar angles Pitch & Yaw from direction vector.
	static glm::vec3 PolarToDirection(float pitch, float yaw);

private:
	// Update cached matrix.
	void UpdateMatrix() const;

public:
	// Identity Transform
	static Transform IDENTITY;

	// Identity Transform
	static glm::vec3 FORWARD;
	static glm::vec3 RIGHT;
	static glm::vec3 UP;

private:
	// Transform - Translation.
	glm::vec3 mTranslate;

	// Transform - Scale.
	glm::vec3 mScale;

	// Transform - Rotation.
	glm::quat mRotate;

	// Transform Matrix Cache.
	mutable glm::mat4 mMtx;

	// Flag true if the cache is dirty;
	mutable bool mIsDirty;
};
