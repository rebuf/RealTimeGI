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




#include "RenderStageLightProbes.h"
#include "Application.h"
#include "Renderer.h"
#include "Render/RendererPipeline.h"
#include "RenderData/RenderScene.h"
#include "RenderData/RenderShadow.h"
#include "RenderData/RenderLight.h"
#include "RenderData/Primitives/RenderSphere.h"
#include "RenderData/Shaders/RenderShader.h"
#include "RenderData/Shaders/RenderUniform.h"


#include "VKInterface/VKIDevice.h"
#include "VKInterface/VKIImage.h"
#include "VKInterface/VKIRenderPass.h"
#include "VKInterface/VKIFramebuffer.h"
#include "VKInterface/VKIDescriptor.h"
#include "VKInterface/VKICommandBuffer.h"
#include "VKInterface/VKISwapChain.h"
#include "VKInterface/VKIGraphicsPipeline.h"





	// Construct.
RenderStageLightProbes::RenderStageLightProbes()
{

}


RenderStageLightProbes::~RenderStageLightProbes()
{

}


void RenderStageLightProbes::Initialize(VKIDevice* device, StageRenderTarget hdrTargets[2],
	StageRenderTarget* dephtTarget, RenderUniform* commonUniform)
{
	mDevice = device;
	mCommon = commonUniform;
	mHDRTarget[0] = &hdrTargets[0];
	mDepth = dephtTarget;

	// The Render Sphere.
	mSphere = Application::Get().GetRenderer()->GetSphere();


	SetupCaptureCubePass();
	SetupIrradianceFilter();
	SetupLightingPass();
	SetupVisualizePass();
}


void RenderStageLightProbes::Destroy()
{

}


void RenderStageLightProbes::RenderCaptureCube(VKICommandBuffer* cmdBuffer, uint32_t frame,
	RenderLightProbe* lightProbe, uint32_t face, const glm::ivec4& viewport)
{
	mCaptureCubeRenderPass->Begin(cmdBuffer, lightProbe->GetRadianceFB(), viewport);
	mCaptureCubeShader->Bind(cmdBuffer);
	mCaptureCubeShader->GetDescriptorSet()->Bind(cmdBuffer, frame, mCaptureCubeShader->GetPipeline());

	int32_t layer = (int32_t)face;

	vkCmdPushConstants(cmdBuffer->GetCurrent(),
		mCaptureCubeShader->GetPipeline()->GetLayout(),
		VK_SHADER_STAGE_GEOMETRY_BIT,
		0, sizeof(int32_t), &layer);

	vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
	mCaptureCubeRenderPass->End(cmdBuffer);
}


void RenderStageLightProbes::RenderCaptureCube(VKICommandBuffer* cmdBuffer, uint32_t frame,
	RenderIrradianceVolume* volume, uint32_t layer, const glm::ivec4& viewport)
{
	mCaptureCubeRenderPass->Begin(cmdBuffer, volume->GetRadianceFB(), viewport);
	mCaptureCubeShader->Bind(cmdBuffer);
	mCaptureCubeShader->GetDescriptorSet()->Bind(cmdBuffer, frame, mCaptureCubeShader->GetPipeline());

	int32_t ilayer = (int32_t)layer;

	vkCmdPushConstants(cmdBuffer->GetCurrent(),
		mCaptureCubeShader->GetPipeline()->GetLayout(),
		VK_SHADER_STAGE_GEOMETRY_BIT,
		0, sizeof(int32_t), &ilayer);

	vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
	mCaptureCubeRenderPass->End(cmdBuffer);
}


void RenderStageLightProbes::FilterIrradiance(VKICommandBuffer* cmdBuffer, uint32_t frame, 
	RenderLightProbe* lightProbe, const glm::ivec4& viewport)
{
	lightProbe->GetIrradiance()->TransitionImageLayout(cmdBuffer->GetCurrent(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	mIrradianceFilterPass->Begin(cmdBuffer, lightProbe->GetIrradianceFB(), viewport);
	mIrradianceFilter->Bind(cmdBuffer);

	lightProbe->GetRadianceDescSet()->Bind(cmdBuffer, frame, mIrradianceFilter->GetPipeline());

	mSphere->Draw(cmdBuffer);
	mIrradianceFilterPass->End(cmdBuffer);

	lightProbe->GetIrradiance()->TransitionImageLayout(cmdBuffer->GetCurrent(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
}


void RenderStageLightProbes::FilterIrradianceVolume(VKICommandBuffer* cmdBuffer, uint32_t frame,
	RenderIrradianceVolume* volume, uint32_t probe, const glm::ivec4& viewport)
{
	volume->GetIrradiance()->TransitionImageLayout(cmdBuffer->GetCurrent(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	mIrradianceFilterPass->Begin(cmdBuffer, volume->GetIrradianceFB(), viewport);
	mIrradianceArrayFilter->Bind(cmdBuffer);
	volume->GetRadianceDescSet()->Bind(cmdBuffer, frame, mIrradianceArrayFilter->GetPipeline());

	int32_t layer = probe;

	vkCmdPushConstants(cmdBuffer->GetCurrent(),
		mIrradianceArrayFilter->GetPipeline()->GetLayout(),
		VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
		0, sizeof(int32_t), &layer);


	mSphere->Draw(cmdBuffer);

	mIrradianceFilterPass->End(cmdBuffer);


	volume->GetIrradiance()->TransitionImageLayout(cmdBuffer->GetCurrent(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
}


void RenderStageLightProbes::Render(VKICommandBuffer* cmdBuffer, uint32_t frame, 
	const std::vector<RenderLightProbe*>& lightProbes)
{
	mLightingShader->Bind(cmdBuffer);

	GUniform::LightProbeConstants constants{};

	for (size_t i = 0; i < lightProbes.size(); ++i)
	{
		if (lightProbes[i]->GetDirty() == LIGHT_PROBES_BOUNCES)
			continue;

		lightProbes[i]->GetLightingDescSet()->Bind(cmdBuffer, frame, mLightingShader->GetPipeline());

		constants.ProbePosition = glm::vec4(lightProbes[i]->GetPosition(), 0.0f);
		constants.Radius.x = lightProbes[i]->GetRadius();

		vkCmdPushConstants(cmdBuffer->GetCurrent(), mLightingShader->GetPipeline()->GetLayout(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(GUniform::LightProbeConstants), &constants);


		vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
	}
}


void RenderStageLightProbes::Render(VKICommandBuffer* cmdBuffer, uint32_t frame, 
	const std::vector<RenderIrradianceVolume*>& volumes)
{
	mLightingVolumeShader->Bind(cmdBuffer);

	GUniform::IrradianceVolumeConstants constants{};


	for (size_t i = 0; i < volumes.size(); ++i)
	{
		if (volumes[i]->GetDirty() == LIGHT_PROBES_BOUNCES)
			continue;

		volumes[i]->GetLightingDescSet()->Bind(cmdBuffer, frame, mLightingVolumeShader->GetPipeline());

		constants.start = glm::vec4(volumes[i]->GetVolumeStart(), 0.0f);
		constants.extent = glm::vec4(volumes[i]->GetVolumeExtent(), 0.0f);
		constants.count = glm::vec4(volumes[i]->GetVolumeCount(), 0);


		vkCmdPushConstants(cmdBuffer->GetCurrent(), mLightingVolumeShader->GetPipeline()->GetLayout(),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(GUniform::IrradianceVolumeConstants), &constants);


		vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
	}
}


void RenderStageLightProbes::SetupCaptureCubePass()
{
	VkExtent2D size = { LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE };

	// Cube Image...
	mCaptureCubeTarget.image = UniquePtr<VKIImage>(new VKIImage());
	mCaptureCubeTarget.image->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	mCaptureCubeTarget.image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size,
		VK_IMAGE_LAYOUT_UNDEFINED);
	mCaptureCubeTarget.image->SetLayers(6, true);
	mCaptureCubeTarget.image->Create(mDevice);

	mCaptureCubeTarget.view = UniquePtr<VKIImageView>(new VKIImageView());
	mCaptureCubeTarget.view->SetType(VK_IMAGE_VIEW_TYPE_CUBE);
	mCaptureCubeTarget.view->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6);
	mCaptureCubeTarget.view->Create(mDevice, mCaptureCubeTarget.image.get());

	mCaptureCubeTarget.sampler = UniquePtr<VKISampler>(new VKISampler());
	mCaptureCubeTarget.sampler->SetFilter(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
	mCaptureCubeTarget.sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	mCaptureCubeTarget.sampler->CreateSampler(mDevice);


	// RenderPass...
	mCaptureCubeRenderPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());
	mCaptureCubeRenderPass->SetColorAttachment(0, mCaptureCubeTarget.image->GetFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_LOAD, true);


	mCaptureCubeRenderPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	mCaptureCubeRenderPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT);

	mCaptureCubeRenderPass->CreateRenderPass(mDevice);


	// Framebuffer...
	mCaptureCubeFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
	mCaptureCubeFB->SetSize(size);
	mCaptureCubeFB->SetLayers(6);
	mCaptureCubeFB->SetImgView(0, mCaptureCubeTarget.view.get());
	mCaptureCubeFB->CreateFrameBuffer(mDevice, mCaptureCubeRenderPass.get());


	// Shader...
	mCaptureCubeShader = UniquePtr<RenderShader>(new RenderShader());
	mCaptureCubeShader->SetDomain(ERenderShaderDomain::Screen);
	mCaptureCubeShader->SetRenderPass(mCaptureCubeRenderPass.get());
	mCaptureCubeShader->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
	mCaptureCubeShader->SetShader(ERenderShaderStage::Geometry, SHADERS_DIRECTORY "CubeCaptureGeom.spv");
	mCaptureCubeShader->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "CubeCaptureFrag.spv");
	mCaptureCubeShader->SetViewport(glm::ivec4(0, 0, size.width, size.height));
	mCaptureCubeShader->SetViewportDynamic(true);
	mCaptureCubeShader->SetBlendingEnabled(0, false);

	mCaptureCubeShader->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
		ERenderShaderStage::AllStages);

	mCaptureCubeShader->AddInput(1, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	mCaptureCubeShader->AddInput(2, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	mCaptureCubeShader->AddPushConstant(0, 0, sizeof(int32_t), ERenderShaderStage::Geometry);

	mCaptureCubeShader->Create();

	// DescriptorSet...
	VKIDescriptorSet* descriptorSet = mCaptureCubeShader->CreateDescriptorSet();
	descriptorSet->SetLayout(mCaptureCubeShader->GetLayout());
	descriptorSet->CreateDescriptorSet(mDevice, Renderer::NUM_CONCURRENT_FRAMES);

	descriptorSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_ALL, mCommon->GetBuffers());

	descriptorSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
		mHDRTarget[0]->view.get(), mHDRTarget[0]->sampler.get());

	descriptorSet->AddDescriptor(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
		mDepth->view.get(), mDepth->sampler.get());

	descriptorSet->UpdateSets();
}


void RenderStageLightProbes::SetupIrradianceFilter()
{
	VkExtent2D size = { LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE };


	// RenderPass...
	mIrradianceFilterPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());
	mIrradianceFilterPass->SetColorAttachment(0, VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE, true);


	mIrradianceFilterPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	mIrradianceFilterPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT);

	mIrradianceFilterPass->CreateRenderPass(mDevice);


	// IBLFilter Shader
	{
		mIrradianceFilter = UniquePtr<RenderShader>(new RenderShader());
		mIrradianceFilter->SetDomain(ERenderShaderDomain::Mesh);
		mIrradianceFilter->SetRenderPass(mIrradianceFilterPass.get());
		mIrradianceFilter->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "SphereVert.spv");
		mIrradianceFilter->SetShader(ERenderShaderStage::Geometry, SHADERS_DIRECTORY "SphereGeom.spv");
		mIrradianceFilter->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "IBLFilter_Irradiance.spv");
		mIrradianceFilter->SetViewport(glm::ivec4(0, 0, size.width, size.height));
		mIrradianceFilter->SetViewportDynamic(true);
		mIrradianceFilter->SetBlendingEnabled(0, false);

		mIrradianceFilter->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		mIrradianceFilter->AddInput(1, ERenderShaderInputType::Uniform,
			ERenderShaderStage::Geometry);

		mIrradianceFilter->AddInput(2, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mIrradianceFilter->Create();
	}


	// IBLFilter Array Shader
	{
		mIrradianceArrayFilter = UniquePtr<RenderShader>(new RenderShader());
		mIrradianceArrayFilter->SetDomain(ERenderShaderDomain::Mesh);
		mIrradianceArrayFilter->SetRenderPass(mIrradianceFilterPass.get());
		mIrradianceArrayFilter->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "SphereVert.spv");
		mIrradianceArrayFilter->SetShader(ERenderShaderStage::Geometry, SHADERS_DIRECTORY "SphereGeom_IrradianceArray.spv");
		mIrradianceArrayFilter->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "IBLFilter_IrradianceArray.spv");
		mIrradianceArrayFilter->SetViewport(glm::ivec4(0, 0, size.width, size.height));
		mIrradianceArrayFilter->SetViewportDynamic(true);
		mIrradianceArrayFilter->SetBlendingEnabled(0, false);

		mIrradianceArrayFilter->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		mIrradianceArrayFilter->AddInput(1, ERenderShaderInputType::Uniform,
			ERenderShaderStage::Geometry);

		mIrradianceArrayFilter->AddInput(2, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mIrradianceArrayFilter->AddPushConstant(0, 0, sizeof(int32_t),
			ERenderShaderStage::Geometry | ERenderShaderStage::Fragment);

		mIrradianceArrayFilter->Create();
	}
	

}


void RenderStageLightProbes::SetupLightingPass()
{
	RendererPipeline* rpipeline = Application::Get().GetRenderer()->GetPipeline();


	// Probe Lighting Shader.
	{
		mLightingShader = UniquePtr<RenderShader>(new RenderShader());
		mLightingShader->SetDomain(ERenderShaderDomain::Screen);
		mLightingShader->SetRenderPass(rpipeline->GetLightingPass());
		mLightingShader->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
		mLightingShader->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "LightingPass_LightProbe.spv");
		mLightingShader->SetViewport(glm::ivec4(0, 0, 1920.0, 1080.0));
		mLightingShader->SetViewportDynamic(true);
		mLightingShader->SetBlendingEnabled(0, true);
		mLightingShader->SetBlending(0, ERenderBlendFactor::SrcAlpha, ERenderBlendFactor::One,
			ERenderBlendOp::Add);

		mLightingShader->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		mLightingShader->AddInput(1, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingShader->AddInput(2, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingShader->AddInput(3, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingShader->AddInput(4, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingShader->AddInput(6, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingShader->AddInput(7, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingShader->AddPushConstant(0, 0, sizeof(GUniform::LightProbeConstants), ERenderShaderStage::Fragment);

		mLightingShader->Create();
	}



	// Irradiance Volume Lighting Shader.
	{
		mLightingVolumeShader = UniquePtr<RenderShader>(new RenderShader());
		mLightingVolumeShader->SetDomain(ERenderShaderDomain::Screen);
		mLightingVolumeShader->SetRenderPass(rpipeline->GetLightingPass());
		mLightingVolumeShader->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
		mLightingVolumeShader->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "LightingPass_IrradianceVolume.spv");
		mLightingVolumeShader->SetViewport(glm::ivec4(0, 0, 1920.0, 1080.0));
		mLightingVolumeShader->SetViewportDynamic(true);
		mLightingVolumeShader->SetBlendingEnabled(0, true);
		mLightingVolumeShader->SetBlending(0, ERenderBlendFactor::One, ERenderBlendFactor::One,
			ERenderBlendOp::Add);

		mLightingVolumeShader->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		mLightingVolumeShader->AddInput(1, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingVolumeShader->AddInput(2, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingVolumeShader->AddInput(3, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingVolumeShader->AddInput(4, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingVolumeShader->AddInput(6, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingVolumeShader->AddInput(7, ERenderShaderInputType::ImageSampler,
			ERenderShaderStage::Fragment);

		mLightingVolumeShader->AddPushConstant(0, 0, sizeof(GUniform::IrradianceVolumeConstants), ERenderShaderStage::Fragment);

		mLightingVolumeShader->Create();
	}


}


void RenderStageLightProbes::SetupVisualizePass()
{
	RendererPipeline* rpipeline = Application::Get().GetRenderer()->GetPipeline();

	// Shader...
	mVisualizeProbeShader = UniquePtr<RenderShader>(new RenderShader());
	mVisualizeProbeShader->SetDomain(ERenderShaderDomain::Screen);
	mVisualizeProbeShader->SetRenderPass(rpipeline->GetLightingPass());
	mVisualizeProbeShader->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
	mVisualizeProbeShader->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "VisualizePass.spv");
	mVisualizeProbeShader->SetViewport(glm::ivec4(0, 0, 1920.0, 1080.0));
	mVisualizeProbeShader->SetViewportDynamic(true);
	mVisualizeProbeShader->SetBlendingEnabled(0, false);

	mVisualizeProbeShader->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
		ERenderShaderStage::AllStages);

	mVisualizeProbeShader->AddInput(10, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	mVisualizeProbeShader->AddInput(11, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	mVisualizeProbeShader->Create();

}


void RenderStageLightProbes::RenderVisualize(VKICommandBuffer* cmdBuffer, uint32_t frame, RenderLightProbe* lightProbe)
{
	if (!lightProbe)
		return;


	if (lightProbe->GetDirty() == 2)
		return;

	mVisualizeProbeShader->Bind(cmdBuffer);
	lightProbe->GetVisualizeDescSet()->Bind(cmdBuffer, frame, mVisualizeProbeShader->GetPipeline());
	vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
}
