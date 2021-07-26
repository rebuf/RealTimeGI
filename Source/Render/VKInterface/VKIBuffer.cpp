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



#include "VKIBuffer.h"
#include "VKIDevice.h"
#include "VKICommandBuffer.h"






VKIBuffer::VKIBuffer()
	: mHandle(VK_NULL_HANDLE)
	, mMemory(VK_NULL_HANDLE)
	, mUsage(0)
	, mProperties(0)
	, mVKDevice(nullptr)
	, mSize(0)
{

}


VKIBuffer::~VKIBuffer()
{

}


void VKIBuffer::SetUsage(VkBufferUsageFlags usage)
{
	CHECK(!IsValid());
	mUsage = usage;
}


void VKIBuffer::SetMemoryProperties(VkMemoryPropertyFlags properties)
{
	CHECK(!IsValid());
	mProperties = properties;
}


void VKIBuffer::SetSize(VkDeviceSize size)
{
	CHECK(!IsValid());
	mSize = size;
}


void VKIBuffer::CreateBuffer(VKIDevice* owner)
{
	mVKDevice = owner;

	// Create Buffer...
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = mSize;
	bufferInfo.usage = mUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.flags = 0;

	VkResult result = vkCreateBuffer(mVKDevice->Get(), &bufferInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);


	// Allocate Device Memory.
	AllocateMemory();

}


void VKIBuffer::AllocateMemory()
{
	CHECK(IsValid());

	// Allocate...
	VkMemoryAllocateInfo memInfo{};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	GetMemoryInfo(memInfo.allocationSize, memInfo.memoryTypeIndex);

	VkResult result = vkAllocateMemory(mVKDevice->Get(), &memInfo, nullptr, &mMemory);
	CHECK(result == VK_SUCCESS);

	// Bind the buffer to its memory.
	result = vkBindBufferMemory(mVKDevice->Get(), mHandle, mMemory, 0);
	CHECK(result == VK_SUCCESS);
}


void VKIBuffer::GetMemoryInfo(VkDeviceSize& outAllocateSize, uint32_t& outTypeIndex)
{
	// Query Memory Requirements for our buffer.
	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(mVKDevice->Get(), mHandle, &memReq);

	outAllocateSize = memReq.size;
	outTypeIndex = mVKDevice->FindMemory(memReq.memoryTypeBits, mProperties);
	CHECK(outTypeIndex != VK_MAX_MEMORY_TYPES && "Failed to find memory.");

}


void VKIBuffer::Destroy()
{
	// Destroy Memory.
	vkFreeMemory(mVKDevice->Get(), mMemory, nullptr);

	// Destroy Buffer.
	vkDestroyBuffer(mVKDevice->Get(), mHandle, nullptr);

	// Destroy staging if valid?
	if (mStaging)
	{
		mStaging->Destroy();
		mStaging.reset();
	}

	//...
	mHandle = VK_NULL_HANDLE;
	mMemory = VK_NULL_HANDLE;
}


void VKIBuffer::UpdateData(const void* data)
{
	UpdateData(0, mSize, data);
}


void VKIBuffer::UpdateData(VkDeviceSize offset, VkDeviceSize size, const void* data)
{
	CHECK(mProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && "Memory must be host visible.");

	void* buffer;
	vkMapMemory(mVKDevice->Get(), mMemory, offset, size, 0, &buffer);
	memcpy(buffer, data, size);

	// Not Host Coherent?
	if ((mProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
	{
		VkMappedMemoryRange range{};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = mMemory;
		range.offset = offset;
		range.size = VK_WHOLE_SIZE;
		VkResult result = vkFlushMappedMemoryRanges(mVKDevice->Get(), 1, &range);
		CHECK(result == VK_SUCCESS);
	}

	
	// Unmap...
	vkUnmapMemory(mVKDevice->Get(), mMemory);
}


void VKIBuffer::UpdateDataStaging(const void* data)
{
	UpdateDataStaging(0, mSize, data);
}


void VKIBuffer::UpdateDataStaging(VkDeviceSize offset, VkDeviceSize size, const void* data)
{
	// New Staging?
	if (!mStaging)
	{
		mStaging = Ptr<VKIBuffer>(new VKIBuffer());
		mStaging->SetSize(size);
		mStaging->SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		mStaging->SetMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mStaging->CreateBuffer(mVKDevice);
		mStaging->UpdateData(offset, size, data);

		VkCommandBuffer cmd = mVKDevice->BeginTransientCmd();

		VkBufferCopy region{};
		region.srcOffset = 0;
		region.dstOffset = offset;
		region.size = size;
		vkCmdCopyBuffer(cmd, mStaging->Get(), mHandle, 1, &region);

		mVKDevice->EndTransientCmd(cmd, Delegate<>::CreateMemberRaw(this, &VKIBuffer::DestroyStaging));
	}
	else
	{
		mStaging->UpdateData(offset, size, data);
	}

}


void VKIBuffer::DestroyStaging()
{
	mStaging->Destroy();
	mStaging.reset();
}


void VKIBuffer::CmdUpdate(VKICommandBuffer* cmdBuffer, uint32_t offset, uint32_t size, const void* data)
{
	vkCmdUpdateBuffer(cmdBuffer->GetCurrent(), mHandle, offset, size, data);
}
