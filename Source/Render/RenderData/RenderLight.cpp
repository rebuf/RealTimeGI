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
#include "Render/RenderData/Shaders/RenderShader.h"
#include "Render/RenderData/Primitives/RenderSphere.h"
#include "RenderTypes.h"

#include "Render/VKInterface/VKIImage.h"
#include "Render/VKInterface/VKIRenderPass.h"
#include "Render/VKInterface/VKIFramebuffer.h"
#include "Render/VKInterface/VKIDescriptor.h"







// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 





RenderLightProbe::RenderLightProbe()
	: mPosition(0.0f)
	, mRadius(0.0f)
	, mIsDirty(2)
{

}


RenderLightProbe::~RenderLightProbe()
{

}



void RenderLightProbe::Create()
{
	VKIDevice* device = Application::Get().GetRenderer()->GetVKDevice();
	Renderer* renderer = Application::Get().GetRenderer();
	RendererPipeline* rpipeline = renderer->GetPipeline();
	VkExtent2D size = { LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE };


	// Irradiance Map.
	{
		mIrradiance = UniquePtr<VKIImage>(new VKIImage());
		mIrradiance->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size, VK_IMAGE_LAYOUT_UNDEFINED);
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
		mIrradianceFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
		mIrradianceFB->SetSize(size);
		mIrradianceFB->SetLayers(6);
		mIrradianceFB->SetImgView(0, mView[0].get());
		mIrradianceFB->CreateFrameBuffer(device, rpipeline->GetStageLightProbes()->GetIrradianceFilterPass());
	}


	// Radiance Map.
	{
		mRadiance = UniquePtr<VKIImage>(new VKIImage());
		mRadiance->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size, VK_IMAGE_LAYOUT_UNDEFINED);
		mRadiance->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mRadiance->SetLayers(6, true);
		mRadiance->Create(device);

		// Image View.
		mView[1] = UniquePtr<VKIImageView>(new VKIImageView());
		mView[1]->SetType(VK_IMAGE_VIEW_TYPE_CUBE);
		mView[1]->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6);
		mView[1]->Create(device, mRadiance.get());

		// Sampler.
		mSampler[1] = UniquePtr<VKISampler>(new VKISampler());
		mSampler[1]->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mSampler[1]->SetFilter(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
		mSampler[1]->CreateSampler(device);

		// Framebuffer for pre-filter pass
		mRadianceFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
		mRadianceFB->SetSize(size);
		mRadianceFB->SetLayers(6);
		mRadianceFB->SetImgView(0, mView[1].get());
		mRadianceFB->CreateFrameBuffer(device, rpipeline->GetStageLightProbes()->GetIrradianceFilterPass());
	}


	{
		mLightingSet = UniquePtr<VKIDescriptorSet>(new VKIDescriptorSet());
		mLightingSet->SetLayout(rpipeline->GetStageLightProbes()->GetLightingShader()->GetLayout());
		mLightingSet->CreateDescriptorSet(device, Renderer::NUM_CONCURRENT_FRAMES);

		rpipeline->AddGBufferToDescSet(mLightingSet.get());

		mLightingSet->AddDescriptor(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
			VK_SHADER_STAGE_FRAGMENT_BIT, mView[0].get(), mSampler[0].get());

		mLightingSet->AddDescriptor(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mView[1].get(), mSampler[1].get());

		mLightingSet->UpdateSets();
	}


	{
		mVisualizeSet = UniquePtr<VKIDescriptorSet>(new VKIDescriptorSet());
		mVisualizeSet->SetLayout(rpipeline->GetStageLightProbes()->GetVisualizeShader()->GetLayout());
		mVisualizeSet->CreateDescriptorSet(device, Renderer::NUM_CONCURRENT_FRAMES);

		mVisualizeSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_ALL, rpipeline->GetUniforms().common->GetBuffers());

		mVisualizeSet->AddDescriptor(10, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mView[0].get(), mSampler[0].get());

		mVisualizeSet->AddDescriptor(11, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mView[1].get(), mSampler[1].get());

		mVisualizeSet->UpdateSets();
	}


	{
		mIrradianceFilterSet = UniquePtr<VKIDescriptorSet>(new VKIDescriptorSet());
		mIrradianceFilterSet->SetLayout(rpipeline->GetStageLightProbes()->GetIrradianceFilterShader()->GetLayout());
		mIrradianceFilterSet->CreateDescriptorSet(device, Renderer::NUM_CONCURRENT_FRAMES);

		mIrradianceFilterSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_ALL, rpipeline->GetUniforms().common->GetBuffers());

		mIrradianceFilterSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT,
			renderer->GetSphere()->GetSphereUnifrom()->GetBuffers());

		mIrradianceFilterSet->AddDescriptor(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
			mView[1].get(), mSampler[1].get());

		mIrradianceFilterSet->UpdateSets();
	}
}


void RenderLightProbe::Destroy()
{
	mIrradiance->Destroy();
	mView[0]->Destroy();
	mSampler[0]->Destroy();
	mIrradianceFB->Destroy();

	mRadiance->Destroy();
	mView[1]->Destroy();
	mSampler[1]->Destroy();
	mRadianceFB->Destroy();

	mLightingSet->Destroy();
	mVisualizeSet->Destroy();
}







// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 



RenderIrradianceVolume::RenderIrradianceVolume()
	: mStart(0.0f)
	, mExtent(0.0f)
	, mCount(0)
{

}


RenderIrradianceVolume::~RenderIrradianceVolume()
{

}



void RenderIrradianceVolume::SetVolume(const glm::vec3& start, const glm::vec3& extent, const glm::ivec3& count)
{
	mStart = start;
	mExtent = extent;
	mCount = count;
}


uint32_t RenderIrradianceVolume::GetNumProbes()
{
	return mCount.x * mCount.y * mCount.z;
}


glm::ivec3 RenderIrradianceVolume::GetProbeGridCoord(uint32_t index)
{
	glm::ivec3 grid;

	grid.z = (index / (mCount.x * mCount.y));
	index -= grid.z * (mCount.x * mCount.y);

	grid.y = (index / mCount.x);
	index -= grid.y * mCount.x;

	grid.x = index;

	return grid;
}


glm::vec3 RenderIrradianceVolume::GetProbePosition(uint32_t index)
{
	glm::vec3 grid = GetProbeGridCoord(index);
	grid /= glm::vec3(mCount);
	return mStart + grid * mExtent + (mExtent / glm::vec3(mCount) * 0.5f);
}


uint32_t RenderIrradianceVolume::GetProbeLayer(uint32_t index, uint32_t face)
{
	return index * 6 + face;
}


void RenderIrradianceVolume::Create()
{
	CHECK(GetNumProbes() > 0);

	VKIDevice* device = Application::Get().GetRenderer()->GetVKDevice();
	Renderer* renderer = Application::Get().GetRenderer();
	RendererPipeline* rpipeline = renderer->GetPipeline();
	VkExtent2D size = { IRRADIANCE_VOLUME_TARGET_SIZE, IRRADIANCE_VOLUME_TARGET_SIZE };

	uint32_t numLayers = GetNumProbes() * 6; // Number of layers in light probe.

	// Irradiance Map.
	{
		mIrradiance = UniquePtr<VKIImage>(new VKIImage());
		mIrradiance->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size, VK_IMAGE_LAYOUT_UNDEFINED);
		mIrradiance->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mIrradiance->SetLayers(numLayers, true);
		mIrradiance->Create(device);

		// Image View.
		mView[0] = UniquePtr<VKIImageView>(new VKIImageView());
		mView[0]->SetType(VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);
		mView[0]->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, numLayers);
		mView[0]->Create(device, mIrradiance.get());

		// Sampler.
		mSampler[0] = UniquePtr<VKISampler>(new VKISampler());
		mSampler[0]->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mSampler[0]->SetFilter(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
		mSampler[0]->CreateSampler(device);

		// Framebuffer for pre-filter pass
		mIrradianceFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
		mIrradianceFB->SetSize(size);
		mIrradianceFB->SetLayers(numLayers);
		mIrradianceFB->SetImgView(0, mView[0].get());
		mIrradianceFB->CreateFrameBuffer(device, rpipeline->GetStageLightProbes()->GetIrradianceFilterPass());
	}


	// Radiance Map.
	{
		mRadiance = UniquePtr<VKIImage>(new VKIImage());
		mRadiance->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size, VK_IMAGE_LAYOUT_UNDEFINED);
		mRadiance->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mRadiance->SetLayers(numLayers, true);
		mRadiance->Create(device);

		// Image View.
		mView[1] = UniquePtr<VKIImageView>(new VKIImageView());
		mView[1]->SetType(VK_IMAGE_VIEW_TYPE_CUBE_ARRAY);
		mView[1]->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, numLayers);
		mView[1]->Create(device, mRadiance.get());

		// Sampler.
		mSampler[1] = UniquePtr<VKISampler>(new VKISampler());
		mSampler[1]->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mSampler[1]->SetFilter(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
		mSampler[1]->CreateSampler(device);

		// Framebuffer for pre-filter pass
		mRadianceFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
		mRadianceFB->SetSize(size);
		mRadianceFB->SetLayers(numLayers);
		mRadianceFB->SetImgView(0, mView[1].get());
		mRadianceFB->CreateFrameBuffer(device, rpipeline->GetStageLightProbes()->GetIrradianceFilterPass());
	}


	{
		mLightingSet = UniquePtr<VKIDescriptorSet>(new VKIDescriptorSet());
		mLightingSet->SetLayout(rpipeline->GetStageLightProbes()->GetLightingVolumeShader()->GetLayout());
		mLightingSet->CreateDescriptorSet(device, Renderer::NUM_CONCURRENT_FRAMES);

		rpipeline->AddGBufferToDescSet(mLightingSet.get());

		mLightingSet->AddDescriptor(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mView[0].get(), mSampler[0].get());

		mLightingSet->AddDescriptor(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mView[1].get(), mSampler[1].get());

		mLightingSet->UpdateSets();
	}


	{
		mIrradianceFilterSet = UniquePtr<VKIDescriptorSet>(new VKIDescriptorSet());
		mIrradianceFilterSet->SetLayout(rpipeline->GetStageLightProbes()->GetIrradianceArrayFilterShader()->GetLayout());
		mIrradianceFilterSet->CreateDescriptorSet(device, Renderer::NUM_CONCURRENT_FRAMES);

		mIrradianceFilterSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_ALL, rpipeline->GetUniforms().common->GetBuffers());

		mIrradianceFilterSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT,
			renderer->GetSphere()->GetSphereUnifrom()->GetBuffers());

		mIrradianceFilterSet->AddDescriptor(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
			mView[1].get(), mSampler[1].get());

		mIrradianceFilterSet->UpdateSets();
	}
}


void RenderIrradianceVolume::Destroy()
{
	mIrradiance->Destroy();
	mView[0]->Destroy();
	mSampler[0]->Destroy();
	mIrradianceFB->Destroy();

	mRadiance->Destroy();
	mView[1]->Destroy();
	mSampler[1]->Destroy();
	mRadianceFB->Destroy();

	mLightingSet->Destroy();
	mIrradianceFilterSet->Destroy();
}
