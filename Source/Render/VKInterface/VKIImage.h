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

#include <array>



class VKIDevice;
class VKIImage;














// VKIImage:
//     - Handle vulkan image & it's DeviceMemory.
//
class VKIImage
{
public:
	// Construct.
	VKIImage();

	// Destruct.
	~VKIImage();

	// Return the vulkan handle.
	inline VkImage Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Create vulkan image.
	void Create(VKIDevice* owner);

	// Destroy vulkan image.
	void Destroy();

	// Set image Info used for creation.
	void SetImageInfo(VkImageType type, VkFormat format, VkExtent2D size, VkImageLayout layout);

	// Set the image tiling.
	void SetTiling(VkImageTiling tiling);

	// Set the number of layers in this image.
	void SetLayers(uint32_t layers, bool isCubeMap);

	// Set the image usage.
	void SetUsage(VkImageUsageFlags usage);

	// Set the number of mipmap levels.
	void SetMipLevels(uint32_t levels);

	// Return the image format.
	inline VkFormat GetFormat() const { return mFormat; }

	// The Image Layout.
	inline VkImageLayout GetLayout() const { return mLayout; }

	// Transition the image layout to a new one.
	void TransitionImageLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkImageAspectFlags aspect);

private:
	// Allocate Device Memory for the created vulkan image.
	void AllocateMemory();

	// Get the memory type index that best suits the created image.
	void GetMemoryInfo(VkDeviceSize& outAllocateSize, uint32_t& outTypeIndex);

	// Compupte the pipeline stages used in memory barrier when doing layout transition.
	void ComputePipelineStage(VkImageLayout newLayout, VkImageMemoryBarrier& imgBarrier, VkPipelineStageFlags& src, VkPipelineStageFlags& dst);

private:
	// Vulkan Image Handle.
	VkImage mHandle;

	// Image device memory.
	VkDeviceMemory mMemory;

	// The device that owns this image.
	VKIDevice* mVKDevice;

	// Image Type.
	VkImageType mImageType;

	// Image Format
	VkFormat mFormat;

	// Image Size.
	VkExtent3D mSize;

	// Image tiling.
	VkImageTiling mTiling;

	// Image usage.
	VkImageUsageFlags mUsage;

	// Image layout.
	VkImageLayout mLayout;

	// Image Memory Properties.
	VkMemoryPropertyFlags mMemProperties;

	// Number of layers in this image.
	uint32_t mLayers;

	// Number of mipmap levels in this image.
	uint32_t mMipLevels;

	// Flag used if the image has 6 layers and a cube map.
	bool mIsCubeMap;
};





// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 







// VKIImage:
//     - Handle vulkan image view.
//
class VKIImageView
{
public:
	// Construct.
	VKIImageView();

	// Destruct.
	~VKIImageView();

	// Return the vulkan handle.
	inline VkImageView Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Create vulkan image view.
	void Create(VKIDevice* device, VKIImage* image);
	void Create(VKIDevice* device, VkImage image, VkFormat format);

	// Destroy vulkan image view.
	void Destroy();

	// Set image view info for creation.
	void SetViewInfo(VkImageAspectFlagBits aspect, uint32_t baseLevel, uint32_t levelCount,
		uint32_t baseLayer, uint32_t layerCount);

	// Set the image view type.
	void SetType(VkImageViewType type);

private:
	// Vulkan Image View Handle.
	VkImageView mHandle;

	// The image that owns this image view.
	VkImage mVKImage;

	// The device that owns this image.
	VKIDevice* mVKDevice;

	// Image View Color Swizzle.
	std::array<VkComponentSwizzle, 4> mSwizzle;

	// Image View Type.
	VkImageViewType mType;

	// Image Aspect Flag.
	VkImageAspectFlagBits mAspect;

	// Image View Range Level Count.
	uint32_t mLevelCount;

	// Image View Range Base Level.
	uint32_t mBaseLevel; 

	// Image View Range Layer Count.
	uint32_t mLayerCount; 

	// Image View Range Base Layer.
	uint32_t mBaseLayer;
};








// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 







// VKISampler:
//     - Handle vulkan sampler.
//
class VKISampler
{
public:
	// Construct.
	VKISampler();

	// Destruct.
	~VKISampler();

	// Return the vulkan handle.
	inline VkSampler Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Create sampler.
	void CreateSampler(VKIDevice* owner);

	// Destroy vulkan image Sampler.
	void Destroy();

	// Set Min/Mag filter.
	void SetFilter(VkFilter min, VkFilter mag);

	// Set Mipmaping
	void SetMipmap(VkSamplerMipmapMode mipmap, float min, float max, float bias);

	// Set Sampler Address Mode.
	void SetAddressMode(VkSamplerAddressMode mode);

	// Compare mode used in Percentage-Closer Filtering.
	void SetCompare(bool enable, VkCompareOp op);

private:
	// Vulkan Image Sampler Handle.
	VkSampler mHandle;

	// The device that owns this image.
	VKIDevice* mVKDevice;

	// Sampler Minification Fitler
	VkFilter mMinFilter;

	// Sampler Magnification Filter
	VkFilter mMagFilter;

	// Sampler Address Mode
	VkSamplerAddressMode mAddressMode;

	// Border color used when address mode is clamp to border.
	VkBorderColor mBorder;

	// Sampler Mipmap Mode.
	VkSamplerMipmapMode mMipmap;

	// Sampler Mipmap bias.
	float mMipLodBias;

	// Sampler Mipmap min level.
	float mMinLod;

	// Sampler Mipmap max level.
	float mMaxLod;

	// Enable compare operation, PCF.
	bool mIsCompare;

	// The compare operation used while sampling, PCF.
	VkCompareOp mCompareOp;
};
