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






#include "RenderShadow.h"
#include "Application.h"
#include "Render/Renderer.h"
#include "Render/RendererPipeline.h"

#include "Render/VKInterface/VKIImage.h"
#include "Render/VKInterface/VKIFramebuffer.h"
#include "Render/VKInterface/VKICommandBuffer.h"








RenderDirShadow::RenderDirShadow()
	: mSize(0, 0)
{

}


RenderDirShadow::~RenderDirShadow()
{

}



void RenderDirShadow::SetSize(glm::ivec2 size)
{
	mSize = size;
}


void RenderDirShadow::Create()
{
	VKIDevice* device = Application::Get().GetRenderer()->GetVKDevice();
	RendererPipeline* pipeline = Application::Get().GetRenderer()->GetPipeline();
	VkExtent2D size = { (uint32_t)mSize.x, (uint32_t)mSize.y };

	// Shadow Map.
	mShadowMap = UniquePtr<VKIImage>(new VKIImage());
	mShadowMap->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, size, VK_IMAGE_LAYOUT_UNDEFINED);
	mShadowMap->SetUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	mShadowMap->Create(device);

	// View.
	mView = UniquePtr<VKIImageView>(new VKIImageView());
	mView->SetType(VK_IMAGE_VIEW_TYPE_2D);
	mView->SetViewInfo(VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1);
	mView->Create(device, mShadowMap.get());

	// Sampler.
	mSampler = UniquePtr<VKISampler>(new VKISampler());
	mSampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	mSampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
	mSampler->CreateSampler(device);

	// Framebuffer.
	mFramebuffer = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
	mFramebuffer->SetImgView(0, mView.get());
	mFramebuffer->SetSize(size);
	mFramebuffer->CreateFrameBuffer(device, pipeline->GetDirShadowPass());

}


void RenderDirShadow::Destroy()
{
	mFramebuffer->Destroy();
	mSampler->Destroy();
	mView->Destroy();
	mShadowMap->Destroy();
}


void RenderDirShadow::ApplyViewport(VKICommandBuffer* cmdBuffer)
{
	VkViewport viewport = { 0, 0, (float)mSize.x, (float)mSize.y, 0.0f, 1.0f };
	VkRect2D scissor = { 0, 0, (uint32_t)mSize.x, (uint32_t)mSize.y };

	vkCmdSetViewport(cmdBuffer->GetCurrent(), 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer->GetCurrent(), 0, 1, &scissor);
}



const glm::mat4& RenderDirShadow::GetShadowMatrix() const
{
	return mShadowMatrix;
}


const glm::vec3& RenderDirShadow::GetLightPos() const
{
	static glm::vec3 tmp = glm::vec3(0.0f);
	return tmp;
}
