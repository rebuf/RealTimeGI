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
#include "vulkan/vulkan.h"

#include <vector>
#include <map>
#include <string>
#include <set>




class VKIDevice;
class VKIRenderPass;
class VKIDescriptorLayout;






// Shader source data for the graphics pipeline.
struct VKIGfxPipelineSource
{
	// The stage this shader is part of
	VkShaderStageFlagBits stage;

	// The path to the shader source.
	std::string source;

	// Entry Point.
	std::string entry;
};




// Vertex Input description for pipeline.
struct VKIGfxPipelineVertexInput
{
	// Vertx Input Attributes.
	std::vector<VkVertexInputAttributeDescription> attributes;

	// Vertx Input Bindings.
	std::vector<VkVertexInputBindingDescription> bindings;
};




 // Rasterizer for the pipeline.
struct VKIGfxPipelineRasterizer
{
	// The Polygon Rasterization Mode, Fill, Line or Point.
	VkPolygonMode polygonMode;

	// The line rasterization width.
	float lineWidth;

	// Culling mode.
	VkCullModeFlagBits cullMode;

	// Front Face CW or CCW
	VkFrontFace frontFace;
};




// Depth & Stencil for the pipeline.
struct VKIGfxPipelineDepthStencil
{
	// Enable/Disable Depth.
	VkBool32 isDepth;

	// Enable/Disable Depth Write.
	VkBool32 isDepthWrite;

	// Depth Compare Operation.
	VkCompareOp compareOp;

	// Enable/Disable Depth.
	VkBool32 isStencil;

	// Stencil Front Face Operation.
	VkStencilOpState stencilOpFront;

	// Stencil Back Face Operation.
	VkStencilOpState stencilOpBack;
};




// Blending data for the pipeline.
struct VKIGfxPipelineBlending
{
	// Enable/Disable Blending.
	VkBool32 isBlending;

	// Source Color Blend Factor.
	VkBlendFactor srcColor;

	// Destination Color Blend Factor.
	VkBlendFactor dstColor;

	// Color Blend operatoin.
	VkBlendOp opColor;

	// Source Alpha Blend Factor.
	VkBlendFactor srcAlpha;

	// Destination Alpha Blend Factor.
	VkBlendFactor dstAlpha;

	// Color Alpha operatoin.
	VkBlendOp opAlpha;

	// Color Write Mask.
	VkColorComponentFlags mask;
};













// VKIGraphicsPipeline:
//    - Handle vulkan command buffers.
//
class VKIGraphicsPipeline
{
public:
	// Construct.
	VKIGraphicsPipeline();

	// Destruct.
	~VKIGraphicsPipeline();

	// Return the vulkan handle.
	inline VkPipeline Get() const { return mHandle; }

	// Return pipeline layout.
	inline VkPipelineLayout GetLayout() const { return mLayout; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Create vulkan pipeline.
	void CreatePipeline(VKIDevice* owner, VKIRenderPass* renderPass);

	// Destroy vulkan graphics pipeline.
	void Destroy();

	// Return the renderpass this pipeline is part of.
	inline VKIRenderPass* GetRenderPass() const { return mRenderPass; }

	// Recreate the pipeline.
	void Recreate();

public:
	// Set shader source.
	void SetShader(VkShaderStageFlagBits stage, const std::string& src);
	void SetShader(VkShaderStageFlagBits stage, const std::string& src, const std::string& entry);

	// Set Vertex Input Description.
	void SetVertexInput(uint32_t index, uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);
	void SetVertexBinding(uint32_t index, uint32_t binding, uint32_t stride, bool isInstance = false);
	void ClearVertexInput();

	// Set the primitive topology.
	void SetTopology(VkPrimitiveTopology topology);

	// Set Rasterizer Polygon Mode.
	void SetPolygonMode(VkPolygonMode mode);

	// Set Rasterizer Culling Mode.
	void SetCulling(VkCullModeFlagBits mode, VkFrontFace face);

	// Set Rasterizer Line Width.
	void SetLineWidth(float width);

	// Enable/Disable Depth & Depth Write.
	void SetDepth(bool isEnabled, bool isWriteEnabled);

	// Set Depth Compare Operation.
	void SetDepthOp(VkCompareOp op);

	// Enable/Disable Stencil & set stencil operations.
	void SetStencil(bool isEnabled);
	void SetStencil(bool isEnabled, VkStencilOpState front, VkStencilOpState back);

	// Set blending state for a color attachment.
	void SetBlendingEnabled(uint32_t index, bool isEnabled, VkColorComponentFlags mask);
	void SetBlending(uint32_t index, VkBlendFactor src, VkBlendFactor dst, VkBlendOp op);
	void SetBlending(uint32_t index, VkBlendFactor srcColor, VkBlendFactor dstColor, VkBlendOp opColor,
		VkBlendFactor srcAlpha, VkBlendFactor dstAlpha, VkBlendOp opAlpha);

	// Set descriptor layout to be used by this pipeline.
	void SetDescriptorLayout(VKIDescriptorLayout* descriptorLayout);

	// Set a push constants.
	void SetPushConstant(uint32_t index, uint32_t offset, uint32_t size, VkShaderStageFlags stage);

	// Clear Push Constants for this pipeline.
	void ClearPushConstants();

	// Set pipeline viewport & scissors.
	void SetViewport(VkViewport viewport, VkRect2D scissors);

	// Add new dynamic state.
	void AddDynamicState(VkDynamicState state);

	// Remove existing dynamic state.
	void RemoveDynamicState(VkDynamicState state);

private:
	// Read shader source from file.
	void ReadShaderFile(const std::string& filename, std::vector<uint8_t>& outCode);

	// Setup the shaders for graphics pipeline creation.
	void SetupPipelineShaders(std::vector<VkPipelineShaderStageCreateInfo>& outPipelineShaders);

	// Create the pipeline layout using the descriptor layout & push constants.
	void CreatePiplineLayout();

private:
	// Vulkan Pipeline Handle.
	VkPipeline mHandle;

	// The device that owns this pipline.
	VKIDevice* mVKDevice;

	// Shaders used by this pipeline.
	std::map<VkShaderStageFlagBits, VKIGfxPipelineSource> mSource;

	// The subpass in the renderpass this pipeline will be used.
	uint32_t mSubpass;

	// The renderpass this pipeline is part of.
	VKIRenderPass* mRenderPass;

	// Vertex Input Description.
	VKIGfxPipelineVertexInput mVertexInput;

	// Primitive Assembly.
	VkPrimitiveTopology mTopology;

	// Pipeline Viewport.
	VkViewport mViewport;

	// Pipeline Scissor.
	VkRect2D mScissors;

	// Pipeline Rasterizer.
	VKIGfxPipelineRasterizer mRasterizer;

	// Pipeline Depth & Stencil.
	VKIGfxPipelineDepthStencil mDepthStencil;

	// Pipeline Blending.
	std::vector<VKIGfxPipelineBlending> mBlending;

	// Pipline Descriptor Layout.
	VKIDescriptorLayout* mDescriptorLayout;

	// The Pipeline Layout.
	VkPipelineLayout mLayout;

	// The Pipeline Push Constants.
	std::vector<VkPushConstantRange> mPushConstant;

	// Dynamic States to enable in this pipeline.
	std::set<VkDynamicState> mDynamicStates;
};

