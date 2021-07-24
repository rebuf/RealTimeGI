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




#include "VKICommandBuffer.h"
#include "VKIDevice.h"





VKICommandBuffer::VKICommandBuffer()
	: mVKDevice(nullptr)
	, mCmdPool(VK_NULL_HANDLE)
{

}


VKICommandBuffer::~VKICommandBuffer()
{

}


void VKICommandBuffer::Destroy()
{
	// Destroy allocated vulkan command buffers.
	vkFreeCommandBuffers(mVKDevice->Get(), mCmdPool, (uint32_t)mCmdBuffers.size(), mCmdBuffers.data());

	//...
	mCmdBuffers.clear();
}


void VKICommandBuffer::CreateCmdBuffer(VKIDevice* owner, VkCommandPool pool, uint32_t count)
{
	mVKDevice = owner;
	mCmdPool = pool;
	mCmdBuffers.resize(count);

	// Allocate Command Buffers...
	VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = mCmdPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = (uint32_t)mCmdBuffers.size();

	VkResult result = vkAllocateCommandBuffers(mVKDevice->Get(), &cmdBufferAllocInfo, mCmdBuffers.data());
	CHECK(result == VK_SUCCESS && "Failed to allocate command buffers!");
}
