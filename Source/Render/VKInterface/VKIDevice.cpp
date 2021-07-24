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





#include "VKIDevice.h"
#include "VKIInstance.h"
#include "VKICommandBuffer.h"
#include "VKISync.h"


#include <vector>






VKIDevice::VKIDevice()
	: mVKInstance(nullptr)
	, mHandle(VK_NULL_HANDLE)
	, mGFXQueue(VK_NULL_HANDLE)
	, mPresentQueue(VK_NULL_HANDLE)
	, mCmdPool(VK_NULL_HANDLE)
{
	// Required Vulkan Extensions that we need the physical device to support.
	mReqExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

}


VKIDevice::~VKIDevice()
{

}


void VKIDevice::Destroy()
{
	// Destroy Fences...
	mTransientSubmitFence->Destroy();

	// Destroy Command Pools...
	vkDestroyCommandPool(mHandle, mCmdPool, nullptr);
	vkDestroyCommandPool(mHandle, mTransientCmdPool, nullptr);

	// Destroy Vulkan Device.
	vkDestroyDevice(mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
	mCmdPool = VK_NULL_HANDLE;
}


void VKIDevice::CreateDevice(VKIInstance* owner)
{
	mVKInstance = owner;

	// Device Queues...
	const float queuePriorities = 1.0f;
	auto uniqueQueueFamilies = mVKInstance->GetQueues().GetUniqueQueues();
	std::vector<VkDeviceQueueCreateInfo> queuesInfo(uniqueQueueFamilies.size());


	for (size_t i = 0; i < queuesInfo.size(); ++i)
	{
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount = 1;
		queueInfo.queueFamilyIndex = uniqueQueueFamilies[i];
		queueInfo.pQueuePriorities = &queuePriorities;

		queuesInfo[i] = queueInfo;
	}

	// Features in the physical device we want to enable/use through our logical device.
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	// Required Ext..
	std::vector<const char*> reqExt;
	reqExt.reserve(mReqExtensions.size());

	for (const auto& ext : mReqExtensions)
		reqExt.push_back(ext.c_str());

	// Create The Logical Device...
	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = (uint32_t)queuesInfo.size();
	deviceInfo.pQueueCreateInfos = queuesInfo.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledExtensionCount = (uint32_t)reqExt.size();
	deviceInfo.ppEnabledExtensionNames = reqExt.data();

	VkResult result = vkCreateDevice(owner->GetPhysicalDevice(), &deviceInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);

	// Get Device Queues...
	vkGetDeviceQueue(mHandle, owner->GetQueues().graphics, 0, &mGFXQueue);

	if (owner->GetQueues().IsPresentUnique())
	{
		vkGetDeviceQueue(mHandle, owner->GetQueues().present, 0, &mPresentQueue);
	}
	else
	{
		mPresentQueue = mGFXQueue;
	}


	// Create Fences...
	mTransientSubmitFence = Ptr<VKIFence>(new VKIFence());
	mTransientSubmitFence->CreateFence(this, false);

}


void VKIDevice::CreateCommandPool()
{
	// MAKING SURE THAT IT WORKS FOR NOW.
	CHECK(mPresentQueue == mGFXQueue && "For now, i will assume that present queue == graphics queue.")

	{
		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = mVKInstance->GetQueues().graphics;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkResult result = vkCreateCommandPool(mHandle, &cmdPoolInfo, nullptr, &mCmdPool);
		CHECK(result == VK_SUCCESS);
	}


	{
		VkCommandPoolCreateInfo cmdPoolInfo{};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = mVKInstance->GetQueues().graphics;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VkResult result = vkCreateCommandPool(mHandle, &cmdPoolInfo, nullptr, &mTransientCmdPool);
		CHECK(result == VK_SUCCESS);
	}

}


void VKIDevice::CreateCommandBuffers(uint32_t count)
{
	mDrawCmdBuffer = UniquePtr<VKICommandBuffer>(new VKICommandBuffer());
	mDrawCmdBuffer->CreateCmdBuffer(this, mCmdPool, count);

}


uint32_t VKIDevice::FindMemory(uint32_t filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties deviceMemProperties;
	vkGetPhysicalDeviceMemoryProperties(mVKInstance->GetPhysicalDevice(), &deviceMemProperties);

	// Serach for desired memory type, which is the type of memory in the VRAM.
	for (uint32_t i = 0; i < deviceMemProperties.memoryTypeCount; i++)
	{
		// Check if required type is supported and return its index.
		if ((filter & (1 << i)) && (deviceMemProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	// Failed...
	return VK_MAX_MEMORY_TYPES;
}


VkCommandBuffer VKIDevice::BeginTransientCmd()
{
	if (mTransientCmdBuffers.size() == 15)
	{
		SubmitTransientCmd();
		WaitForTransientCmd();
	}

	mTransientCmdBuffers.emplace_back( UniquePtr<VKICommandBuffer>(new VKICommandBuffer()) );
	mTransientCmdBuffers.back()->CreateCmdBuffer(this, mTransientCmdPool, 1);

	// Begin...
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // One Time Use...
																																 
	vkBeginCommandBuffer(mTransientCmdBuffers.back()->Get(), &beginInfo);

	return mTransientCmdBuffers.back()->Get();
}


void VKIDevice::EndTransientCmd(VkCommandBuffer cmd, const Delegate<>& callback)
{
	// End...
	vkEndCommandBuffer(cmd);

	if (callback.IsValid())
	{
		mTransientSubmitEvent.Add(callback);
	}
}


void VKIDevice::SubmitTransientCmd()
{
	if (mTransientCmdBuffers.empty())
		return;

	// All transient command buffers...
	std::vector<VkCommandBuffer> cmdBuffers;
	cmdBuffers.resize(mTransientCmdBuffers.size());

	for (size_t i = 0; i < mTransientCmdBuffers.size(); ++i)
		cmdBuffers[i] = mTransientCmdBuffers[i]->Get();


	// Submit...
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = (uint32_t)cmdBuffers.size();
	submitInfo.pCommandBuffers = cmdBuffers.data();

	vkQueueSubmit(mGFXQueue, 1, &submitInfo, mTransientSubmitFence->Get());
}	 
	 
	 
void VKIDevice::WaitForTransientCmd()
{
	if (mTransientCmdBuffers.empty())
		return;

	// Wait...
	mTransientSubmitFence->Wait(UINT32_MAX);
	mTransientSubmitFence->Reset();


	// Destroy/Free Transient Commands...
	for (size_t i = 0; i < mTransientCmdBuffers.size(); ++i)
		mTransientCmdBuffers[i]->Destroy();

	mTransientCmdBuffers.clear();


	// Signal Events...
	mTransientSubmitEvent.Execute();
	mTransientSubmitEvent.Clear();

}
