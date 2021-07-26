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



#include "RenderLight.h"
#include "Application.h"
#include "Render/Renderer.h"
#include "Render/RendererPipeline.h"
#include "Render/RenderStageLightProbes.h"
#include "RenderTypes.h"

#include "Render/VKInterface/VKIImage.h"
#include "Render/VKInterface/VKIRenderPass.h"
#include "Render/VKInterface/VKIFramebuffer.h"







// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 





RenderLightProbe::RenderLightProbe()
	: mPosition(0.0f)
	, mRadius(0.0f)
{

}


RenderLightProbe::~RenderLightProbe()
{

}



void RenderLightProbe::Create()
{
	VKIDevice* device = Application::Get().GetRenderer()->GetVKDevice();
	RendererPipeline* rpipeline = Application::Get().GetRenderer()->GetPipeline();
	VkExtent2D size = { LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE };


	// Irradiance Map.
	{
		mIrradiance = UniquePtr<VKIImage>(new VKIImage());
		mIrradiance->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, size, VK_IMAGE_LAYOUT_UNDEFINED);
		mIrradiance->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mIrradiance->SetLayers(6, true);
		mIrradiance->Create(device);

		// Image View.
		mView[0] = UniquePtr<VKIImageView>(new VKIImageView());
		mView[0]->SetType(VK_IMAGE_VIEW_TYPE_CUBE);
		mView[0]->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6);
		mView[0]->Create(device, mIrradiance.get());

		// Sampler.
		mSampler[0] = UniquePtr<VKISampler>(new VKISampler());
		mSampler[0]->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mSampler[0]->SetFilter(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
		mSampler[0]->CreateSampler(device);

		// Framebuffer for pre-filter pass
		IrradianceFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
		IrradianceFB->SetSize(size);
		IrradianceFB->SetLayers(6);
		IrradianceFB->SetImgView(0, mView[0].get());
		IrradianceFB->CreateFrameBuffer(device, rpipeline->GetStageLightProbes()->GetIrradianceFilterPass());
	}



}


void RenderLightProbe::Destroy()
{
	mIrradiance->Destroy();
	mView[0]->Destroy();
	mSampler[0]->Destroy();
	IrradianceFB->Destroy();

}

