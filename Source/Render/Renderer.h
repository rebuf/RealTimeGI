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

#include <vector>



class Scene;
class RendererPipeline;
class RenderShader;
class RenderUniform;
class RenderScene;
class RenderSphere;
class RenderImGUI;
class Image2D;

class VKIInstance;
class VKIDevice;
class VKISwapChain;
class VKISemaphore;
class VKIFence;
class VKIDescriptorSet;
class VKIBuffer;









// Vulkan sync objects for each concurrent frame.
struct VKFrameSync
{
	// Semaphore that signal next swapchain image is available.
	Ptr<VKISemaphore> smImage;

	// Semaphore that singal rendering completion. 
	Ptr<VKISemaphore> smRender;

	// Fence for frame completion.
	Ptr<VKIFence> fnFrame;
};





// Vulakn Render Data:
//     - vulkan data used by the renderer.
//
struct VKRenderData
{
	// Vulkan Instance.
	UniquePtr<VKIInstance> instance;

	// Vulkan Logical Device.
	UniquePtr<VKIDevice> device;

	// Vulkan Swap Chain.
	UniquePtr<VKISwapChain> swapchain;

	// Sync objects for each concurrent frame.
	std::vector<VKFrameSync> frameSync;
};







// Renderer:
//   - The Application Renderer, uses Vulkan API for rendering.
//
class Renderer
{
public:
	// Construct.
	Renderer();

	// Destruct.
	~Renderer();

	// Initialize the renderer.
	void Initialize();

	// Destroy the renderer.
	void Destroy();

	// Begin Rendering.
	void BeginRender(Scene* scene);

	// End Rendering.
	void EndRender();

	// Render a single frame of the Scene.
	void Render();

	// Return the renderer vulkan instance.
	inline VKIInstance* GetVKInstance() { return mVKData.instance.get(); }

	// Return the renderer vulkan swapchain.
	inline VKISwapChain* GetVKSwapChain() { return mVKData.swapchain.get(); }

	// Return the renderer vulkan device.
	inline VKIDevice* GetVKDevice() { return mVKData.device.get(); }

	// Return the rendrer pipeline.
	inline RendererPipeline* GetPipeline() { return mPipeline.get(); }

	// Return the rendrer pipeline.
	RenderUniform* GetMaterialUniform();

	// Return the renderer sphere.
	inline RenderSphere* GetSphere() { return mRSphere.get(); }
	inline RenderSphere* GetSphereLow() { return mRSphere.get(); }

	// Return
	inline Image2D* GetDefaultImage(uint32_t i) { return mDefaultImages[i].get(); }

	// Wait for the queues to be Idle/
	void WaitForIdle();

private:
	// Create Vulkan Sync Objects.
	void CreateVKSync();

	// Acquire Next Concurrent Frame.
	void NextFrame();

	// Record Commands for current frame.
	void RecordFrameCommands(uint32_t imgIndex);

	// Recreate the swapchain to match the current surface size.
	void RecreateSwapchain();

	// Load Default Images from file.
	void LoadDefaultImages();

public:
	// The number of concurrent frames we are allowed to render.
	static const uint32_t NUM_CONCURRENT_FRAMES;

private:
	// Vulkan Renderer Data.
	VKRenderData mVKData;

	// Flag to check if we are currently rendering.
	bool mIsRendering;

	// The index of the current frame we are rendering.
	uint32_t mCurrentFrame;

	// The Pipeline.
	UniquePtr<RendererPipeline> mPipeline;

	// The Render Data for the current scene we are rendering.
	UniquePtr<RenderScene> mRScene;

	// The Render Sphere Mesh, used to draw sphere for various render stages.
	UniquePtr<RenderSphere> mRSphere;

	// ImGUI Render UI.
	UniquePtr<RenderImGUI> mRenderUI;

	// Default Render Images used for material.
	Ptr<Image2D> mDefaultImages[2];
};


