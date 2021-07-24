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





class VKIDevice;






// VKISemaphore:
//     - Handle vulkan semaphore.
//
class VKISemaphore
{
public:
	// Construct.
	VKISemaphore();

	// Destruct.
	~VKISemaphore();

	// Return the vulkan handle.
	inline VkSemaphore Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }
	
	// Create vulkan semaphore.
	void CreateSemaphore(VKIDevice* owner);

	// Destroy vulkan Semaphore.
	void Destroy();

private:
	// Vulkan Semaphore Handle.
	VkSemaphore mHandle;

	// The device that owns this semaphore.
	VKIDevice* mVKDevice;
};






// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 






// VKISemaphore:
//     - Handle vulkan Fence.
//
class VKIFence
{
public:
	// Construct.
	VKIFence();

	// Destruct.
	~VKIFence();

	// Return the vulkan handle.
	inline VkFence Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Create vulkan Fence.
	void CreateFence(VKIDevice* owner, bool isSignaled);

	// Destroy vulkan Fence.
	void Destroy();

	// Perform fence wait.
	void Wait(uint64_t timeout);

	// Perform fence reset to signaled state.
	void Reset();

private:
	// Vulkan Fence Handle.
	VkFence mHandle;

	// The device that owns this Fence.
	VKIDevice* mVKDevice;
};
