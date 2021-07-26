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
#include "glm/vec3.hpp"





class VKIImage;
class VKISampler;
class VKIImageView;
class VKIFramebuffer;









// RenderLight:
//    - 
//
class RenderLight
{
public:

};







// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 




// RenderLightProbe:
//     - 
//
class RenderLightProbe
{
public:
	// Construct.
	RenderLightProbe();

	// Destruct.
	~RenderLightProbe();

	// Create the light probe render data.
	void Create();

	// Destroy the light probe render data.
	void Destroy();

	// Flag this light probe dirty to be updated.
	inline void SetDirty(bool val) { mIsDirty = val; }

	// Return true if this light probe is dirty.
	inline bool IsDirty() const { return mIsDirty; }

	// Set/Get Position.
	inline void SetPosition(const glm::vec3& pos) { mPosition = pos; }
	inline const glm::vec3& GetPosition() const { return mPosition; }

	// Set/Get Radius.
	inline void SetRadius(float value) { mRadius = value; }
	inline float GetRadius() const { return mRadius; }

	// Return irradiance map frame buffer.
	inline VKIFramebuffer* GetIrradianceFB() const { return IrradianceFB.get(); }

private:
	// Flag used to check if its dirty and need updating.
	bool mIsDirty;

	// Radiance Image.
	UniquePtr<VKIImage> mRadiance;

	// Irradiance Image.
	UniquePtr<VKIImage> mIrradiance;

	// Sampler for Radiance[0] & Irradiance[1].
	UniquePtr<VKISampler> mSampler[2];

	// Image View for Radiance[0] & Irradiance[1].
	UniquePtr<VKIImageView> mView[2];

	// Framebuffer for irradiance target.
	UniquePtr<VKIFramebuffer> IrradianceFB;

	// The Position of the light probe.
	glm::vec3 mPosition;

	// The Radius of the light probe.
	float mRadius;
};


