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





#include "VKIDescriptor.h"
#include "VKIDevice.h"
#include "VKIBuffer.h"
#include "VKIImage.h"
#include "VKICommandBuffer.h"
#include "VKIGraphicsPipeline.h"









VKIDescriptorLayout::VKIDescriptorLayout()
	: mHandle(VK_NULL_HANDLE)
	, mVKDevice(nullptr)
{

}


VKIDescriptorLayout::~VKIDescriptorLayout()
{

}


void VKIDescriptorLayout::CreateLayout(VKIDevice* owner)
{
	mVKDevice = owner;

	// Create Layout...
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = (uint32_t)mBindings.size();
	layoutInfo.pBindings = mBindings.data();


	VkResult result = vkCreateDescriptorSetLayout(mVKDevice->Get(), &layoutInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);

}


void VKIDescriptorLayout::Destroy()
{
	vkDestroyDescriptorSetLayout(mVKDevice->Get(), mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
}


void VKIDescriptorLayout::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages)
{
	AddBinding(binding, type, stages, 1);
}


void VKIDescriptorLayout::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages, uint32_t count)
{
	VkDescriptorSetLayoutBinding newBinding{};
	newBinding.binding = binding;
	newBinding.descriptorType = type;
	newBinding.descriptorCount = count;
	newBinding.stageFlags = stages;
	newBinding.pImmutableSamplers = nullptr;

	mBindings.emplace_back(newBinding);
	IncTypeCount(type);
}


void VKIDescriptorLayout::IncTypeCount(VkDescriptorType type)
{
	auto iter = mTypeCount.find(type);

	// Exist?
	if (iter != mTypeCount.end())
	{
		(*iter).second += 1;
	}
	else
	{
		mTypeCount[type] = 1;
	}
}



void VKIDescriptorLayout::ClearBindings()
{
	mBindings.clear();
	mTypeCount.clear();
}


void VKIDescriptorLayout::Recreate()
{
	Destroy();
	CreateLayout(mVKDevice);
}





// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 





VKIDescriptorSet::VKIDescriptorSet()
	: mLayout(nullptr)
	, mVKDevice(nullptr)
{

}


VKIDescriptorSet::~VKIDescriptorSet()
{
	CHECK(mPool == VK_NULL_HANDLE);
}


void VKIDescriptorSet::CreateDescriptorPool(uint32_t count)
{
	// Pool Size for each descriptor type in layout.
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (const auto& typeCount : mLayout->GetTypeCount())
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = typeCount.first;
		poolSize.descriptorCount = typeCount.second * count;
		poolSizes.emplace_back(poolSize);
	}


	// Create Pool
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = count;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

	VkResult result = vkCreateDescriptorPool(mVKDevice->Get(), &poolInfo, nullptr, &mPool);
	CHECK(result == VK_SUCCESS);
}


void VKIDescriptorSet::CreateDescriptorSet(VKIDevice* owner, uint32_t count)
{
	mVKDevice = owner;
	mHandles.resize(count);

	// Pool
	CreateDescriptorPool(count);


	// Allocate Sets.
	VkDescriptorSetLayout vklayout = mLayout->Get();
	std::vector<VkDescriptorSetLayout> layouts(count, vklayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = mPool;
	descriptorSetAllocInfo.descriptorSetCount = count;
	descriptorSetAllocInfo.pSetLayouts = layouts.data();

	vkAllocateDescriptorSets(mVKDevice->Get(), &descriptorSetAllocInfo, mHandles.data());

}


void VKIDescriptorSet::Destroy()
{
	// Free Sets.
	vkResetDescriptorPool(mVKDevice->Get(), mPool, 0);

	// Destroy Pool.
	vkDestroyDescriptorPool(mVKDevice->Get(), mPool, nullptr);

	//...
	mHandles.clear();
	mPool = VK_NULL_HANDLE;
}


void VKIDescriptorSet::UpdateSets()
{
	const uint32_t setCount = (uint32_t)mHandles.size();

	// Infos used to update the writers.
	std::vector< Ptr<VkDescriptorBufferInfo> > bufferInfos;
	std::vector< Ptr<VkDescriptorImageInfo> > imageInfos;

	std::vector<VkWriteDescriptorSet> writers;
	writers.resize(mDescriptors.size() * setCount);

	for (uint32_t i = 0; i < setCount; ++i)
	{
		for (size_t r = 0; r < mDescriptors.size(); ++r)
		{
			VkWriteDescriptorSet& writer = writers[r + i * mDescriptors.size()];
			writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writer.dstSet = mHandles[i];
			writer.dstBinding = mDescriptors[r].binding;
			writer.dstArrayElement = 0;
			writer.descriptorCount = 1;
			writer.descriptorType = mDescriptors[r].type;

			// Descriptor Info based on type...
			switch (mDescriptors[r].type)
			{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			{
				VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();
				bufferInfo->buffer = mDescriptors[r].buffer[i]->Get();
				bufferInfo->offset = mDescriptors[r].bufferOffset;
				bufferInfo->range = mDescriptors[r].bufferSize;
				bufferInfos.emplace_back(bufferInfo);

				writer.pBufferInfo = bufferInfo;
			}
				break;


			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			{
				VkDescriptorImageInfo* imgInfo = new VkDescriptorImageInfo();
				imgInfo->imageView = mDescriptors[r].imageView->Get();
				imgInfo->sampler = mDescriptors[r].sampler->Get();
				imgInfo->imageLayout = mDescriptors[r].imgLayout;
				imageInfos.emplace_back(imgInfo);

				writer.pImageInfo = imgInfo;
			}
				break;

			}
		}

	}


	// Update Sets...
	vkUpdateDescriptorSets(mVKDevice->Get(), (uint32_t)writers.size(), writers.data(), 0, nullptr);

}


void VKIDescriptorSet::UpdateDescriptorSet(uint32_t index, uint32_t binding)
{
	// Infos used to update the writers.
	Ptr<VkDescriptorBufferInfo> bufferInfo;
	Ptr<VkDescriptorImageInfo> imgInfo;

	VkWriteDescriptorSet writer{};
	writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writer.dstSet = mHandles[index];
	writer.dstBinding = mDescriptors[binding].binding;
	writer.dstArrayElement = 0;
	writer.descriptorCount = 1;
	writer.descriptorType = mDescriptors[index].type;
	
	// Descriptor Info based on type...
	switch (mDescriptors[index].type)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
	{
		bufferInfo = Ptr<VkDescriptorBufferInfo>(new VkDescriptorBufferInfo());
		bufferInfo->buffer = mDescriptors[binding].buffer[index]->Get();
		bufferInfo->offset = mDescriptors[binding].bufferOffset;
		bufferInfo->range = mDescriptors[binding].bufferSize;
	
		writer.pBufferInfo = bufferInfo.get();
	}
		break;
	
	
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	{
		imgInfo = Ptr<VkDescriptorImageInfo>(new VkDescriptorImageInfo());
		imgInfo->imageView = mDescriptors[binding].imageView->Get();
		imgInfo->sampler = mDescriptors[binding].sampler->Get();
		imgInfo->imageLayout = mDescriptors[binding].imgLayout;
	
		writer.pImageInfo = imgInfo.get();
	}
		break;
	
	}


	// Update Sets...
	vkUpdateDescriptorSets(mVKDevice->Get(), 1, &writer, 0, nullptr);

}


void VKIDescriptorSet::ClearDescriptor()
{
	mDescriptors.clear();
}


void VKIDescriptorSet::AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages,
	const std::vector<VKIBuffer*>& buffers)
{
	AddDescriptor(binding, type, stages, buffers, 0, buffers[0]->GetSize(), 1);
}


void VKIDescriptorSet::AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages,
	const std::vector<VKIBuffer*>& buffers, uint32_t count)
{
	AddDescriptor(binding, type, stages, buffers, 0, buffers[0]->GetSize(), count);
}


void VKIDescriptorSet::AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages,
	const std::vector<VKIBuffer*>& buffers, VkDeviceSize offset, VkDeviceSize size)
{
	AddDescriptor(binding, type, stages, buffers, offset, size, 1);
}


void VKIDescriptorSet::AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages,
	const std::vector<VKIBuffer*>& buffers, VkDeviceSize offset, VkDeviceSize size, uint32_t count)
{
	VKIDescriptor descriptor{};
	descriptor.type = type;
	descriptor.binding = binding;
	descriptor.stages = stages;
	descriptor.count = count;
	descriptor.buffer = buffers;
	descriptor.bufferOffset = offset;
	descriptor.bufferSize = size;

	mDescriptors.emplace_back(descriptor);
}


void VKIDescriptorSet::AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages,
	VKIImageView* view, VKISampler* sampler)
{
	AddDescriptor(binding, type, stages, view, sampler, 1);
}


void VKIDescriptorSet::AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages,
	VKIImageView* view, VKISampler* sampler, uint32_t count)
{
	VKIDescriptor descriptor{};
	descriptor.type = type;
	descriptor.binding = binding;
	descriptor.stages = stages;
	descriptor.count = count;
	descriptor.imageView = view;
	descriptor.sampler = sampler;
	descriptor.imgLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	mDescriptors.emplace_back(descriptor);
}


void VKIDescriptorSet::Bind(VKICommandBuffer* cmdBuffer, uint32_t index, const VKIGraphicsPipeline* pipeline) const
{
	std::vector<uint32_t> dynamicOffsets;
	Bind(cmdBuffer, index, pipeline, dynamicOffsets);
}


void VKIDescriptorSet::Bind(VKICommandBuffer* cmdBuffer, uint32_t index, const VKIGraphicsPipeline* pipeline,
	const std::vector<uint32_t>& dynamicOffsets) const
{
	vkCmdBindDescriptorSets(cmdBuffer->GetCurrent(),
		VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(),
		0, 1, &mHandles[index],
		(uint32_t)dynamicOffsets.size(), dynamicOffsets.data());

}
