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
#include <map>




class VKIDevice;
class VKISampler;
class VKIImageView;
class VKIBuffer;
class VKICommandBuffer;
class VKIGraphicsPipeline;







// VKIDescriptorLayout:
//     - Handle vulkan Descriptor Set Layout.
//
class VKIDescriptorLayout
{
public:
	// Construct
	VKIDescriptorLayout();

	// Destruct.
	~VKIDescriptorLayout();

	// Return the vulkan handle.
	inline VkDescriptorSetLayout Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Create vulkan descriptor set layout.
	void CreateLayout(VKIDevice* owner);

	// Destroy vulkan descriptor set layout.
	void Destroy();

	// Add new binding to the layout.
	void AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages);
	void AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages, uint32_t count);

	// Clear all the bindings.
	void ClearBindings();

	// Recreate the decriptor layout.
	void Recreate();

	// Return all the bindings in this layout.
	inline const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const { return mBindings; }

	// Return the number of descriptors per type for all added descriptors.
	inline const std::map<VkDescriptorType, uint32_t>& GetTypeCount() const { return mTypeCount; }

private:
	// Increment Type Count for a descriptor type.
	void IncTypeCount(VkDescriptorType type);

private:
	// Vulkan Layout Handle.
	VkDescriptorSetLayout mHandle;

	// The device that owns this layout.
	VKIDevice* mVKDevice;

	// Layout Bindings that describe the descriptor layout.
	std::vector<VkDescriptorSetLayoutBinding> mBindings;

	// Count the number of descriptors of a single type.
	std::map<VkDescriptorType, uint32_t> mTypeCount;
};






// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 





// Vulkan Descriptor of different types.
struct VKIDescriptor
{
	// The binding of the descriptor.
	uint32_t binding;

	// Number of elemnts if the descriptor is an array.
	uint32_t count;

	// The shader stages that uses this descriptor.
	VkShaderStageFlags stages;

	// The type of the Descriptor.
	VkDescriptorType type;

	// The buffer used for uniform descriptor one for each set.
	std::vector<VKIBuffer*> buffer;

	// Offset from the start of the buffer to be used by the uniform.
	VkDeviceSize bufferOffset;

	// The size of the buffer to use by the uniform.
	VkDeviceSize bufferSize;

	// The image view used by the sampler descriptor.
	VKIImageView* imageView;

	// The image sampler used by the sampler descriptor.
	VKISampler* sampler;

	// The image layout used by the sampler descriptor.
	VkImageLayout imgLayout;
};







// VKIDescriptorSet:
//     - Handle vulkan Descriptor Set.
//
class VKIDescriptorSet
{
public:
	// Construct.
	VKIDescriptorSet();

	// Destruct.
	~VKIDescriptorSet();

	// Return the vulkan handle.
	inline const VkDescriptorSet& Get(uint32_t index) const { return mHandles[index]; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return !mHandles.empty() && mHandles[0] != VK_NULL_HANDLE; }

	// Set he Descriptor Layout.
	inline void SetLayout(VKIDescriptorLayout* layout) { mLayout = layout; }

	// Return The Descriptor Layout.
	inline VKIDescriptorLayout* GetLayout() const { return mLayout; }

	// Create vulkan Descriptor Set.
	void CreateDescriptorSet(VKIDevice* owner, uint32_t count);

	// Destroy vulkan Descriptor Set.
	void Destroy();

	// Update DescriptorSet with all added descriptors.
	void UpdateSets();

	// Update a DescriptorSet of index with a descriptor at binding.
	void UpdateDescriptorSet(uint32_t index, uint32_t binding);

	// Clear all descriptors from this set.
	void ClearDescriptor();

	// Add Buffer descriptor to be used by this set.
	void AddDescriptor(uint32_t binding, VkDescriptorType type,  VkShaderStageFlags stages, const std::vector<VKIBuffer*>& buffers);
	void AddDescriptor(uint32_t binding, VkDescriptorType type,  VkShaderStageFlags stages, const std::vector<VKIBuffer*>& buffers, uint32_t count);
	void AddDescriptor(uint32_t binding, VkDescriptorType type,  VkShaderStageFlags stages, const std::vector<VKIBuffer*>& buffers, VkDeviceSize offset, VkDeviceSize size);
	void AddDescriptor(uint32_t binding, VkDescriptorType type,  VkShaderStageFlags stages, const std::vector<VKIBuffer*>& buffers, VkDeviceSize offset, VkDeviceSize size, uint32_t count);

	// Add Image descriptor to be used by this set.
	void AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages, VKIImageView* view, VKISampler* sampler);
	void AddDescriptor(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stages, VKIImageView* view, VKISampler* sampler, uint32_t count);

	// Bind this Descriptor Set with a graphics pipeline.
	void Bind(VKICommandBuffer* cmdBuffer, uint32_t index, const VKIGraphicsPipeline* pipeline) const;
	void Bind(VKICommandBuffer* cmdBuffer, uint32_t index, const VKIGraphicsPipeline* pipeline, const std::vector<uint32_t>& dynamicOffsets) const;


private:
	// Create a Descriptor Pool used to allocate Descriptor Set.
	void CreateDescriptorPool(uint32_t count);

private:
	// Vulkan Descriptor Set Handle.
	std::vector<VkDescriptorSet> mHandles;

	// The device that owns this Descriptor.
	VKIDevice* mVKDevice;

	// The Descriptor Layout.
	VKIDescriptorLayout* mLayout;

	// The Descriptor Pool used to allocate descriptor sets.
	VkDescriptorPool mPool;

	// The Descriptors.
	std::vector<VKIDescriptor> mDescriptors;
};
