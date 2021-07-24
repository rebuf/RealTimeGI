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





#include "VKISwapChain.h"
#include "VKIInstance.h"
#include "VKIDevice.h"
#include "VKIRenderPass.h"
#include "VKIFramebuffer.h"
#include "VKISync.h"
#include "VKIImage.h"




#include "glm/common.hpp"
#include <array>









VKISwapChain::VKISwapChain()
	: mVKDevice(nullptr)
	, mImgCount(0)
	, mHandle(VK_NULL_HANDLE)
	, mPresentMode(VK_PRESENT_MODE_FIFO_KHR)
	, mSurfaceTransform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	, mNeedRecreate(false)
{
	mSurfaceFormat = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	mExtent = { 0, 0 };
}


VKISwapChain::~VKISwapChain()
{

}


void VKISwapChain::Destroy()
{
	// Destroy ImageViews & Framebuffers...
	for (size_t i = 0; i < mImages.size(); ++i)
	{
		mFramebuffer[i]->Destroy();
		mImageViews[i]->Destroy();
	}

	// Destroy Render Pass.
	mRenderPass->Destroy();

	// Destroy Swapchain.
	vkDestroySwapchainKHR(mVKDevice->Get(), mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
}



void VKISwapChain::QuerySwapchainProperties()
{
	// Get surface properties...
	VKIInstance* vkinstance = mVKDevice->GetInstance();
	const VKISurface& vksurface = vkinstance->GetSurface();


	// 1. Choose The Format:
	mSurfaceFormat = vksurface.formats[0]; // Initialize with formats[0] incase we did not found the one we need.

	for (const auto& format : vksurface.formats)
	{
		if ( format.format == VK_FORMAT_B8G8R8A8_UNORM
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			mSurfaceFormat = format;
			break;
		}
	}


	// 2. Choose The Present Mode: 
	mPresentMode = VK_PRESENT_MODE_FIFO_KHR; // Initialize to V-Sync incase we did not found the one we need.

	for (const auto& mode : vksurface.presentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			mPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}


	// 3. Choose the extent from surface capabilities: 
	if (vksurface.capabilities.currentExtent.width != UINT32_MAX)
	{
		// We are not allowed to have different exten than the surface just set it to the surface value.
		mExtent = vksurface.capabilities.currentExtent;
	}
	else
	{
		// We are allowed to have different extent than the one in the surface, Therefore,
		// are going to use glfw to get the window size and set it as our extent.
		mExtent = vkinstance->GetFramebufferSize();

		// Clamp Width to surface Min/Max...
		mExtent.width = glm::clamp(mExtent.width, 
			vksurface.capabilities.minImageExtent.width,
			vksurface.capabilities.maxImageExtent.width);

		// Clamp Height to surface Min/Max...
		mExtent.height = glm::clamp(mExtent.height,
			vksurface.capabilities.minImageExtent.height,
			vksurface.capabilities.maxImageExtent.height);
	}

	// 4. Choose how many images we want to have in the swapchain. it is recommended
	//    to request at least one more image than the minimum.
	mImgCount = vksurface.capabilities.minImageCount + 1;

	// 5. Current surface transform.
	mSurfaceTransform = vksurface.capabilities.currentTransform;
}


void VKISwapChain::CreateSwapchain(VKIDevice* owner)
{
	mVKDevice = owner;
	VKIInstance* vkinstance = mVKDevice->GetInstance();

	// Query & Set Swapchain Properties...
	QuerySwapchainProperties();


	// Create Swapchain...
	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = vkinstance->GetSurface().handle;
	swapchainInfo.preTransform = mSurfaceTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // No Alpha for window.
	swapchainInfo.presentMode = mPresentMode;
	swapchainInfo.clipped = VK_TRUE; // Discard regions that is not visiable on the surface.


	// Swapchain Images Info.
	swapchainInfo.minImageCount = mImgCount;
	swapchainInfo.imageFormat = mSurfaceFormat.format;
	swapchainInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	swapchainInfo.imageExtent = mExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


	// Swapchain images ownership between Queue Families.
	if (vkinstance->GetQueues().IsPresentUnique())
	{
		// Queue Families Array...
		auto uniqueQueues = vkinstance->GetQueues().GetUniqueQueues();

		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = (uint32_t)uniqueQueues.size();
		swapchainInfo.pQueueFamilyIndices = uniqueQueues.data();
	}
	else
	{
		// We only have one queue make access exclusive
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	

	VkResult result = vkCreateSwapchainKHR(mVKDevice->Get(), &swapchainInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS && "Failed to create swap chain!");


	// Get The allocated Swapchain Images...
	vkGetSwapchainImagesKHR(mVKDevice->Get(), mHandle, &mImgCount, nullptr);

	mImages.resize(mImgCount);
	vkGetSwapchainImagesKHR(mVKDevice->Get(), mHandle, &mImgCount, mImages.data());


	//...
	CreateImageViews();
	CreateRenderPass();
	CreateFramebuffer();


	mFrameFences.resize(mImgCount);

}


void VKISwapChain::CreateImageViews()
{
	mImageViews.resize(mImages.size());

	for (size_t i = 0; i < mImages.size(); ++i)
	{
		mImageViews[i] = Ptr<VKIImageView>(new VKIImageView());
		mImageViews[i]->SetType(VK_IMAGE_VIEW_TYPE_2D);
		mImageViews[i]->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
		mImageViews[i]->Create(mVKDevice, mImages[i], mSurfaceFormat.format);
	}
}


void VKISwapChain::CreateRenderPass()
{
	mRenderPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());

	// Color Attachment...
	mRenderPass->SetColorAttachment(
		0, mSurfaceFormat.format,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		true
	);


	// Dependency...
	mRenderPass->AddDependency(
		VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	);


	mRenderPass->CreateRenderPass(mVKDevice);
}


void VKISwapChain::CreateFramebuffer()
{
	mFramebuffer.resize(mImages.size());

	for (size_t i = 0; i < mImages.size(); ++i)
	{
		mFramebuffer[i] = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
		mFramebuffer[i]->SetSize(mExtent);
		mFramebuffer[i]->SetImgView(0, mImageViews[i].get());
		mFramebuffer[i]->CreateFrameBuffer(mVKDevice, mRenderPass.get());
	}

}


uint32_t VKISwapChain::AcquireNextImage(VKISemaphore* smImage, VKIFence* fnFrame)
{
	// Try to acquire the next swapchain image to render on.
	uint32_t imageIndex;

	VkResult result = vkAcquireNextImageKHR(
		mVKDevice->Get(),
		mHandle,
		UINT64_MAX, 
		smImage->Get(), 
		VK_NULL_HANDLE, 
		&imageIndex
	);


	// The swapchain is incompatible with the sruface and can not be used for rendering
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		mNeedRecreate = true;
		return INVALID_UINDEX;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) // Mismatch between the swapchain and the surface properties
	{
		CHECK(0 && "Failed to acquire swap chain image!");
		return INVALID_UINDEX;
	}


	// Image is currently used by another frame? wait for the other frame to finish.
	if (mFrameFences[imageIndex] != nullptr)
	{
		mFrameFences[imageIndex]->Wait(UINT64_MAX);
	}

	mFrameFences[imageIndex] = fnFrame;
	return imageIndex;
}


void VKISwapChain::PresentImage(uint32_t imgIndex, VKISemaphore* smRender)
{
	// Present swapchain image to surface.
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	std::array<VkSemaphore, 1> smWait = { smRender->Get() };
	presentInfo.waitSemaphoreCount = (uint32_t)smWait.size();
	presentInfo.pWaitSemaphores = smWait.data();

  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &mHandle;
  presentInfo.pImageIndices = &imgIndex;

	VkResult result = vkQueuePresentKHR(mVKDevice->GetPresentQueue(), &presentInfo);

	// Failed to present? Mismatch between surface and swapchain?
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		mNeedRecreate = true;
	}
	else if (result != VK_SUCCESS)
	{
		CHECK(0 && "Failed to present swap chain image!");
		return;
	}

}


void VKISwapChain::Recreate()
{
	mNeedRecreate = false;

	CHECK(0 && "TODO: Implement Recreation of swpchain.");
}
