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




#include "RendererPipeline.h"
#include "Core/Transform.h"
#include "Application.h"
#include "Renderer.h"
#include "RenderStageLightProbes.h"
#include "RenderData/RenderScene.h"
#include "RenderData/RenderShadow.h"
#include "RenderData/RenderLight.h"
#include "RenderData/Shaders/RenderShader.h"
#include "RenderData/Shaders/RenderUniform.h"


#include "VKInterface/VKIDevice.h"
#include "VKInterface/VKISwapChain.h"
#include "VKInterface/VKIImage.h"
#include "VKInterface/VKIRenderPass.h"
#include "VKInterface/VKIFramebuffer.h"
#include "VKInterface/VKIDescriptor.h"
#include "VKInterface/VKICommandBuffer.h"
#include "VKInterface/VKISwapChain.h"
#include "VKInterface/VKIGraphicsPipeline.h"


#include "glm/gtc/type_ptr.hpp"








void StageRenderTarget::Reset()
{
	image.reset();
	view.reset();
	sampler.reset();
}



void StageRenderTarget::Destroy()
{
	image->Destroy();
	view->Destroy();
	sampler->Destroy();
}



// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 






RendererPipeline::RendererPipeline()
	: mDevice(nullptr)
	, mSwapchain(nullptr)
	, mSize(0, 0)
	, mIsRendering(false)
	, mFrame(0)
{

}


RendererPipeline::~RendererPipeline()
{

}



void RendererPipeline::Initialize()
{
	// Get Renderer Vulkan Data...
	mDevice = Application::Get().GetRenderer()->GetVKDevice();
	mSwapchain = Application::Get().GetRenderer()->GetVKSwapChain();

	// Initial Targets Size.
	mSize = glm::ivec2(1920, 1080);

	// Create Renderer Pipeline Uniforms 
	SetupUniforms();

	// ...
	SetupTargets();
	SetupGBufferPass();
	SetupLightingPass();
	SetupPostProcessPass();
	SetupBlitSwapchain();
	SetupShadowPasses();

	//
	mStageLightProbes = UniquePtr<RenderStageLightProbes>(new RenderStageLightProbes());
	mStageLightProbes->Initialize(mDevice, mHDRTarget, mUniforms.common.get());
}


void RendererPipeline::Resize(const glm::ivec2& size)
{
	// Invalid Size?
	if (size.x == 0 || size.y == 0)
		return;

	// Smaller Size?
	if (size.x <= mSize.x && size.y <= mSize.y)
		return;


	// Resize Targets...
	CHECK(0 && "TODO: Resize Render Targets...");
}


void RendererPipeline::BeginRender(uint32_t frame, RenderScene* rscene, const glm::vec4& viewport)
{
	CHECK(!mIsRendering);
	mIsRendering = true;
	mFrame = frame;
	mScene = rscene;
	mViewport = viewport;
	mIntViewport = glm::ivec4(viewport);


	// Common Block.
	{
		mCommonBlock.viewProjMatrix = rscene->GetViewProj();
		mCommonBlock.viewProjMatrixInverse = rscene->GetViewProjInv();
		mCommonBlock.viewDir = rscene->GetViewDir();
		mCommonBlock.viewPos = rscene->GetViewPos();
		mCommonBlock.viewport = mViewport;
		mCommonBlock.targetSize = glm::vec4(mSize, 0.0f, 0.0f); // The Actual Size of the target
		mCommonBlock.sunDir = rscene->GetEnvironment().sunDir;
		mCommonBlock.sunColorAndPower = rscene->GetEnvironment().sunColorAndPower;
		mCommonBlock.nearFar = rscene->GetNearFar();
		mCommonBlock.time = Application::Get().GetAppTime();

		mUniforms.common->Update(frame, &mCommonBlock); // Update Common Block.
	}


}


void RendererPipeline::EndRender()
{
	CHECK(mIsRendering)
	mIsRendering = false;
	mFrame = INVALID_UINDEX;
}


void RendererPipeline::Render(VKICommandBuffer* cmdBuffer)
{
	CHECK(mIsRendering);

	// Update shadow maps if needed...
	//UpdateShadows(cmdBuffer);
	
	// Update light probes if needed...
	UpdateLightProbes(cmdBuffer);



	// Viewport...
	VkViewport viewport = { mViewport.x, mViewport.y, mViewport.z, mViewport.w, 0.0f, 1.0f };
	VkRect2D scissor = { { (int32_t)mIntViewport.x, (int32_t)mIntViewport.y },
		{ (uint32_t)mIntViewport.z, (uint32_t)mIntViewport.w } };

	vkCmdSetViewport(cmdBuffer->GetCurrent(), 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer->GetCurrent(), 0, 1, &scissor);


	// Testing............................................................
	return;


	// --- -- - -- ---
	// The Scene.
	RenderSceneStage(cmdBuffer);



	// --- -- - -- --- -- -
	// Tone-Mapping Pass.
	{
		mPostProPass->Begin(cmdBuffer, mPostProFB.get(), mIntViewport);
		mPostProShader->Bind(cmdBuffer);
		mPostProShader->GetDescriptorSet()->Bind(cmdBuffer, mFrame, mPostProShader->GetPipeline());
		vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
		mPostProPass->End(cmdBuffer);
	}


}


void RendererPipeline::FinalToSwapchain(VKICommandBuffer* cmdBuffer, uint32_t imgIndex)
{
	mSwapchain->GetRenderPass()->Begin(cmdBuffer, mSwapchain->GetFrameBuffer(imgIndex), mIntViewport);
	mBlitSwapchain->Bind(cmdBuffer);
	mBlitSwapchain->GetDescriptorSet()->Bind(cmdBuffer, mFrame, mBlitSwapchain->GetPipeline());
	
	// Testing............................................................
	//vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);

	mSwapchain->GetRenderPass()->End(cmdBuffer);
}


void RendererPipeline::RenderSceneStage(VKICommandBuffer* cmdBuffer)
{

	// G-Buffer Pass...
	{
		mGBufferPass->Begin(cmdBuffer, mGBufferFB.get(), mIntViewport);
		mScene->DrawSceneDeferred(cmdBuffer, mFrame);
		mGBufferPass->End(cmdBuffer);
	}



	// Lighting Pass...
	{
		mLightingPass->Begin(cmdBuffer, mLightingFB.get(), mIntViewport);
		mLightingShader->Bind(cmdBuffer);
		mLightingShader->GetDescriptorSet()->Bind(cmdBuffer, mFrame, mLightingShader->GetPipeline());
		vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
		mLightingPass->End(cmdBuffer);
	}

}


void RendererPipeline::UpdateShadows(VKICommandBuffer* cmdBuffer)
{
	// The Sun Shadow...
	if (mScene->GetSunShadow()->IsDirty())
	{
		RenderDirShadow* shadow = mScene->GetSunShadow();
		shadow->ApplyViewport(cmdBuffer);
		mDirShadowPass->Begin(cmdBuffer, shadow->GetFramebuffer(), shadow->GetViewport());
		mScene->DrawSceneShadow(cmdBuffer, mFrame, mScene->GetSunShadow());
		mDirShadowPass->End(cmdBuffer);

		// Clear Flag.
		mScene->GetSunShadow()->SetDirty(false);
	}

}


void RendererPipeline::UpdateLightProbes(VKICommandBuffer* cmdBuffer)
{
	// Is scene's light probes update to date?
	if (!mScene->HasDirtyLightProbes())
		return;


	glm::vec4 rViewport(0.0f, 0.0f, LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE);
	glm::ivec4 riViewport(0, 0, LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE);

	mUniforms.common->CmdUpdate(cmdBuffer, mFrame,
		offsetof(GUniform::CommonBlock, viewport), 16, glm::value_ptr(rViewport));


	// Viewport...
	VkViewport viewport = { rViewport.x, rViewport.y, rViewport.z, rViewport.w, 0.0f, 1.0f };
	VkRect2D scissor = { { (int32_t)rViewport.x, (int32_t)rViewport.y },
		{ (uint32_t)rViewport.z, (uint32_t)rViewport.w } };

	vkCmdSetViewport(cmdBuffer->GetCurrent(), 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer->GetCurrent(), 0, 1, &scissor);



	// iteratet over all the light probes in the scene and update the dirty ones
	const std::vector<RenderLightProbe*>& lightProbes = mScene->GetLightProbes();


	for (RenderLightProbe* probe : lightProbes)
	{
		// Need Update?
		if (!probe->IsDirty())
			continue;

		mStageLightProbes->GetCaptureCube()->TransitionImageLayout(cmdBuffer->GetCurrent(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

		// Capture the scene for each cubemap face.
		for (uint32_t iface = 0; iface < 6; ++iface)
		{

			glm::mat4 probeFaceViewProj = Transform::GetCubeViewProj(iface, probe->GetPosition());

			mUniforms.common->CmdUpdate(cmdBuffer, mFrame,
				offsetof(GUniform::CommonBlock, viewProjMatrix), sizeof(glm::mat4), glm::value_ptr(probeFaceViewProj));

			// G-Buffer Pass...
			{
				mGBufferPass->Begin(cmdBuffer, mGBufferFB.get(), riViewport);
				mScene->DrawSceneDeferred(cmdBuffer, mFrame);
				mGBufferPass->End(cmdBuffer);
			}


			// Lighting Pass...
			{
				mLightingPass->Begin(cmdBuffer, mLightingFB.get(), riViewport);
				mLightingShader->Bind(cmdBuffer);
				mLightingShader->GetDescriptorSet()->Bind(cmdBuffer, mFrame, mLightingShader->GetPipeline());
				vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
				mLightingPass->End(cmdBuffer);
			}

			// Render the captured scene into the cubemap.
			mStageLightProbes->RenderCaptureCube(cmdBuffer, mFrame, iface, riViewport);
		}

		mStageLightProbes->GetCaptureCube()->TransitionImageLayout(cmdBuffer->GetCurrent(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);


		// Pre-Filter cube map and store it into the probe images to be used later for lighting.
		mStageLightProbes->FilterCaptureCube(cmdBuffer, mFrame, probe, riViewport);


		// Clear Dirty Flag.
		// Testing..................................................................
		probe->SetDirty(false);
	}





	// Reset Common Block Uniform...
	mUniforms.common->CmdUpdate(cmdBuffer, mFrame,
		0, sizeof(GUniform::CommonBlock), &mCommonBlock);

}


void RendererPipeline::Destroy()
{
	// Destroy Stages...
	mStageLightProbes->Destroy();


	// Destory Uniforms...
	mUniforms.common->Destroy();
	mUniforms.common->Destroy();


	// Destory G-Buffer...
	mAlbedoTarget.Destroy();
	mBRDFTarget.Destroy();
	mNormalsTarget.Destroy();
	mDepthTarget.Destroy();
	mGBufferPass->Destroy();
	mGBufferFB->Destroy();


	// Targets...
	mHDRTarget[0].Destroy();
	mHDRTarget[1].Destroy();
	mLDRTarget[0].Destroy();
	mLDRTarget[1].Destroy();

	// Passes...
	mLightingPass->Destroy();
	mLightingFB->Destroy();
	mLightingShader->Destroy();

	mPostProPass->Destroy();
	mPostProFB->Destroy();
	mPostProShader->Destroy();

	mBlitSwapchain->Destroy();
	mDirShadowPass->Destroy();

}


void RendererPipeline::SetupUniforms()
{
	mUniforms.common = UniquePtr<RenderUniform>(new RenderUniform());
	mUniforms.common->SetTransferDst(true);
	mUniforms.common->Create(Application::Get().GetRenderer(), sizeof(GUniform::CommonBlock), false);

}


void RendererPipeline::SetupGBufferPass()
{
	const VkExtent2D size = { (uint32_t)mSize.x, (uint32_t)mSize.y };

	// Albedo...
	{
		mAlbedoTarget.image = UniquePtr<VKIImage>(new VKIImage());
		mAlbedoTarget.image->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mAlbedoTarget.image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, size,
			VK_IMAGE_LAYOUT_UNDEFINED);
		mAlbedoTarget.image->Create(mDevice);

		mAlbedoTarget.view = UniquePtr<VKIImageView>(new VKIImageView());
		mAlbedoTarget.view->SetType(VK_IMAGE_VIEW_TYPE_2D);
		mAlbedoTarget.view->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
		mAlbedoTarget.view->Create(mDevice, mAlbedoTarget.image.get());

		mAlbedoTarget.sampler = UniquePtr<VKISampler>(new VKISampler());
		mAlbedoTarget.sampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
		mAlbedoTarget.sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mAlbedoTarget.sampler->CreateSampler(mDevice);
	}


	// BRDF...
	{
		mBRDFTarget.image = UniquePtr<VKIImage>(new VKIImage());
		mBRDFTarget.image->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mBRDFTarget.image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, size,
			VK_IMAGE_LAYOUT_UNDEFINED);
		mBRDFTarget.image->Create(mDevice);

		mBRDFTarget.view = UniquePtr<VKIImageView>(new VKIImageView());
		mBRDFTarget.view->SetType(VK_IMAGE_VIEW_TYPE_2D);
		mBRDFTarget.view->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
		mBRDFTarget.view->Create(mDevice, mBRDFTarget.image.get());

		mBRDFTarget.sampler = UniquePtr<VKISampler>(new VKISampler());
		mBRDFTarget.sampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
		mBRDFTarget.sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mBRDFTarget.sampler->CreateSampler(mDevice);
	}


	// Normals...
	{
		mNormalsTarget.image = UniquePtr<VKIImage>(new VKIImage());
		mNormalsTarget.image->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mNormalsTarget.image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size,
			VK_IMAGE_LAYOUT_UNDEFINED);
		mNormalsTarget.image->Create(mDevice);

		mNormalsTarget.view = UniquePtr<VKIImageView>(new VKIImageView());
		mNormalsTarget.view->SetType(VK_IMAGE_VIEW_TYPE_2D);
		mNormalsTarget.view->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
		mNormalsTarget.view->Create(mDevice, mNormalsTarget.image.get());

		mNormalsTarget.sampler = UniquePtr<VKISampler>(new VKISampler());
		mNormalsTarget.sampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
		mNormalsTarget.sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mNormalsTarget.sampler->CreateSampler(mDevice);
	}


	// Depth...
	{
		mDepthTarget.image = UniquePtr<VKIImage>(new VKIImage());
		mDepthTarget.image->SetUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mDepthTarget.image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, size,
			VK_IMAGE_LAYOUT_UNDEFINED);
		mDepthTarget.image->Create(mDevice);

		mDepthTarget.view = UniquePtr<VKIImageView>(new VKIImageView());
		mDepthTarget.view->SetType(VK_IMAGE_VIEW_TYPE_2D);
		mDepthTarget.view->SetViewInfo(VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1);
		mDepthTarget.view->Create(mDevice, mDepthTarget.image.get());

		mDepthTarget.sampler = UniquePtr<VKISampler>(new VKISampler());
		mDepthTarget.sampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
		mDepthTarget.sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mDepthTarget.sampler->CreateSampler(mDevice);
	}


	// RenderPass...
	mGBufferPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());
	mGBufferPass->SetColorAttachment(0, mAlbedoTarget.image->GetFormat(), 
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR, true);

	mGBufferPass->SetColorAttachment(1, mBRDFTarget.image->GetFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR, true);

	mGBufferPass->SetColorAttachment(2, mNormalsTarget.image->GetFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR, true);

	mGBufferPass->SetDepthAttachment(mDepthTarget.image->GetFormat(),
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR, true, false);

	mGBufferPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_ACCESS_MEMORY_READ_BIT, 
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);

	mGBufferPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);


	std::vector<VkClearValue> clearValues(4);
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[3].depthStencil = { 1.0f, 0};
	mGBufferPass->SetClearValues(clearValues);

	mGBufferPass->CreateRenderPass(mDevice);

	// Framebuffer...
	mGBufferFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
	mGBufferFB->SetSize(size);
	mGBufferFB->SetImgView(0, mAlbedoTarget.view.get());
	mGBufferFB->SetImgView(1, mBRDFTarget.view.get());
	mGBufferFB->SetImgView(2, mNormalsTarget.view.get());
	mGBufferFB->SetImgView(3, mDepthTarget.view.get());
	mGBufferFB->CreateFrameBuffer(mDevice, mGBufferPass.get());

}


void RendererPipeline::SetupTargets()
{
	VkExtent2D size = { (uint32_t)mSize.x, (uint32_t)mSize.y };

	// HDR 0 & 1...
	for (uint32_t i = 0; i < 2; ++i)
	{
		mHDRTarget[i].image = UniquePtr<VKIImage>(new VKIImage());
		mHDRTarget[i].image->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mHDRTarget[i].image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size,
			VK_IMAGE_LAYOUT_UNDEFINED);
		mHDRTarget[i].image->Create(mDevice);

		mHDRTarget[i].view = UniquePtr<VKIImageView>(new VKIImageView);
		mHDRTarget[i].view->SetType(VK_IMAGE_VIEW_TYPE_2D);
		mHDRTarget[i].view->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
		mHDRTarget[i].view->Create(mDevice, mHDRTarget[i].image.get());

		mHDRTarget[i].sampler = UniquePtr<VKISampler>(new VKISampler());
		mHDRTarget[i].sampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
		mHDRTarget[i].sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mHDRTarget[i].sampler->CreateSampler(mDevice);
	}


	// LDR 0 & 1...
	for (uint32_t i = 0; i < 2; ++i)
	{
		mLDRTarget[i].image = UniquePtr<VKIImage>(new VKIImage());
		mLDRTarget[i].image->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		mLDRTarget[i].image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, size,
			VK_IMAGE_LAYOUT_UNDEFINED);
		mLDRTarget[i].image->Create(mDevice);

		mLDRTarget[i].view = UniquePtr<VKIImageView>(new VKIImageView);
		mLDRTarget[i].view->SetType(VK_IMAGE_VIEW_TYPE_2D);
		mLDRTarget[i].view->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
		mLDRTarget[i].view->Create(mDevice, mLDRTarget[i].image.get());

		mLDRTarget[i].sampler = UniquePtr<VKISampler>(new VKISampler());
		mLDRTarget[i].sampler->SetFilter(VK_FILTER_NEAREST, VK_FILTER_NEAREST);
		mLDRTarget[i].sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		mLDRTarget[i].sampler->CreateSampler(mDevice);
	}

}


void RendererPipeline::SetupLightingPass()
{
	const VkExtent2D size = { (uint32_t)mSize.x, (uint32_t)mSize.y };

	// Render Pass...
	mLightingPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());

	mLightingPass->SetColorAttachment(0, mHDRTarget[0].image->GetFormat(), 
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE, true);

	mLightingPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);

	mLightingPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);

	mLightingPass->CreateRenderPass(mDevice);


	// Framebuffer...
	mLightingFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
	mLightingFB->SetImgView(0, mHDRTarget[0].view.get());
	mLightingFB->SetSize(size);
	mLightingFB->CreateFrameBuffer(mDevice, mLightingPass.get());


	// Shader...
	mLightingShader = UniquePtr<RenderShader>(new RenderShader());
	mLightingShader->SetDomain(ERenderShaderDomain::Screen);
	mLightingShader->SetRenderPass(mLightingPass.get());
	mLightingShader->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
	mLightingShader->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "LightingPass.spv");
	mLightingShader->SetViewport(glm::ivec4(0, 0, size.width, size.height));
	mLightingShader->SetViewportDynamic(true);
	mLightingShader->SetBlendingEnabled(0, false);

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

	mLightingShader->Create();


	// Descriptors...
	VKIDescriptorSet* lightingDescSet = mLightingShader->CreateDescriptorSet();
	lightingDescSet->SetLayout(mLightingShader->GetLayout());
	lightingDescSet->CreateDescriptorSet(mDevice, Renderer::NUM_CONCURRENT_FRAMES);

	lightingDescSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_ALL, mUniforms.common->GetBuffers());

	lightingDescSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, mAlbedoTarget.view.get(), mAlbedoTarget.sampler.get());

	lightingDescSet->AddDescriptor(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, mBRDFTarget.view.get(), mBRDFTarget.sampler.get());

	lightingDescSet->AddDescriptor(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, mNormalsTarget.view.get(), mNormalsTarget.sampler.get());

	lightingDescSet->AddDescriptor(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		VK_SHADER_STAGE_FRAGMENT_BIT, mDepthTarget.view.get(), mDepthTarget.sampler.get());

	lightingDescSet->UpdateSets();

}


void RendererPipeline::SetupPostProcessPass()
{
	const VkExtent2D size = { (uint32_t)mSize.x, (uint32_t)mSize.y };

	// Render Pass...
	mPostProPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());

	mPostProPass->SetColorAttachment(0, mLDRTarget[0].image->GetFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE, true);

	mPostProPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);

	mPostProPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);

	mPostProPass->CreateRenderPass(mDevice);


	// Framebuffer...
	mPostProFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
	mPostProFB->SetImgView(0, mLDRTarget[0].view.get());
	mPostProFB->SetSize(size);
	mPostProFB->CreateFrameBuffer(mDevice, mPostProPass.get());

	// Shader...
	mPostProShader = UniquePtr<RenderShader>(new RenderShader());
	mPostProShader->SetDomain(ERenderShaderDomain::Screen);
	mPostProShader->SetRenderPass(mPostProPass.get());
	mPostProShader->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
	mPostProShader->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "PostProcess.spv");
	mPostProShader->SetBlendingEnabled(0, false);
	mPostProShader->SetViewport(glm::ivec4(0, 0, size.width, size.height));
	mPostProShader->SetViewportDynamic(true);
	mPostProShader->AddInput(0, ERenderShaderInputType::Uniform, ERenderShaderStage::AllStages);
	mPostProShader->AddInput(1, ERenderShaderInputType::ImageSampler, ERenderShaderStage::Fragment);
	mPostProShader->Create();

	// Descriptors...
	VKIDescriptorSet* ppDescSet = mPostProShader->CreateDescriptorSet();
	ppDescSet->SetLayout(mPostProShader->GetLayout());
	ppDescSet->CreateDescriptorSet(mDevice, Renderer::NUM_CONCURRENT_FRAMES);

	ppDescSet->AddDescriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL,
		mUniforms.common->GetBuffers());

	ppDescSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
		mHDRTarget[0].view.get(), mHDRTarget[0].sampler.get());

	ppDescSet->UpdateSets();

}


void RendererPipeline::SetupBlitSwapchain()
{
	const VkExtent2D size = { (uint32_t)mSize.x, (uint32_t)mSize.y };

	// Shader...
	mBlitSwapchain = UniquePtr<RenderShader>(new RenderShader());
	mBlitSwapchain->SetDomain(ERenderShaderDomain::Screen);
	mBlitSwapchain->SetRenderPass(mSwapchain->GetRenderPass());
	mBlitSwapchain->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
	mBlitSwapchain->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "FinalBlit.spv");
	mBlitSwapchain->SetBlendingEnabled(0, false);
	mBlitSwapchain->SetViewport(glm::ivec4(0, 0, size.width, size.height));
	mBlitSwapchain->SetViewportDynamic(true);
	mBlitSwapchain->AddInput(0, ERenderShaderInputType::Uniform, ERenderShaderStage::AllStages);
	mBlitSwapchain->AddInput(1, ERenderShaderInputType::ImageSampler, ERenderShaderStage::Fragment);
	mBlitSwapchain->Create();

	// Descriptor...
	VKIDescriptorSet* swDescSet = mBlitSwapchain->CreateDescriptorSet();
	swDescSet->SetLayout(mBlitSwapchain->GetLayout());
	swDescSet->CreateDescriptorSet(mDevice, Renderer::NUM_CONCURRENT_FRAMES);

	swDescSet->AddDescriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL,
		mUniforms.common->GetBuffers());

	swDescSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
		mLDRTarget[0].view.get(), mLDRTarget[0].sampler.get());

	swDescSet->UpdateSets();

}


void RendererPipeline::SetupShadowPasses()
{
	// Directional Shadow Pass...
	mDirShadowPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());

	mDirShadowPass->SetDepthAttachment(VK_FORMAT_D32_SFLOAT, 
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_CLEAR, true, false);

	mGBufferPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);

	mGBufferPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_DEPENDENCY_BY_REGION_BIT);

	std::vector<VkClearValue> clearValues(1);
	clearValues[0].depthStencil = { 1.0f, 0 };
	mDirShadowPass->SetClearValues(clearValues);

	mDirShadowPass->CreateRenderPass(mDevice);




	// Omni-directional Shadow Pass...


}
