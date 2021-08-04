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




#include "Renderer.h"
#include "Application.h"
#include "Core/GISystem.h"

#include "RendererPipeline.h"
#include "RenderData/RenderScene.h"
#include "RenderData/Shaders/RenderShader.h"
#include "RenderData/Shaders/RenderUniform.h"
#include "RenderData/Shaders/RenderShaderBlocks.h"
#include "RenderData/Shaders/RenderMaterial.h"
#include "RenderData/Primitives/RenderSphere.h"


#include "VKInterface/VKIInstance.h"
#include "VKInterface/VKIDevice.h"
#include "VKInterface/VKISwapChain.h"
#include "VKInterface/VKISync.h"
#include "VKInterface/VKICommandBuffer.h"
#include "VKInterface/VKIRenderPass.h"
#include "VKInterface/VKIFramebuffer.h"
#include "VKInterface/VKIDescriptor.h"
#include "VKInterface/VKIGraphicsPipeline.h"
#include "VKInterface/VKIBuffer.h"



#include <array>



const uint32_t Renderer::NUM_CONCURRENT_FRAMES = 2;






Renderer::Renderer()
	: mCurrentFrame(1)
	, mIsRendering(false)
{

}


Renderer::~Renderer()
{

}


void Renderer::Initialize()
{
	AppWindow* appWnd = Application::Get().GetMainWindow();


	// Vulkan Instance.
	mVKData.instance = UniquePtr<VKIInstance>(new VKIInstance());
	mVKData.instance->CreateInstance();
	mVKData.instance->CreateSurface(appWnd);
	mVKData.instance->PickPhysicalDevice();

	// Vulkan Device.
	mVKData.device = UniquePtr<VKIDevice>(new VKIDevice());
	mVKData.device->CreateDevice(mVKData.instance.get());

	// Vulkan Swapchain.
	mVKData.swapchain = UniquePtr<VKISwapChain>(new VKISwapChain());
	mVKData.swapchain->CreateSwapchain(mVKData.device.get());

	// Create Command buffers.
	mVKData.device->CreateCommandPool();
	mVKData.device->CreateCommandBuffers(NUM_CONCURRENT_FRAMES);

	// Create Vulkan Sync Objects.
	CreateVKSync();

	// The Renderer Sphere.
	mRSphere = UniquePtr<RenderSphere>(new RenderSphere());
	mRSphere->UpdateData(32);

	// The Renderer Pipeline.
	mPipeline = UniquePtr<RendererPipeline>(new RendererPipeline());
	mPipeline->Initialize();

	// Render Scene.
	mRScene = UniquePtr<RenderScene>(new RenderScene());
	mRScene->Initialize();


	// Material Shaders.
	RenderMaterial::SetupMaterialShaders(this, mRScene->GetTransformUniform());
}


void Renderer::Destroy()
{
	//
	mRSphere.reset();

	// Destroy Material Shaders.
	RenderMaterial::DestroyMaterialShaders();

	// Destroy the scene Render Data.
	mRScene->Destroy();

	// Destroy the Renderer Pipeline Data.
	mPipeline->Destroy();

	// Destroy Sync Objects.
	for (size_t i = 0; i < mVKData.frameSync.size(); ++i)
	{
		mVKData.frameSync[i].smImage->Destroy();
		mVKData.frameSync[i].smRender->Destroy();
		mVKData.frameSync[i].fnFrame->Destroy();
	}


	// Destroy Swapcahin.
	mVKData.swapchain->Destroy();

	// Destroy Logical Device.
	mVKData.device->Destroy();

	// Destroy Instance.
	mVKData.instance->Destroy();
}


void Renderer::CreateVKSync()
{
	auto& frameSync = mVKData.frameSync;
	frameSync.resize(NUM_CONCURRENT_FRAMES);

	// Create Fame Sync Objects...
	for (uint32_t i = 0; i < NUM_CONCURRENT_FRAMES; ++i)
	{
		frameSync[i].smImage = UniquePtr<VKISemaphore>(new VKISemaphore());
		frameSync[i].smImage->CreateSemaphore(mVKData.device.get());

		frameSync[i].smRender = UniquePtr<VKISemaphore>(new VKISemaphore());
		frameSync[i].smRender->CreateSemaphore(mVKData.device.get());

		frameSync[i].fnFrame = UniquePtr<VKIFence>(new VKIFence());
		frameSync[i].fnFrame->CreateFence(mVKData.device.get(), true);
	}

}


void Renderer::NextFrame()
{
	uint32_t nxtFrame = (mCurrentFrame + 1) % NUM_CONCURRENT_FRAMES;

	// Wait for frame to be ready...
	VKIFence* fnFrame = mVKData.frameSync[nxtFrame].fnFrame.get();
	fnFrame->Wait(UINT32_MAX);
	fnFrame->Reset(); // Reset Signal.

	mCurrentFrame = nxtFrame;
}


void Renderer::BeginRender(Scene* scene)
{
	CHECK(!mIsRendering && "No Rendering should be enabled.");
	mIsRendering = true;

	// Collect Render Data from sceen.
	mRScene->BuildRenderScene(scene);

	// Submit all transient command buffers.
	mVKData.device->SubmitTransientCmd();

}


void Renderer::EndRender()
{
	CHECK(mIsRendering && "BeginRender should be called first.");
	mIsRendering = false;

	// Clear render scene data.
	mRScene->Reset();

	// Wait for all transient command buffers to be submited.
	mVKData.device->WaitForTransientCmd();


	// To avoid the next frame from using previous frame data. we wait when update happen.
	if (mPipeline->IsWaitForUpdate())
	{
		WaitForIdle();
	}


	// Swapchain need to be recreated?
	if (mVKData.swapchain->NeedRecreate())
	{
		RecreateSwapchain();
	}

}


void Renderer::Render()
{
	CHECK(mIsRendering && "BeginRender should be called first.");

	// Next Concurrent Frame...
	NextFrame();


	// Current Frame Sync Data
	VKIFence* fnFrame = mVKData.frameSync[mCurrentFrame].fnFrame.get();
	VKISemaphore* smImage = mVKData.frameSync[mCurrentFrame].smImage.get();
	VKISemaphore* smRender = mVKData.frameSync[mCurrentFrame].smRender.get();


	// Acquire Swapchain Image...
	uint32_t imgIndex = mVKData.swapchain->AcquireNextImage(smImage, fnFrame);

	// Failed to Acquire?
	if (imgIndex == INVALID_UINDEX)
		return;

	// Swapchain need to be recreated?
	if (mVKData.swapchain->NeedRecreate())
		return;


	// Begin Pipeline.
	glm::vec4 viewport(0.0f, 0.0f, mVKData.swapchain->GetExtent().width, mVKData.swapchain->GetExtent().height);
	mPipeline->BeginRender(mCurrentFrame, mRScene.get(), viewport);


	// Render Command Buffers...
	VKICommandBuffer* cmdBuffer = mVKData.device->GetDrawCmd();
	cmdBuffer->SetCurrent(mCurrentFrame);

	RecordFrameCommands(imgIndex);
	// TODO: UI DARW COMMANDS...


	std::array<VkCommandBuffer, 1> cmdBuffers = {
		mVKData.device->GetDrawCmd()->Get(mCurrentFrame)
	};


	// Submit to Graphics Queue...
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = (uint32_t)cmdBuffers.size();
	submitInfo.pCommandBuffers = cmdBuffers.data();

	// Submit-Sync...
	std::array<VkSemaphore, 1> smWait = { smImage->Get() };
	submitInfo.waitSemaphoreCount = (uint32_t)smWait.size();
  submitInfo.pWaitSemaphores = smWait.data();

	std::array<VkPipelineStageFlags, 1> stageWait = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	if (mPipeline->IsWaitForUpdate())
		stageWait[0] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  submitInfo.pWaitDstStageMask = stageWait.data();

	std::array<VkSemaphore, 1> smSignal = { smRender->Get() };
  submitInfo.signalSemaphoreCount = (uint32_t)smSignal.size();
  submitInfo.pSignalSemaphores = smSignal.data();

	if (vkQueueSubmit(mVKData.device->GetGFXQueue(), 1, &submitInfo, fnFrame->Get()) != VK_SUCCESS)
	{
		CHECK(0 && "Failed to submit draw command buffer!");
		return;
	}

	// Present Rendererd Frame...
	mVKData.swapchain->PresentImage(imgIndex, smRender);

	// End Pipeline.
	mPipeline->EndRender();
}


void Renderer::RecordFrameCommands(uint32_t imgIndex)
{
	VKICommandBuffer* cmdBuffer = mVKData.device->GetDrawCmd();
	VkCommandBuffer cmd = cmdBuffer->GetCurrent();


	// Begin...
	VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	// Pipeline...
	mPipeline->Render(cmdBuffer);

	// Don't Render To swapchain while updating...
	mPipeline->FinalToSwapchain(cmdBuffer, imgIndex);

	// End.
	vkEndCommandBuffer(cmd);
}


void Renderer::WaitForIdle()
{
	LOGE("-> WAIT");
	vkDeviceWaitIdle(mVKData.device->Get());

}


void Renderer::RecreateSwapchain()
{
	WaitForIdle();
	mVKData.instance->ReQuerySurface();
	mVKData.swapchain->Recreate();

	VkExtent2D swExtent = mVKData.swapchain->GetExtent();
	mPipeline->Resize(glm::ivec2(swExtent.width, swExtent.height));
}
