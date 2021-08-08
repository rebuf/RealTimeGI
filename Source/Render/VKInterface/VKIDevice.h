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
#include "Core/Delegate.h"
#include "vulkan/vulkan.h"

#include <set>
#include <string>





class VKIInstance;
class VKICommandBuffer;
class VKIFence;







// VKIDevice:
//    - Interface for Vulkan Device.
//
class VKIDevice
{
public:
	// Construct.
	VKIDevice();

	// Destruct.
	~VKIDevice();

	// Return the vulkan handle.
	inline VkDevice Get() const { return mHandle; }

	// Return vulkan instance.
	inline VKIInstance* GetInstance() const { return mVKInstance; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Create vulkan device form the vulkan instance.
	void CreateDevice(VKIInstance* owner);

	// Destroy Device.
	void Destroy();

	// Create Vulkan Command Pools.
	void CreateCommandPool();

	// Create Vulkan Command Buffers.
	void CreateCommandBuffers(uint32_t count);

	// Return Draw Command Buffer.
	inline VKICommandBuffer* GetDrawCmd() const { return mDrawCmdBuffer.get(); }

	// Return graphics queue.
	inline VkQueue GetGFXQueue() const { return mGFXQueue; }

	// Return present queue.
	inline VkQueue GetPresentQueue() const { return mPresentQueue; }

	// Find the memory type index that match filter and properties.
	uint32_t FindMemory(uint32_t filter, VkMemoryPropertyFlags properties);

	// Return vulkan command pool created by this device.
	inline VkCommandPool GetCmdPool() { return mCmdPool; }

public:
	// Begin Transient Command Buffer.
	VkCommandBuffer BeginTransientCmd();

	// End Transient Command Buffer.
	void EndTransientCmd(VkCommandBuffer cmd, const Delegate<>& callback);

	// Submit all transient command buffers.
	void SubmitTransientCmd();

	// Wait for transient commands to be fully submitted.
	void WaitForTransientCmd();

private:
	// Vulkan Device Handle.
	VkDevice mHandle;

	// Vulkan Instance.
	VKIInstance* mVKInstance;

	// List of required extensions to be enabled by this device.
	std::set<std::string> mReqExtensions;

	// Graphics Queue for submitting queue commands.
	VkQueue mGFXQueue;

	// Present queue for submitting present commands.
	VkQueue mPresentQueue;

	// Command Pool for this device.
	VkCommandPool mCmdPool;

	// Command Pool used for transent commands buffers.
	VkCommandPool mTransientCmdPool;

	// Command Buffer used to hold draw commands.
	UniquePtr<VKICommandBuffer> mDrawCmdBuffer;

	// Command Buffer used to hold transient commands for a single frame.
	std::vector< UniquePtr<VKICommandBuffer> > mTransientCmdBuffers;

	// Event called when transient commands get submited & finished.
	MultiDelegate<> mTransientSubmitEvent;

	// Fence for submiting transient commands.
	Ptr<VKIFence> mTransientSubmitFence;
};
