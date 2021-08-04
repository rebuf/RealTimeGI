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




#include "VKIImage.h"
#include "VKIDevice.h"
#include "VKIBuffer.h"








VKIImage::VKIImage()
	: mHandle(VK_NULL_HANDLE)
	, mMemory(VK_NULL_HANDLE)
	, mVKDevice(nullptr)
	, mImageType(VK_IMAGE_TYPE_2D)
	, mFormat(VK_FORMAT_UNDEFINED)
	, mTiling(VK_IMAGE_TILING_OPTIMAL)
	, mUsage(VK_IMAGE_USAGE_SAMPLED_BIT)
	, mLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	, mMemProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	, mLayers(1)
	, mMipLevels(1)
	, mIsCubeMap(false)
{
	mSize = { 0, 0, 0 };
}


VKIImage::~VKIImage()
{

}


void VKIImage::Create(VKIDevice* owner)
{
	mVKDevice = owner;

	// Create Image.
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.flags = 0;
  imageInfo.imageType = mImageType;
  imageInfo.format = mFormat;
  imageInfo.extent = mSize;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // No Multisampling Support.
  imageInfo.tiling = mTiling;
  imageInfo.usage = mUsage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	
  imageInfo.initialLayout = mLayout;

	// Mipmapping...
	imageInfo.mipLevels = mMipLevels;
	imageInfo.arrayLayers = mLayers;

	// CubeMap?
	if (mIsCubeMap)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkResult result = vkCreateImage(mVKDevice->Get(), &imageInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);


	// Allocate Memory...
	AllocateMemory();
}


void VKIImage::AllocateMemory()
{
	// Allocate...
	VkMemoryAllocateInfo memInfo{};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	GetMemoryInfo(memInfo.allocationSize, memInfo.memoryTypeIndex);

	VkResult result = vkAllocateMemory(mVKDevice->Get(), &memInfo, nullptr, &mMemory);
	CHECK(result == VK_SUCCESS);

	// Bind the image to its memory.
	vkBindImageMemory(mVKDevice->Get(), mHandle, mMemory, 0);
	CHECK(result == VK_SUCCESS);
}


void VKIImage::GetMemoryInfo(VkDeviceSize& outAllocateSize, uint32_t& outTypeIndex)
{
	// Query Memory Requirements for our buffer.
	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(mVKDevice->Get(), mHandle, &memReq);

	outAllocateSize = memReq.size;
	outTypeIndex = mVKDevice->FindMemory(memReq.memoryTypeBits, mMemProperties);
	CHECK(outTypeIndex != VK_MAX_MEMORY_TYPES && "Failed to find memory.");
}


void VKIImage::Destroy()
{
	// Destroy Memory.
	vkFreeMemory(mVKDevice->Get(), mMemory, nullptr);

	// Destroy Image.
	vkDestroyImage(mVKDevice->Get(), mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
	mMemory = VK_NULL_HANDLE;
}


void VKIImage::SetImageInfo(VkImageType type, VkFormat format, VkExtent2D size, VkImageLayout layout)
{
	CHECK(mHandle == VK_NULL_HANDLE);
	mImageType = type;
	mFormat = format;
	mLayout = layout;

	// Size 2D.
	mSize.width = size.width;
	mSize.height = size.height;
	mSize.depth = 1;
}


void VKIImage::SetTiling(VkImageTiling tiling)
{
	CHECK(mHandle == VK_NULL_HANDLE);
	mTiling = tiling;
}


void VKIImage::SetUsage(VkImageUsageFlags usage)
{
	CHECK(mHandle == VK_NULL_HANDLE);
	mUsage = usage;
}


void VKIImage::SetLayers(uint32_t layers, bool isCubeMap)
{
	CHECK(mHandle == VK_NULL_HANDLE);
	mLayers = layers;

	if (isCubeMap)
	{
		CHECK((mLayers % 6) == 0);
		mIsCubeMap = isCubeMap;
	}
}


void VKIImage::SetMipLevels(uint32_t levels)
{
	CHECK(mHandle == VK_NULL_HANDLE);
	mMipLevels = levels;
}


void VKIImage::TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkImageAspectFlags aspect)
{
	// Barrier...
	VkImageMemoryBarrier imgBarrier{};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.oldLayout = mLayout;
	imgBarrier.newLayout = newLayout;
	imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarrier.image = mHandle;
	imgBarrier.subresourceRange.aspectMask = aspect;
	imgBarrier.subresourceRange.baseMipLevel = 0;
	imgBarrier.subresourceRange.levelCount = mMipLevels;
	imgBarrier.subresourceRange.baseArrayLayer = 0;
	imgBarrier.subresourceRange.layerCount = mLayers;

	VkPipelineStageFlags srcStage, dstStage;
	ComputePipelineStage(newLayout, imgBarrier, srcStage, dstStage);


	// Transition...
	vkCmdPipelineBarrier(cmd, 
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &imgBarrier);

	mLayout = newLayout;
}


void VKIImage::ComputePipelineStage(VkImageLayout newLayout, VkImageMemoryBarrier& imgBarrier, 
	VkPipelineStageFlags& src, VkPipelineStageFlags& dst)
{
	// Transition from Unedefined?
	if (mLayout == VK_IMAGE_LAYOUT_UNDEFINED)
	{
		imgBarrier.srcAccessMask = 0;
		src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		// To?
		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			imgBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			imgBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;

		default:
			CHECK(0 && "Unsupported Transition.");
			break;
		}

	}
	else if (mLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		// TRANSFER_DST -> SHADER_READ_ONLY 
		if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			src = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	}
	else if (mLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		// COLOR_ATTACHMENT -> SHADER_READ_ONLY 
		if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	}
	else if (mLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		// SHADER_READ_ONLY - > COLOR_ATTACHMENT
		if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			imgBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imgBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			src = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	}
	else
	{
		CHECK(0 && "Unsupported Transition.");
	}

}


void VKIImage::UpdateImage(VkCommandBuffer cmd, VKIBuffer* buffer)
{
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = mLayers;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = mSize;

	vkCmdCopyBufferToImage(cmd,
		buffer->Get(), mHandle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region);

}





// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 




VKIImageView::VKIImageView()
	: mVKDevice(nullptr)
	, mVKImage(nullptr)
	, mHandle(VK_NULL_HANDLE)
{
	mSwizzle = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A,
	};
}


VKIImageView::~VKIImageView()
{

}


void VKIImageView::Create(VKIDevice* device, VKIImage* image)
{
	Create(device, image->Get(), image->GetFormat());
}


void VKIImageView::Create(VKIDevice* device, VkImage image, VkFormat format)
{
	mVKDevice = device;
	mVKImage = image;

	// Create View...
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.flags = 0;
	viewInfo.image = image;
	viewInfo.viewType = mType;
	viewInfo.format = format;

	viewInfo.components.r = mSwizzle[0];
	viewInfo.components.g = mSwizzle[1];
	viewInfo.components.b = mSwizzle[2];
	viewInfo.components.a = mSwizzle[3];

	viewInfo.subresourceRange.aspectMask = mAspect;
	viewInfo.subresourceRange.levelCount = mLevelCount;
	viewInfo.subresourceRange.baseMipLevel = mBaseLevel;
	viewInfo.subresourceRange.layerCount = mLayerCount;
	viewInfo.subresourceRange.baseArrayLayer = mBaseLayer;

	VkResult result = vkCreateImageView(device->Get(), &viewInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);

}


void VKIImageView::Destroy()
{
	vkDestroyImageView(mVKDevice->Get(), mHandle, nullptr);

	// ...
	mHandle = VK_NULL_HANDLE;
}


void VKIImageView::SetType(VkImageViewType type)
{
	CHECK(!IsValid());
	mType = type;
}


void VKIImageView::SetViewInfo(VkImageAspectFlagBits aspect, uint32_t baseLevel, uint32_t levelCount,
	uint32_t baseLayer, uint32_t layerCount)
{
	CHECK(!IsValid());

	mAspect = aspect;
	mBaseLevel = baseLevel;
	mLevelCount = levelCount;
	mBaseLayer = baseLayer;
	mLayerCount = layerCount;
}





// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 





VKISampler::VKISampler()
	: mHandle(VK_NULL_HANDLE)
	, mVKDevice(nullptr)
	, mMinFilter(VK_FILTER_NEAREST)
	, mMagFilter(VK_FILTER_NEAREST)
	, mAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
	, mMipmap(VK_SAMPLER_MIPMAP_MODE_NEAREST)
	, mMipLodBias(0.0f)
	, mMinLod(0.0f)
	, mMaxLod(0.0f)
	, mIsCompare(false)
	, mCompareOp(VK_COMPARE_OP_ALWAYS)
{

}


VKISampler::~VKISampler()
{

}


void VKISampler::CreateSampler(VKIDevice* owner)
{
	mVKDevice = owner;

	// Create Sampler...
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = mMagFilter;
	samplerInfo.minFilter = mMinFilter;

	samplerInfo.addressModeU = mAddressMode;
	samplerInfo.addressModeV = mAddressMode;
	samplerInfo.addressModeW = mAddressMode;
	samplerInfo.borderColor = mBorder;

	samplerInfo.mipmapMode = mMipmap;
	samplerInfo.mipLodBias = mMipLodBias;
	samplerInfo.minLod = mMinLod;
	samplerInfo.maxLod = mMaxLod;

	samplerInfo.compareEnable = mIsCompare ? VK_TRUE : VK_FALSE;
	samplerInfo.compareOp = mCompareOp;


	VkResult result = vkCreateSampler(mVKDevice->Get(), &samplerInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);

}


void VKISampler::Destroy()
{
	vkDestroySampler(mVKDevice->Get(), mHandle, nullptr);

	//...
	mHandle = VK_NULL_HANDLE;
}


void VKISampler::SetFilter(VkFilter min, VkFilter mag)
{
	CHECK(!IsValid());
	mMinFilter = min;
	mMagFilter = mag;
}


void VKISampler::SetMipmap(VkSamplerMipmapMode mipmap, float min, float max, float bias)
{
	CHECK(!IsValid());
	mMipmap = mipmap;
	mMinLod = min;
	mMaxLod = max;
	mMipLodBias = bias;
}


void VKISampler::SetAddressMode(VkSamplerAddressMode mode)
{
	CHECK(!IsValid());
	mAddressMode = mode;
}


void VKISampler::SetCompare(bool enable, VkCompareOp op)
{
	CHECK(!IsValid());
	mIsCompare = enable;
	mCompareOp = op;
}
