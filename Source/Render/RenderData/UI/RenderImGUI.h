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




class Renderer;
class AppWindow;
class VKIDevice;
class VKISwapChain;
class VKICommandBuffer;
class VKIRenderPass;
class ImGUIDescriptorPool;






// RenderImGUI
//    - Renderer interface with ImGui and rendering UI using vulkan.
//
class RenderImGUI
{
public:
	// Construct.
	RenderImGUI();

	// Destruct.
	~RenderImGUI();

	// Initialize ImGui.
	void Initialize(Renderer* renderer, AppWindow* wnd);

	// Destroy ImGui.
	void Destroy();

	// Render an ImGui frame and record the draw commands in the command buffer.
	void RenderFrame(uint32_t imgIndex, uint32_t frame);

	// Return the ui command buffers.
	VKICommandBuffer* GetCmdBuffer() { return mCmdBuffer.get(); }

private:
	// record the draw commands in the command buffer.
	void RecordFrameCommands(uint32_t imgIndex, uint32_t frame);

	// Setup vulkan object for ui.
	void SetupVulkan();

	// Upload ImGui fonts.
	void UploadFonts();

private:
	// The Vulkan Device.
	VKIDevice* mDevice;

	// The swapchain to draw ui on.
	VKISwapChain* mSwapchain;

	// Command Buffer for recording ui draw commands.
	UniquePtr<VKICommandBuffer> mCmdBuffer;

	// UI Pass.
	UniquePtr<VKIRenderPass> mRenderPass;

	// Vulkan Descriptor Pool for imgui.
	UniquePtr<ImGUIDescriptorPool> mPool;
};

