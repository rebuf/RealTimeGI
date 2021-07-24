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





// VKICommandBuffer:
//    - Handle vulkan command buffers.
//
class VKICommandBuffer
{
public:
	// Construct.
	VKICommandBuffer();

	// Destruct.
	~VKICommandBuffer();

	// Return the vulkan handle.
	inline VkCommandBuffer Get(uint32_t idx = 0) const { return mCmdBuffers[idx]; }

	// Return the if the vulkan command buffer is valid.
	inline bool IsValid() const { return !mCmdBuffers.empty() && mCmdBuffers[0] != VK_NULL_HANDLE; }

	// Destroy Vulkan Command Buffers.
	void Destroy();

	// Create/Allocate Command Buffers.
	void CreateCmdBuffer(VKIDevice* owner, VkCommandPool pool, uint32_t count);

	// Set current command buffer used by the current frame.
	inline void SetCurrent(uint32_t index) { mCurrentIndex = index; }

	// Return the current vulkan command buffer.
	inline VkCommandBuffer GetCurrent() const { return mCmdBuffers[mCurrentIndex]; };

private:
	// Command Buffer for each frame in flight.
	std::vector<VkCommandBuffer> mCmdBuffers;

	// The device that owns this command buffer.
	VKIDevice* mVKDevice;

	// The Command Pool that allocated this command buffer.
	VkCommandPool mCmdPool;
	
	// The index of the current command buffer used by the current frame.
	uint32_t mCurrentIndex;
};


