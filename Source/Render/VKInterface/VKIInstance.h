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



// Enable/Disable Vulkan Validation.
#define USE_VULKAN_VALIDATION_LAYER 1





class AppWindow;




// Struct for holding the queue families supported by the physical device.
struct VKIQueueFamiles
{
	// Index to the graphics queue. this queue for graphics command. 
	uint32_t graphics;

	// Index of a queue that can be used for present.
	uint32_t present;

	// Construct.
	VKIQueueFamiles()
		: graphics(INVALID_UINDEX)
		, present(INVALID_UINDEX)
	{

	}

	// Return true if it include all the required queues.
	inline bool IsComplete() const
	{
		return graphics != INVALID_UINDEX && present != INVALID_UINDEX;
	}

	// Return true if present queue is different than the graphics queue.
	inline bool IsPresentUnique() const
	{
		return present != graphics;
	}

	// Return a lost of all the unique queues.
	inline std::vector<uint32_t> GetUniqueQueues() const
	{
		std::vector<uint32_t> uniqueQueues;
		uniqueQueues.push_back(graphics);

		if (IsPresentUnique())
		{
			uniqueQueues.push_back(present);
		}

		return uniqueQueues;
	}
};



// Vulkan Surface & its Properties.
struct VKISurface
{
	// The window Surface we display our render on.
	VkSurfaceKHR handle;

	// Surface Capabilities
	VkSurfaceCapabilitiesKHR capabilities;

	// Surface supported formats.
	std::vector<VkSurfaceFormatKHR> formats;

	// Surface supported present modes.
	std::vector<VkPresentModeKHR> presentModes;

	// Return true if the surface properties support our needs.
	inline bool CheckSupport()
	{
		return !formats.empty() && !presentModes.empty();
	}
};





// VKIInstance:
//    - Mainly manage vulkan instance, but also try to manage anything related directly
//      to the instance, like SurfaceKHR and PhysicalDevice.
//
class VKIInstance
{
public:
	// Construct.
	VKIInstance();

	// Destruct.
	~VKIInstance();

	// Return the vulkan handle.
	inline VkInstance Get() const { return mHandle; }

	// Return true if the vulkan handle is valid.
	inline bool IsValid() const { return mHandle != VK_NULL_HANDLE; }

	// Return the vulkan surface handle.
	inline const VKISurface& GetSurface() const { return mSurface; }

	// Return the selected physical device by this instance.
	inline const VkPhysicalDevice GetPhysicalDevice() const { return mPhysicalDevice; }

	// Return the queue families in the physical device.
	inline const VKIQueueFamiles& GetQueues() const { return mQueues; }

	// Create The Vulkan Instance.
	void CreateInstance();

	// Create window surface to be used for rendering.
	void CreateSurface(AppWindow* wnd);

	// Pick a vulkan physical device to be used.
	void PickPhysicalDevice();

	// Destroy Instance.
	void Destroy();

	// Return the current framebuffer size aquired from our windowing system.
	VkExtent2D GetFramebufferSize();

	// Update/Query surface information from the current physical device.
	void ReQuerySurface();

private:
#if USE_VULKAN_VALIDATION_LAYER
	// Chech if validation layer supported by the vulkan driver.
	bool CheckValidationLayerSupport();

	// Setup debug messenger for vulkan validation callbacks.
	void SetupDebugMessenger();
#endif

	// Find the required queue families supported by the physical device.
	void FindQueueFamilies(VkPhysicalDevice physicalDevice, VKIQueueFamiles& outFamilies);

	// Check if swap chain is supported by this physical device.
	void QuerySwapChainSupport(VkPhysicalDevice physicalDevice);

private:
	// Vulkan Instance Handle.
	VkInstance mHandle;

	// The window used for creating the surface.
	AppWindow* mWndContext;

	// The selected physical device.
	VkPhysicalDevice mPhysicalDevice;

	// The surface & its information.
	VKISurface mSurface;

#if USE_VULKAN_VALIDATION_LAYER
	// True if validation layer is enabled
	bool isValidationLayer;
#endif

	// Physical Device queues.
	VKIQueueFamiles mQueues;

};

