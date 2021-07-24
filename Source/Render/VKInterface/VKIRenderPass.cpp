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




#include "VKIRenderPass.h"
#include "VKIDevice.h"
#include "VKIFramebuffer.h"
#include "VKICommandBuffer.h"


#include <array>




VKIRenderPass::VKIRenderPass()
	: mVKDevice(nullptr)
	, mHandle(VK_NULL_HANDLE)
{
	mDepthAttachment = VKIRenderPassAttachment{};
	mDepthAttachment.desc.format = VK_FORMAT_UNDEFINED;

}


VKIRenderPass::~VKIRenderPass()
{

}


void VKIRenderPass::SetColorAttachment(uint32_t index, VkFormat format, VkImageLayout layout,
	VkImageLayout iniLayout, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, bool isStore)
{
	if (mColorAttachments.size() < index + 1)
		mColorAttachments.resize(index + 1);

	VKIRenderPassAttachment attachment{};
	attachment.desc.format = format;
	attachment.desc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.desc.loadOp = loadOp;
	attachment.desc.storeOp = isStore ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

	attachment.desc.initialLayout = iniLayout;
	attachment.desc.finalLayout = finalLayout;

	// No Stencil...
	attachment.desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; 
	attachment.desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// The renderpass reference layout.
	attachment.layout = layout;

	mColorAttachments[index] = attachment;
}



void VKIRenderPass::SetDepthAttachment(VkFormat format, VkImageLayout layout, VkImageLayout iniLayout, 
	VkImageLayout finalLayout, VkAttachmentLoadOp loadOp, bool isStore, bool isStencil)
{
	VKIRenderPassAttachment attachment{};
	attachment.desc.format = format;
	attachment.desc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.desc.loadOp = loadOp;
	attachment.desc.storeOp = isStore ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

	attachment.desc.initialLayout = iniLayout;
	attachment.desc.finalLayout = finalLayout;

	// Has Stencil?
	if (isStencil)
	{
		attachment.desc.stencilLoadOp = attachment.desc.loadOp;
		attachment.desc.stencilStoreOp = attachment.desc.storeOp;
	}
	else
	{
		attachment.desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}


	// The renderpass reference layout.
	attachment.layout = layout;

	mDepthAttachment = attachment;
}


bool VKIRenderPass::HasDepth()
{
	return mDepthAttachment.desc.format != VK_FORMAT_UNDEFINED;
}


uint32_t VKIRenderPass::GetNumColorAttachments()
{
	return (uint32_t)mColorAttachments.size();
}


void VKIRenderPass::ClearAllAttachments()
{
	mColorAttachments.clear();
	mDepthAttachment.desc.format = VK_FORMAT_UNDEFINED;
}


std::vector<VkAttachmentDescription> VKIRenderPass::GetAttachments()
{
	std::vector<VkAttachmentDescription> attachments;
	attachments.resize(HasDepth() ? mColorAttachments.size() + 1 : mColorAttachments.size());

	// Color...
	for (size_t i = 0; i < mColorAttachments.size(); ++i)
	{
		attachments[i] = mColorAttachments[i].desc;
	}

	// Depth...
	if (HasDepth())
	{
		attachments.back() = mDepthAttachment.desc;
	}

	return attachments;
}


std::vector<VkAttachmentReference> VKIRenderPass::GetAttachmentsRef()
{
	std::vector<VkAttachmentReference> attachmentsRef;
	attachmentsRef.resize(HasDepth() ? mColorAttachments.size() + 1 : mColorAttachments.size());

	// Color...
	for (size_t i = 0; i < mColorAttachments.size(); ++i)
	{
		attachmentsRef[i].attachment = (uint32_t)i;
		attachmentsRef[i].layout = mColorAttachments[i].layout;
	}

	// Depth...
	if (HasDepth())
	{
		attachmentsRef.back().attachment = (uint32_t)(attachmentsRef.size() - 1);
		attachmentsRef.back().layout = mDepthAttachment.layout;
	}

	return attachmentsRef;
}


void VKIRenderPass::CreateRenderPass(VKIDevice* owner)
{
	CHECK(!mColorAttachments.empty() || HasDepth() && "No Attachments!");
	mVKDevice = owner;

	// Attachments List...
	auto attachments = GetAttachments();
	auto attachmentsRef = GetAttachmentsRef();

	// The Subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Graphics Pipeline.
	subpass.colorAttachmentCount = HasDepth() ? (uint32_t)(attachments.size() - 1) : (uint32_t)attachments.size();
	subpass.pColorAttachments = attachmentsRef.data();
	subpass.pDepthStencilAttachment = HasDepth() ? &(attachmentsRef.back()) : nullptr;


	// Create Render Pass...
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = (uint32_t)mDependencies.size();
	renderPassInfo.pDependencies = mDependencies.data();

	VkResult result = vkCreateRenderPass(mVKDevice->Get(), &renderPassInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);

}


void VKIRenderPass::Destroy()
{
	vkDestroyRenderPass(mVKDevice->Get(), mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
}


void VKIRenderPass::SetClearValues(const std::vector<VkClearValue>& clearValues)
{
	mClearValues = clearValues;
}


void VKIRenderPass::ClearDependencies()
{
	mDependencies.clear();
}


void VKIRenderPass::AddDependency(uint32_t src, uint32_t dst, VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
	VkSubpassDependency dependency{};
	dependency.srcSubpass = src;
	dependency.dstSubpass = dst;
	dependency.srcStageMask = srcStage;
	dependency.dstStageMask = dstStage;
	dependency.srcAccessMask = srcAccess;
	dependency.dstAccessMask = dstAccess;

	mDependencies.emplace_back(dependency);
}

void VKIRenderPass::AddDependency(uint32_t src, uint32_t dst, VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkDependencyFlags flags)
{
	VkSubpassDependency dependency{};
	dependency.srcSubpass = src;
	dependency.dstSubpass = dst;
	dependency.srcStageMask = srcStage;
	dependency.dstStageMask = dstStage;
	dependency.srcAccessMask = srcAccess;
	dependency.dstAccessMask = dstAccess;
	dependency.dependencyFlags = flags;

	mDependencies.emplace_back(dependency);
}



void VKIRenderPass::Begin(VKICommandBuffer* cmdBuffer, VKIFramebuffer* framebuffer, const glm::ivec4& viewport)
{
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = mHandle;
	renderPassBeginInfo.framebuffer = framebuffer->Get();
	renderPassBeginInfo.renderArea = { viewport.x, viewport.y, (uint32_t)viewport.z, (uint32_t)viewport.w };
	renderPassBeginInfo.clearValueCount = (uint32_t)mClearValues.size();
	renderPassBeginInfo.pClearValues = mClearValues.data();

	vkCmdBeginRenderPass(cmdBuffer->GetCurrent(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void VKIRenderPass::End(VKICommandBuffer* cmdBuffer)
{
	vkCmdEndRenderPass(cmdBuffer->GetCurrent());
}
