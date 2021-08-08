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





#include "RenderImage.h"
#include "Core/Image2D.h"
#include "Application.h"
#include "Render/Renderer.h"


#include "Render/VKInterface/VKIDevice.h"
#include "Render/VKInterface/VKIBuffer.h"
#include "Render/VKInterface/VKIImage.h"




VkFormat ToVKFormat(EImageFormat format, bool isSRGB)
{
	switch (format)
	{
	case EImageFormat::R:
		return isSRGB ? VK_FORMAT_R8_SRGB : VK_FORMAT_R8_UNORM;

	case EImageFormat::RGBA:
		return isSRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
	}

	CHECK(0 && "Unsupported.");
	return VK_FORMAT_UNDEFINED;
}






// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- ---




RenderImage::RenderImage()
	: mImgBuffer(nullptr)
{

}


RenderImage::~RenderImage()
{
	if (mImage)
	{
		mSampler->Destroy();
		mView->Destroy();
		mImage->Destroy();

		if (mImgBuffer)
		{
			mImgBuffer->Destroy();
			delete mImgBuffer;
		}
	}
}


void RenderImage::SetData(Image2D* img)
{
	mDevice = Application::Get().GetRenderer()->GetVKDevice();
	VkExtent2D size = { (uint32_t)img->GetSize().x, (uint32_t)img->GetSize().y };


	// Pixel Info...
	VkDeviceSize imgSize = img->GetImgData().GetSize();
	const uint8_t* imgData = img->GetImgData().GetData();


	// New Staging?
	if (!mImgBuffer)
	{
		mImgBuffer = new VKIBuffer();
		mImgBuffer->SetSize(imgSize);
		mImgBuffer->SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		mImgBuffer->SetMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mImgBuffer->CreateBuffer(mDevice);
	}


	// Update Data...
	mImgBuffer->UpdateData(0, imgSize, imgData);


	mImage = UniquePtr<VKIImage>(new VKIImage());
	mImage->SetImageInfo(VK_IMAGE_TYPE_2D, ToVKFormat(img->GetFormat(), img->IsSRGB()), size, VK_IMAGE_LAYOUT_UNDEFINED);
	mImage->SetMipLevels(1);
	mImage->SetUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	mImage->Create(mDevice);


	// Transient Command...
	VkCommandBuffer cmd = mDevice->BeginTransientCmd();
	
	mImage->TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
	mImage->UpdateImage(cmd, mImgBuffer);
	mImage->TransitionImageLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	mDevice->EndTransientCmd(cmd, Delegate<>::CreateMemberRaw(this, &RenderImage::DestroyStaging));
}


void RenderImage::DestroyStaging()
{
	mImgBuffer->Destroy();
	delete mImgBuffer;
	mImgBuffer = nullptr;
}


void RenderImage::CreateView()
{
	mView = UniquePtr<VKIImageView>(new VKIImageView());
	mView->SetType(VK_IMAGE_VIEW_TYPE_2D);
	mView->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, mImage->GetMipLevels(), 0, 1);
	mView->Create(mDevice, mImage.get());
}

void RenderImage::CreateSampler()
{
	mSampler = UniquePtr<VKISampler>(new VKISampler());
	mSampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT);
	mSampler->SetFilter(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
	mSampler->CreateSampler(mDevice);
}
