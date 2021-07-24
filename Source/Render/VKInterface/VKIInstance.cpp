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





#include "VKIInstance.h"
#include "Application.h"
#include "AppWindow.h"

#include "GLFW/glfw3.h"







#if USE_VULKAN_VALIDATION_LAYER

// Vector of all validation layer we want to enable.
static const std::vector<const char*> gValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
};


// a handle for managing the debug callback function with vulkan.
static VkDebugUtilsMessengerEXT gDebugMessenger = VK_NULL_HANDLE;


// Debug callback from vulkan validation layer debug utils.
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData )
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		LOGW("Validation Layer: %s\n-------------\n", pCallbackData->pMessage);
	}
	else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		LOGE("Validation Layer: %s\n-------------\n", pCallbackData->pMessage);
	}

	return VK_FALSE;
}


// Create the debug messenger after getting the function
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	// Get the function from vulkan.
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	// if exist call it
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}


// Destroy the debug messenger after getting the function
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

#endif






// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 
// - --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- - 







VKIInstance::VKIInstance()
	: mHandle(VK_NULL_HANDLE)
	, mWndContext(nullptr)
	, mPhysicalDevice(VK_NULL_HANDLE)
{

#if USE_VULKAN_VALIDATION_LAYER
	isValidationLayer = false;
#endif

	mSurface.handle = VK_NULL_HANDLE;
	mSurface.capabilities = VkSurfaceCapabilitiesKHR{};
}


VKIInstance::~VKIInstance()
{
	CHECK(!IsValid() && "VKIInstance::Destroy() should be called before destructor.");

}


void VKIInstance::Destroy()
{
	if (mSurface.handle != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(mHandle, mSurface.handle, nullptr);
	}

#if USE_VULKAN_VALIDATION_LAYER
	// Destroy the debug messenger.
	DestroyDebugUtilsMessengerEXT(mHandle, gDebugMessenger, nullptr);
#endif

	// Destroy Vulkan Instance.
	vkDestroyInstance(mHandle, nullptr);


	//...
	mHandle = VK_NULL_HANDLE;
	mSurface.handle = VK_NULL_HANDLE;

}


void VKIInstance::CreateInstance()
{
	// Application info for vulkan instance creation.
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = Application::Get().GetAppName().c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "NOT_ENGINE";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;



	// Requird Instance Extensions...
	std::vector<const char*> reqExt;

	// Window Surface Ext...
	{
		uint32_t count = 0, s = 0;
		auto wndReqExt = glfwGetRequiredInstanceExtensions(&count);

		while (s < count)
		{
			reqExt.push_back(*(wndReqExt + s));
			++s;
		}
	}


#if USE_VULKAN_VALIDATION_LAYER
	// Is Validation Layer Supported?
	isValidationLayer = CheckValidationLayerSupport();

	// Enable Debug Utils Ext only when validation layer supported
	if (isValidationLayer)
	{
		// Add debug utils to the required extensions to enalbe it.
		reqExt.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
#endif




	// Info used for vulkan instance creation.
	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = (uint32_t)reqExt.size();
	instanceInfo.ppEnabledExtensionNames = reqExt.data();

#if USE_VULKAN_VALIDATION_LAYER
	// Validation Layer Supported?
	if (isValidationLayer)
	{
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(gValidationLayers.size());
		instanceInfo.ppEnabledLayerNames = gValidationLayers.data();
	}
	else
	{
		instanceInfo.enabledLayerCount = 0;
		instanceInfo.ppEnabledLayerNames = 0;
	}
#else
	// No Validation Layers
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = 0;
#endif


	// Create Vulkan Instance...
	VkResult vresult = vkCreateInstance(&instanceInfo, nullptr, &mHandle);
	CHECK(vresult == VK_SUCCESS);



#if USE_VULKAN_VALIDATION_LAYER
	// Setup debug messenger for validation callbacks.
	SetupDebugMessenger();
#endif

}


#if USE_VULKAN_VALIDATION_LAYER

bool VKIInstance::CheckValidationLayerSupport()
{
	// Get the number of layers supported.
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Get the layers supported.
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Find if the required validation layers are supported.
	for (const char* layerName : gValidationLayers)
	{
		bool layerFound = false;

		// Iterate over all supported layers and check if the ones we want exist.
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}


void VKIInstance::SetupDebugMessenger()
{
	// Structure for creating a debug messenger
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	// Setup Type of messages we want to receive we want WARNING & ERROR...
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = &VulkanDebugCallback;

	// Create The Messenger
	if (CreateDebugUtilsMessengerEXT(mHandle, &createInfo, nullptr, &gDebugMessenger) != VK_SUCCESS)
	{
		printf("Failed to set up debug messenger!");
	}
}

#endif



void VKIInstance::CreateSurface(AppWindow* wnd)
{
	VkResult result = glfwCreateWindowSurface(mHandle, wnd->GetHandle(), nullptr, &mSurface.handle);
	CHECK(result == VK_SUCCESS);
}


void VKIInstance::FindQueueFamilies(VkPhysicalDevice physicalDevice, VKIQueueFamiles& outFamilies)
{
	// Get Queue Families...
	uint32_t familiesCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familiesCount, nullptr);

	std::vector<VkQueueFamilyProperties> familiesProperties(familiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familiesCount, familiesProperties.data());

	// Iterate over the queues and find the ones that we need.
	for (uint32_t i = 0; i < familiesCount; ++i)
	{
		// Check if this queue support graphics commands
		if (familiesProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			outFamilies.graphics = i;
		}

		// Check if this queue family support presenting
		VkBool32 supportPresent = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, mSurface.handle, &supportPresent);

		if (supportPresent)
		{
			outFamilies.present = i;
		}

		// Check for completion
		if (outFamilies.IsComplete())
		{
			// all the queues that we need found.
			break;
		}
	}

}


void VKIInstance::QuerySwapChainSupport(VkPhysicalDevice physicalDevice)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mSurface.handle, &mSurface.capabilities);

	// Query the formats supported by the device for the swapchain.
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface.handle, &formatCount, nullptr);

	mSurface.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mSurface.handle,
		&formatCount, mSurface.formats.data());

	// Query the number of presentation modes supported by the device for the swapchain.
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface.handle, &presentModeCount, nullptr);

	mSurface.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mSurface.handle,
		&presentModeCount, mSurface.presentModes.data());
}



void VKIInstance::PickPhysicalDevice()
{
	// Get Physical Devices...
	uint32_t devicesCount = 0;
	vkEnumeratePhysicalDevices(mHandle, &devicesCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices(devicesCount);
	vkEnumeratePhysicalDevices(mHandle, &devicesCount, physicalDevices.data());


	// Iterate over all devices and find the first suitable one for our rendering.
	for (uint32_t i = 0; i < physicalDevices.size(); ++i)
	{
		// Check device queues support...
		VKIQueueFamiles deviceQueues;
		FindQueueFamilies(physicalDevices[i], deviceQueues);

		if (!deviceQueues.IsComplete())
			continue;

		// Check device swapchain support...
		QuerySwapChainSupport(physicalDevices[i]);

		if (!mSurface.CheckSupport())
			continue;

		// Found...
		mPhysicalDevice = physicalDevices[i];
		mQueues = deviceQueues;
		break;
	}

	// No Suitable device found?
	CHECK(mPhysicalDevice != VK_NULL_HANDLE && "No suitable vulkan physical device found!");
}


VkExtent2D VKIInstance::GetFramebufferSize()
{
	int width, height;
	glfwGetFramebufferSize(mWndContext->GetHandle(), &width, &height);

	VkExtent2D fboSize;
	fboSize.width = (uint32_t)width;
	fboSize.height = (uint32_t)height;
	return fboSize;
}
