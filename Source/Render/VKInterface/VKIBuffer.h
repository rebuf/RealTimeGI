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






// VKIBuffer:
//     - Handle vulkan buffer & it's DeviceMemory.
//
class VKIBuffer
{
public:
	// Construct.
	VKIBuffer();

	// Destruct.
	~VKIBuffer();

	// Return the vulkan handle.
	inline VkBuffer Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Set Buffer Usage.
	void SetUsage(VkBufferUsageFlags usage);
	
	// Set Buffer's Memory Properties.
	void SetMemoryProperties(VkMemoryPropertyFlags properties);

	// Set the size of the buffer in bytes.
	void SetSize(VkDeviceSize size);

	// Create vulkan semaphore.
	void CreateBuffer(VKIDevice* owner);

	// Destroy vulkan Buffer & it's DeviceMemory.
	void Destroy();

	// Update buffer data by mapping the buffer memory, only used on host visible buffers.
	void UpdateData(void* data);
	void UpdateData(VkDeviceSize offset, VkDeviceSize size, void* data);

	// Update data using staging buffer, used for local device memory.
	void UpdateDataStaging(void* data);
	void UpdateDataStaging(VkDeviceSize offset, VkDeviceSize size, void* data);

	// Return the size of the buffer.
	inline VkDeviceSize GetSize() const { return mSize; }

private:
	// Allocate Device Memory for the created vulkan buffer.
	void AllocateMemory();

	// Get the memory type index that best suits the created buffer.
	void GetMemoryInfo(VkDeviceSize& outAllocateSize, uint32_t& outTypeIndex);

	// Destroy Staging after it was submited & finished.
	void DestroyStaging();

private:
	// Vulkan Buffer Handle.
	VkBuffer mHandle;

	// Buffer's device memory.
	VkDeviceMemory mMemory;

	// The device that owns this buffer.
	VKIDevice* mVKDevice;

	// The Buffer Usage.
	VkBufferUsageFlags mUsage;

	// Buffer's Memory Properties.
	VkMemoryPropertyFlags mProperties;

	// The Buffer size in bytes.
	VkDeviceSize mSize;

	// Staging Buffer.
	Ptr<VKIBuffer> mStaging;
};



