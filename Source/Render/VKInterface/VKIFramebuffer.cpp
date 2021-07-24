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



#include "VKIFramebuffer.h"
#include "VKIDevice.h"
#include "VKIRenderPass.h"
#include "VKIImage.h"






VKIFramebuffer::VKIFramebuffer()
  : mLayers(1)
  , mHandle(VK_NULL_HANDLE)
{
  mSize = { 0, 0 };
}


VKIFramebuffer::~VKIFramebuffer()
{

}


void VKIFramebuffer::SetImgView(uint32_t idx, VKIImageView* imgView)
{
	if (mImgViews.size() < idx + 1)
		mImgViews.resize(idx + 1);

	mImgViews[idx] = imgView;
}


void VKIFramebuffer::ClearImgViews()
{
	mImgViews.clear();
}


void VKIFramebuffer::SetSize(VkExtent2D size)
{
  mSize = size;
}


void VKIFramebuffer::SetLayers(uint32_t layers)
{
  mLayers = layers;
}


void VKIFramebuffer::CreateFrameBuffer(VKIDevice* owner, VKIRenderPass* renderPass)
{
	CHECK(!mImgViews.empty() && "Empty Image Views.");
  mVKDevice = owner;
  mRenderPass = renderPass;

  // Attachments...
  std::vector<VkImageView> attachments;
  attachments.resize(mImgViews.size());

  for (size_t i = 0; i < mImgViews.size(); ++i)
  {
    attachments[i] = mImgViews[i]->Get();
  }


  // Create Framebuffer....
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = mRenderPass->Get();
  framebufferInfo.attachmentCount = (uint32_t)attachments.size();
  framebufferInfo.pAttachments = attachments.data();
  framebufferInfo.width = mSize.width;
  framebufferInfo.height = mSize.height;
  framebufferInfo.layers = mLayers;

  VkResult result = vkCreateFramebuffer(mVKDevice->Get(), &framebufferInfo, nullptr, &mHandle);
  CHECK(result == VK_SUCCESS);
}


void VKIFramebuffer::Destroy()
{
  // Destroy Framebuffer.
  vkDestroyFramebuffer(mVKDevice->Get(), mHandle, nullptr);

  //...
  mHandle = VK_NULL_HANDLE;
}
