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
#include "glm/vec4.hpp"

#include <vector>




class VKIDevice;
class VKICommandBuffer;
class VKIFramebuffer;





// Data for a single attachment in for render pass.
struct VKIRenderPassAttachment
{
	// Attachment Description
	VkAttachmentDescription desc;

	// Attachment layout in the render pass.
	VkImageLayout layout;
};





// VKIRenderPass:
//    - Interface for Vulkan RenderPass.
//
class VKIRenderPass
{
public:
	// Construct.
	VKIRenderPass();

	// Destruct.
	~VKIRenderPass();

	// Return the vulkan handle.
	inline VkRenderPass Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Add/Replace color attachment at index.
	// @param format: the format of the attachment.
	// @param layout: the layout of the attachment used in the render pass.
	// @param iniLayout: the attachment initial layout before the render pass.
	// @param finalLayout: the attachment final layout after the render pass.
	// @param isLoadClear: clear the attachment before loading.
	// @param isStore: store the attachment data after the render pass.
	void SetColorAttachment(uint32_t index, VkFormat format, VkImageLayout layout,
		VkImageLayout iniLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, bool isStore);

	// Return Color Attachment at index.
	inline const VKIRenderPassAttachment& GetColorAttachment(uint32_t index) const { CHECK(index < mColorAttachments.size() && "Invalid Index.");  return mColorAttachments[index]; }

	// Return Depth Attachment.
	inline const VKIRenderPassAttachment& GetDepthAttachment() const { return mDepthAttachment; }


	// Add/Replace depth attachment.
	// @param format: the format of the attachment.
	// @param layout: the layout of the attachment used in the render pass.
	// @param iniLayout: the attachment initial layout before the render pass.
	// @param finalLayout: the attachment final layout after the render pass.
	// @param isLoadClear: clear the attachment before loading.
	// @param isStore: store the attachment data after the render pass.
	void SetDepthAttachment(VkFormat format, VkImageLayout layout, VkImageLayout iniLayout, 
		VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, bool isStore, bool isStencil);

	// Return true if the renderpass has depth attachment.
	bool HasDepth();

	// Return the number of color attachments in this render pass.
	uint32_t GetNumColorAttachments();

	// Clear all attachments from this renderpass.
	void ClearAllAttachments();

	// Add new Subpass Dependency
	// @param src/dst: source and destination sub passes or VK_SUBPASS_EXTERNAL.
	// @param src/dst stage: source and destination pipeline stage.
	// @param src/dst access: source and destination access.
	void AddDependency(uint32_t src, uint32_t dst, VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess);

	// Add new Subpass Dependency
	// @param src/dst: source and destination sub passes or VK_SUBPASS_EXTERNAL.
	// @param src/dst stage: source and destination pipeline stage.
	// @param src/dst access: source and destination access.
	void AddDependency(uint32_t src, uint32_t dst, VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkDependencyFlags flags);

	// Set clear value for color attachments.
	void SetClearValues(const std::vector<VkClearValue>& clearValues);

	// Clear all added dependencies.
	void ClearDependencies();

	// Create vulkan render pass.
	void CreateRenderPass(VKIDevice* owner);

	// Destroy vulkan render pass.
	void Destroy();

	// Begin The Render Pass.
	void Begin(VKICommandBuffer* cmdBuffer, VKIFramebuffer* framebuffer, const glm::ivec4& viewport);

	// End The Render Pass.
	void End(VKICommandBuffer* cmdBuffer);

private:
	// List all attachments.
	std::vector<VkAttachmentDescription> GetAttachments();

	// List all attachments references.
	std::vector<VkAttachmentReference> GetAttachmentsRef();

private:
	// The device that owns this renderpass.
	VKIDevice* mVKDevice;

	// Vulkan Swapchain.
	VkRenderPass mHandle;

	// Color attachments description in this render pass.
	std::vector<VKIRenderPassAttachment> mColorAttachments;

	// Depth attachments description in this render pass.
	VKIRenderPassAttachment mDepthAttachment;

	// Subpass Dependencies.
	std::vector<VkSubpassDependency> mDependencies;

	// Clear Values used to clear the attachments.
	std::vector<VkClearValue> mClearValues;
};

