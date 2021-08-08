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
class VKIFramebuffer;
class VKIFence;
class VKISemaphore;
class VKIImageView;





// VKISwapChain:
//    - Handle vulkan swapchain and anything related to it.
//
class VKISwapChain
{
public:
	// Construct.
	VKISwapChain();

	// Destruct.
	~VKISwapChain();

	// Create Vulkan Swapchain.
	void CreateSwapchain(VKIDevice* owner);

	// Destroy the swapchain.
	void Destroy();

	// Return the vulkan handle.
	inline VkSwapchainKHR Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Return the number of swapchain images.
	inline uint32_t GetNumImages() { return (uint32_t)mImages.size(); }

	// Return the number of swapchain images.
	inline VkImage GetImage(uint32_t index) { return mImages[index]; }

	// Acquire next available swapchain image.
	uint32_t AcquireNextImage(VKISemaphore* smImage, VKIFence* fnFrame);

	// Present swapchain image to surface.
	void PresentImage(uint32_t imgIndex, VKISemaphore* smRender);

	// Return true if the swapchain need to be recreated.
	inline bool NeedRecreate() const { return mNeedRecreate; };

	// Recreate a swapchain.
	void Recreate();

	// Return the render pass that describe the swapchain images as attachments.
	inline VKIRenderPass* GetRenderPass() const { return mRenderPass.get(); }

	// Return the framebuffer that attach the swapchain images.
	inline VKIFramebuffer* GetFrameBuffer(uint32_t index) const { return mFramebuffer[index].get(); }

	// Return the extent of the swapchain images.
	inline VkExtent2D GetExtent() { return mExtent; }

	// Return the surface format
	inline VkSurfaceFormatKHR GetFormat() { return mSurfaceFormat; }


private:
	// Query swapchain perperties from the device.
	void QuerySwapchainProperties();

	// Create image views for the swapchain images to be used by the swapchain render pass.
	void CreateImageViews();

	// Create depth buffer to be used by the swapchain render pass.
	// void CreateDepthBuffer();

	// Create Framebuffer from swpachain image views.
	void CreateFramebuffer();

	// Create a render pass for the swapchain.
	void CreateRenderPass();

private:
	// The device that owns this swapchain.
	VKIDevice* mVKDevice;

	// Vulkan Swapchain.
	VkSwapchainKHR mHandle;

	// The Surface Format used for the swapchain images.
	VkSurfaceFormatKHR mSurfaceFormat;

	// The Present Mode used for presenting the rendered images.
	VkPresentModeKHR mPresentMode;

	// The Surface Extent, used as the size of the swapchain images.
	VkExtent2D mExtent;

	// The number of swpachain images, this value is queried from surface capabilities.
	uint32_t mImgCount;

	// The transformation on the swapcahin images, we don't do any transformation this is just the currrent surface transform.
	VkSurfaceTransformFlagBitsKHR mSurfaceTransform;

	// The Swapchain images.
	std::vector<VkImage> mImages;

	// The Swapchain image views.
	std::vector< Ptr<VKIImageView> > mImageViews;
	 
	// Swapchain render pass.
	UniquePtr<VKIRenderPass> mRenderPass;

	// Swapchain framebuffer.
	std::vector< UniquePtr<VKIFramebuffer> > mFramebuffer;

	// Frame fences that uses a swapchain image.
	std::vector<VKIFence*> mFrameFences;

	// Flag used to check if the swapchain need to be recreated.
	bool mNeedRecreate;
};
