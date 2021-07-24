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

#include <tuple>



class RenderShader;
class RenderScene;
class RenderUniform;


class VKIDevice;
class VKISwapChain;
class VKIImage;
class VKIFramebuffer;
class VKIRenderPass;
class VKIImageView;
class VKISampler;
class VKICommandBuffer;







// Uniforms used by the renderer pipeline.
struct RendererPipelineUniform
{
	// Common Uniform.
	UniquePtr<RenderUniform> common;

};





// Render Target in the renderer pipeline.
struct PipelineRenderTarget
{
	// The Image.
	UniquePtr<VKIImage> image;

	// The View.
	UniquePtr<VKIImageView> view;

	// The Sampler.
	UniquePtr<VKISampler> sampler;

	// Reset Pointers.
	void Reset();

	// Destroy Vulkan Objects.
	void Destroy();
};







// RendererPipeline:
//    - Handle the entire render pipeline.
//    - only one instance is created by the Renderer.
//
class RendererPipeline
{
	// Friend...
	friend class Renderer;

	// Private Construct.
	RendererPipeline();

public:
	// Destruct.
	~RendererPipeline();

	// Initialize The Pipeline.
	void Initialize();

	// Destroy The Pipeline.
	void Destroy();

	// Resize the pipeline targets.
	void Resize(const glm::ivec2& size);

	// Return G-Buffer Render Pass.
	inline VKIRenderPass* GetGBufferPass() const { return mGBufferPass.get(); }

	// Return The Shadow Passe.
	inline VKIRenderPass* GetDirShadowPass() const { return mDirShadowPass.get(); }
	inline VKIRenderPass* GetOmniShadowPass() const { return mOmniShadowPass.get(); }

	// Return the size of the 
	inline glm::ivec2 GetSize() const { return mSize; };

	// Begin Rendering.
	void BeginRender(uint32_t frame, RenderScene* rscene, const glm::vec4& viewport);

	// End Rendering.
	void EndRender();

	// Render The Scene through the entire pipeline.
	void Render(VKICommandBuffer* cmdBuffer);

	// Perfrom a swapchain render step where we copy the final render to the swapchain image.
	void FinalToSwapchain(VKICommandBuffer* cmdBuffer, uint32_t imgIndex);

	// Retrun the uniforms.
	inline const RendererPipelineUniform& GetUniforms() const { return mUniforms; }

private:
	// Setup/Create the renderer pipeline uniforms.
	void SetupUniforms();

	// Setup HDR & LDR targets used for rendering.
	void SetupTargets();

	// Setup GBuffer Targets & Render Pass.
	void SetupGBufferPass();

	//  Setup the lighting pass.
	void SetupLightingPass();

	//  Setup Post Processing.
	void SetupPostProcessPass();

	// Setup BlitSwapchain.
	void SetupBlitSwapchain();

	// Setup the shadow passes.
	void SetupShadowPasses();

	// Rende Scene Shadows.
	void RenderShadows(VKICommandBuffer* cmdBuffer);

	//
	void RenderLightProbes();

private:
	// The vulkan device.
	VKIDevice* mDevice;

	// The vulkan swapchain.
	VKISwapChain* mSwapchain;

	// The Pipeline targets size.
	glm::vec2 mSize;

	// The Pipeline viewport.
	glm::vec4 mViewport;
	glm::vec4 mIntViewport;

	// Flag to check if we are currently rendering.
	bool mIsRendering;

	// The index of the concurrent frame we are currently rendering.
	uint32_t mFrame;

	// The uniforms used & update by the renderer pipeline.
	RendererPipelineUniform mUniforms;

	// The scene we are currently rendering.
	RenderScene* mScene;

	// GBuffer Targets.
	PipelineRenderTarget mAlbedoTarget;
	PipelineRenderTarget mBRDFTarget;
	PipelineRenderTarget mNormalsTarget;
	PipelineRenderTarget mDepthTarget;

	// HDR Target.
	PipelineRenderTarget mHDRTarget[2];

	// LDR Target.
	PipelineRenderTarget mLDRTarget[2];

	// GBuffer Render Pass.
	UniquePtr<VKIRenderPass> mGBufferPass;

	// GBuffer Framebuffer.
	UniquePtr<VKIFramebuffer> mGBufferFB;

	// Lighting Stage Pass.
	UniquePtr<VKIRenderPass> mLightingPass;
	UniquePtr<VKIFramebuffer> mLightingFB;
	UniquePtr<RenderShader> mLightingShader;

	// Post-Procecssing.
	UniquePtr<VKIRenderPass> mPostProPass;
	UniquePtr<VKIFramebuffer> mPostProFB;
	UniquePtr<RenderShader> mPostProShader;

	// Blit final render to swapchain.
	UniquePtr<RenderShader> mBlitSwapchain;

	// Shadow.
	UniquePtr<VKIRenderPass> mDirShadowPass;
	UniquePtr<VKIRenderPass> mOmniShadowPass;


};
