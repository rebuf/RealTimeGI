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
#include "Render/RenderData/RenderTypes.h"
#include "glm/vec4.hpp"

#include <string>



class Renderer;
class VKIRenderPass;
class VKIGraphicsPipeline;
class VKIDescriptorLayout;
class VKICommandBuffer;
class VKIDescriptorSet;










// RenderShader:
//     - 
//
class RenderShader
{
public:
	// Construct.
	RenderShader();

	// Destruct.
	~RenderShader();

	// Create new RenderShader.
	void Create();

	// Destroy RenderShader created objects/data.
	void Destroy();

	// Set render pass this shader is part of.
	void SetRenderPass(VKIRenderPass* pass);

	// The Shader Domain.
	void SetDomain(ERenderShaderDomain domain);

	// Enable/Dsiable wire frame polygon mode.
	void SetWireframe(bool value);

	// Set the shader source.
	void SetShader(ERenderShaderStage stage, const std::string& srcPath);

	// Add input description.
	void AddInput(uint32_t binding, ERenderShaderInputType inputType, ERenderShaderStage stages);
	void AddInput(uint32_t binding, ERenderShaderInputType inputType, ERenderShaderStage stages, uint32_t count);

	// Add push constant at index.
	void AddPushConstant(uint32_t index, uint32_t offset, uint32_t size, ERenderShaderStage stage);

	// Clear all added input description.
	void CleareInput();

	// Recreate the shader pipeline to match the new data.
	void Recreate();

	// Bind the shader.
	void Bind(VKICommandBuffer* cmdBuffer) const;

	// Return the descriptor layout.
	inline VKIDescriptorLayout* GetLayout() { return mDescLayout.get(); }

	// Return the pipeline.
	inline const VKIGraphicsPipeline* GetPipeline() const { return mPipeline.get(); }

	// Create the optional descriptor set for this shader and return it.
	VKIDescriptorSet* CreateDescriptorSet();

	// Return this shader's descriptor set. This will return null if not created.
	inline VKIDescriptorSet* GetDescriptorSet() { return mDescriptorSet.get(); }
	inline const VKIDescriptorSet* GetDescriptorSet() const { return mDescriptorSet.get(); }

public:
	// Set the viewport.
	void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height);
	void SetViewport(const glm::ivec4& viewport);

	// Enable/Disable Dynamic Viewport.
	void SetViewportDynamic(bool value);

	// Recreate the shader pipeline with a new viewport size.
	void Resize(int32_t x, int32_t y, uint32_t width, uint32_t height);

	// Enable/Disable depth test and depth write.
	void SetDepth(bool isEnabled, bool isWrite);

	// Enable/Disable blending for a specific attachment index.
	void SetBlendingEnabled(uint32_t index, bool isEnabled);

	// Set blending operation for a specific attachment index.
	void SetBlending(uint32_t index, ERenderBlendFactor src, ERenderBlendFactor dst, ERenderBlendOp op);
	void SetBlending(uint32_t index, ERenderBlendFactor csrc, ERenderBlendFactor cdst, ERenderBlendOp cop,
		ERenderBlendFactor asrc, ERenderBlendFactor adst, ERenderBlendOp aop);

	// Return true if the viewport of this pipeline is a dynamic state.
	inline bool IsViewportDynamic() { return mIsDynamicViewport; }

private:
	// Setup pipelinebased on domain.
	void SetupPipelineDomain();

public:
	// The binding of the common uniform block.
	static const uint32_t COMMON_BLOCK_BINDING;
	static const uint32_t TRANSFORM_BLOCK_BINDING;

public:
	// The graphics pipeline that execute this shader.
	UniquePtr<VKIGraphicsPipeline> mPipeline;

	// The render pass this shader is used in.
	VKIRenderPass* mRenderPass;

	// Descriptor Layout of the shader input.
	UniquePtr<VKIDescriptorLayout> mDescLayout;

	// The domain this shader resides in. it defines many parameters around the shader.
	ERenderShaderDomain mDomain;

	// If true set the viewport to dynamic state.
	bool mIsDynamicViewport;

	// Optional DescriptorSet managed by the shader.
	UniquePtr<VKIDescriptorSet> mDescriptorSet;
};


