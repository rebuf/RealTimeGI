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



#include "VKISync.h"
#include "VKIDevice.h"







VKISemaphore::VKISemaphore()
	: mHandle(VK_NULL_HANDLE)
	, mVKDevice(nullptr)
{

}


VKISemaphore::~VKISemaphore()
{

}


void VKISemaphore::CreateSemaphore(VKIDevice* owner)
{
	mVKDevice = owner;

	// Create Semaphore.
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult result = vkCreateSemaphore(mVKDevice->Get(), &semaphoreInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);
}


void VKISemaphore::Destroy()
{
	vkDestroySemaphore(mVKDevice->Get(), mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
}







// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 





VKIFence::VKIFence()
	: mHandle(VK_NULL_HANDLE)
	, mVKDevice(nullptr)
{

}


VKIFence::~VKIFence()
{

}


void VKIFence::CreateFence(VKIDevice* owner, bool isSignaled)
{
	mVKDevice = owner;

	// Create Fence...
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	// Create in signaled state?
	if (isSignaled)
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  

	VkResult result = vkCreateFence(mVKDevice->Get(), &fenceInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);

}


void VKIFence::Destroy()
{
	vkDestroyFence(mVKDevice->Get(), mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
}


void VKIFence::Wait(uint64_t timeout)
{
	vkWaitForFences(mVKDevice->Get(), 1, &mHandle, VK_TRUE, timeout);
}


void VKIFence::Reset()
{
	vkResetFences(mVKDevice->Get(), 1, &mHandle);
}
