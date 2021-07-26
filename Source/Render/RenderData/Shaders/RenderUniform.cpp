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




#include "RenderUniform.h"
#include "Render/Renderer.h"
#include "Render/VKInterface/VKIBuffer.h"
#include "Render/VKInterface/VKICommandBuffer.h"




RenderUniform::RenderUniform()
	: mIsTransferDst(false)
	, mIsDynamic(false)
{

}


RenderUniform::~RenderUniform()
{

}



void RenderUniform::Create(Renderer* owner, uint32_t size, bool isDynamic)
{
	mIsDynamic = isDynamic;
	mBuffers.resize(Renderer::NUM_CONCURRENT_FRAMES);

	for (size_t i = 0; i < mBuffers.size(); ++i)
	{
		mBuffers[i] = UniquePtr<VKIBuffer>(new VKIBuffer());
		mBuffers[i]->SetSize((VkDeviceSize)size);
		mBuffers[i]->SetMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (mIsTransferDst)
		{
			mBuffers[i]->SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		}
		else
		{
			mBuffers[i]->SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}

		mBuffers[i]->CreateBuffer(owner->GetVKDevice());
	}
}


void RenderUniform::Destroy()
{
	for (size_t i = 0; i < mBuffers.size(); ++i)
	{
		mBuffers[i]->Destroy();
	}

	mBuffers.clear();
}


std::vector<VKIBuffer*> RenderUniform::GetBuffers() const
{
	std::vector<VKIBuffer*> buffers;
	buffers.resize(mBuffers.size());

	for (size_t i = 0; i < mBuffers.size(); ++i)
		buffers[i] = mBuffers[i].get();

	return buffers;
}


void RenderUniform::Update(uint32_t frame, uint32_t offset, uint32_t size, const void* data)
{
	mBuffers[frame]->UpdateData(offset, size, data);
}


void RenderUniform::Update(uint32_t frame, const void* data)
{
	mBuffers[frame]->UpdateData(data);
}


void RenderUniform::CmdUpdate(VKICommandBuffer* cmdBuffer, int32_t frame, uint32_t offset, uint32_t size, const void* data)
{
	mBuffers[frame]->CmdUpdate(cmdBuffer, offset, size, data);
}


void RenderUniform::SetTransferDst(bool value)
{
	mIsTransferDst = value;
}
