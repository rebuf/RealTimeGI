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
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include "glm/matrix.hpp"




class VKIImage;
class VKIImageView;
class VKISampler;
class VKIFramebuffer;
class VKICommandBuffer;



// IRenderShadow:
//    - Interface for all shadow types.
//
class IRenderShadow
{
public: 
	// Destructor.
	virtual ~IRenderShadow() { }

	// Return the shadow matrix.
	virtual const glm::mat4& GetShadowMatrix() const = 0;

	// Return the light position, used for omni-shadows.
	virtual const glm::vec3& GetLightPos() const = 0;
};




// RenderShadow:
//   - Directional Light Shadow.
//
class RenderDirShadow : public IRenderShadow
{
public:
	// Construct.
	RenderDirShadow();

	// Destruct.
	~RenderDirShadow();

	// Set the shadow map resolution.
	void SetSize(glm::ivec2 size);

	// Create the shadow map.
	void Create();

	// Destroy the shadow map.
	void Destroy();

	// Return ShadowMap's framebuffer.
	inline VKIFramebuffer* GetFramebuffer() { return mFramebuffer.get(); }

	// Return ShadowMap's sampler.
	inline VKISampler* GetSampler() { return mSampler.get(); }

	// Return ShadowMap's image view.
	inline VKIImageView* GetView() { return mView.get(); }

	// Set the viewport & scissor to match the shadow map render size.
	void ApplyViewport(VKICommandBuffer* cmdBuffer);

	// Return the shadow map viewport.
	inline glm::ivec4 GetViewport() { return glm::ivec4(0, 0, mSize.x, mSize.y); }

	// Return the shadow matrix.
	virtual const glm::mat4& GetShadowMatrix() const override;

	// Return the light position.
	virtual const glm::vec3& GetLightPos() const override;

	// The shadow matrix.
	inline void SetShadowMatrix(const glm::mat4& mtx) { mShadowMatrix = mtx; }

	// Flag/Unflag this shadow map as dirty.
	inline void SetDirty(bool val) { mIsDirty = val; }

	// Return true if this shadow map is drity and need updating.
	inline bool IsDirty() const { return mIsDirty; }

private:
	// The Shadow map contain the depth.
	UniquePtr<VKIImage> mShadowMap;

	// The framebuffer to be used in the shadow pass..
	UniquePtr<VKIFramebuffer> mFramebuffer;

	// The shadow map sampler
	UniquePtr<VKISampler> mSampler;

	// The shadow map view.
	UniquePtr<VKIImageView> mView;

	// The shadow map resolution.
	glm::ivec2 mSize;

	// Shadow Matrix for rendering shadow map.
	glm::mat4 mShadowMatrix;

	// If true the shadow map is dirty and need to be udpated.
	bool mIsDirty;
};
