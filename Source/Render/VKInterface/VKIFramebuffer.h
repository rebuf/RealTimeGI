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



class VKIDevice;
class VKIRenderPass;
class VKIImageView;





// VKIFramebuffer:
//    - Interface for Vulkan Framebuffer.
//
class VKIFramebuffer
{
public:
	// Construct.
	VKIFramebuffer();

	// Destruct.
	~VKIFramebuffer();

	// Return the vulkan handle.
	inline VkFramebuffer Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Set image view to index.
	void SetImgView(uint32_t idx, VKIImageView* imgView);

	// Set Framebuffer size.
	void SetSize(VkExtent2D size);

	// Set Framebuffer layers.
	void SetLayers(uint32_t layers);

	// Clear all image views added to this framebuffer.
	void ClearImgViews();

	// Create vulkan framebuffer.
	void CreateFrameBuffer(VKIDevice* owner, VKIRenderPass* renderPass);

	// Destroy vulkan framebuffer.
	void Destroy();

private:
	// The device that owns this framebuffer.
	VKIDevice* mVKDevice;

	// Vulkan Swapchain.
	VkFramebuffer mHandle;

	// The RenderPass.
	VKIRenderPass* mRenderPass;

	// Image views for the framebuffer attachemnts.
	std::vector<VKIImageView*> mImgViews;

	// Framebuffer size.
	VkExtent2D mSize;

	// Number of layers.
	uint32_t mLayers;
};

