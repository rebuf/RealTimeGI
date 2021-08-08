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







#include "RenderImGUI.h"
#include "Render/Renderer.h"
#include "Application.h"
#include "AppWindow.h"
#include "AppUser.h"


#include "Render/VKInterface/VKIInstance.h"
#include "Render/VKInterface/VKIDevice.h"
#include "Render/VKInterface/VKISwapChain.h"
#include "Render/VKInterface/VKICommandBuffer.h"
#include "Render/VKInterface/VKIRenderPass.h"

#include "Core/UI/imGUI/imgui.h"
#include "Core/UI/imGUI/imgui_impl_vulkan.h"
#include "Core/UI/imGUI/imgui_impl_glfw.h"





class ImGUIDescriptorPool
{
public:
  VkDescriptorPool handle;
};






RenderImGUI::RenderImGUI()
{

}


RenderImGUI::~RenderImGUI()
{

}


void RenderImGUI::SetupVulkan()
{
  // UI Command Buffer.
  mCmdBuffer = UniquePtr<VKICommandBuffer>(new VKICommandBuffer());
  mCmdBuffer->CreateCmdBuffer(mDevice, mDevice->GetCmdPool(), Renderer::NUM_CONCURRENT_FRAMES);

  // UI Render Pass.
  {
    mRenderPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());

    mRenderPass->SetColorAttachment(
      0, mSwapchain->GetFormat().format,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      VK_ATTACHMENT_LOAD_OP_LOAD,
      true
    );


    // Dependency...
    mRenderPass->AddDependency(
      VK_SUBPASS_EXTERNAL, 0,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      0,
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    );


    mRenderPass->CreateRenderPass(mDevice);
  }


  // UI Descriptor Pool
  {
    mPool = UniquePtr<ImGUIDescriptorPool>(new ImGUIDescriptorPool());

    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(mDevice->Get(), &pool_info, nullptr, &mPool->handle);
  }

}


void RenderImGUI::Initialize(Renderer* renderer, AppWindow* wnd)
{
  mDevice = renderer->GetVKDevice();
  mSwapchain = renderer->GetVKSwapChain();
  VKIInstance* vki = renderer->GetVKInstance();

  // Vulkan...
  SetupVulkan();

 
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForVulkan(wnd->GetHandle(), true);
  ImGui_ImplVulkan_InitInfo init_info{};
  init_info.Instance = vki->Get();
  init_info.PhysicalDevice = vki->GetPhysicalDevice();
  init_info.Device = mDevice->Get();
  init_info.QueueFamily = vki->GetQueues().graphics;
  init_info.Queue = mDevice->GetGFXQueue();
  init_info.PipelineCache = nullptr;
  init_info.DescriptorPool = mPool->handle;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = nullptr;
  ImGui_ImplVulkan_Init(&init_info, mRenderPass->Get());


  // Fonts...
  UploadFonts();

}


void RenderImGUI::Destroy()
{
  mCmdBuffer->Destroy();

}


void RenderImGUI::UploadFonts()
{
  ImGui::GetIO().Fonts->AddFontDefault();

  // Upload Fonts
  {
    VkCommandBuffer cmd = mDevice->BeginTransientCmd();

    // Upload...
    ImGui_ImplVulkan_CreateFontsTexture(cmd);

    mDevice->EndTransientCmd(cmd, Delegate<>());
    mDevice->SubmitTransientCmd();
    mDevice->WaitForTransientCmd();

    // Cleanup...
    ImGui_ImplVulkan_InvalidateFontUploadObjects();


  }

}



void RenderImGUI::RenderFrame(uint32_t imgIndex, uint32_t frame)
{
  // Start the Dear ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::CaptureMouseFromApp();
  ImGui::NewFrame();

  // User UI...
  Application::Get().GetUser()->UpdateImGui();


  // Rendering
  ImGui::Render();

  // Frame Rendering....
  RecordFrameCommands(imgIndex, frame);
}


void RenderImGUI::RecordFrameCommands(uint32_t imgIndex, uint32_t frame)
{
  mCmdBuffer->SetCurrent(frame);
  VkCommandBuffer cmd = mCmdBuffer->GetCurrent();


  // Begin...
  VkCommandBufferBeginInfo cmdBeginInfo{};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(cmd, &cmdBeginInfo);

  // Render Pass...
  glm::ivec4 viewport(0, 0, mSwapchain->GetExtent().width, mSwapchain->GetExtent().height);
  mRenderPass->Begin(mCmdBuffer.get(), mSwapchain->GetFrameBuffer(imgIndex), viewport);

  // Record Imgui Draw Data and draw funcs into command buffer
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

  mRenderPass->End(mCmdBuffer.get());


  // End...
  vkEndCommandBuffer(cmd);
}
