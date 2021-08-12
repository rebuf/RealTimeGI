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



#include "RenderMaterial.h"
#include "Render/Renderer.h"
#include "Render/RendererPipeline.h"
#include "Render/RenderData/RenderImage.h"
#include "RenderShader.h"
#include "RenderUniform.h"
#include "RenderShaderBlocks.h"


#include "Application.h"
#include "Core/Material.h"
#include "Core/Image2D.h"


#include "Render/VKInterface/VKISwapChain.h"
#include "Render/VKInterface/VKIDescriptor.h"
#include "Render/VKInterface/VKICommandBuffer.h"
#include "Render/VKInterface/VKIGraphicsPipeline.h"



#include <array>



Ptr<RenderShader> RenderMaterial::OPAQUE_SHADER;
Ptr<RenderShader> RenderMaterial::SHADOW_DIR_SHADER[2];
Ptr<RenderShader> RenderMaterial::SHADOW_OMNI_SHADER[2];

Ptr<RenderShader> RenderMaterial::SPHERE_HELPER_SHADER;







void RenderMaterial::SetupMaterialShaders(Renderer* renderer, RenderUniform* transformUniform)
{
	VkExtent2D swExtent = renderer->GetVKSwapChain()->GetExtent();
	RenderUniform* commonUniform = renderer->GetPipeline()->GetUniforms().common.get();

	// Opaque...
	{
		// Deferred.
		OPAQUE_SHADER = Ptr<RenderShader>(new RenderShader());
		OPAQUE_SHADER->SetDomain(ERenderShaderDomain::Mesh);
		OPAQUE_SHADER->SetRenderPass(renderer->GetPipeline()->GetGBufferPass());
		OPAQUE_SHADER->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "MeshVert.spv");
		OPAQUE_SHADER->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "MeshFrag.spv");
		OPAQUE_SHADER->SetBlendingEnabled(0, false);
		OPAQUE_SHADER->SetBlendingEnabled(1, false);
		OPAQUE_SHADER->SetBlendingEnabled(2, false);
		OPAQUE_SHADER->SetBlendingEnabled(3, false);
		OPAQUE_SHADER->SetViewport(0, 0, swExtent.width, swExtent.height);
		OPAQUE_SHADER->SetViewportDynamic(true);
		OPAQUE_SHADER->SetDepth(true, true);

		OPAQUE_SHADER->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		OPAQUE_SHADER->AddInput(3, ERenderShaderInputType::DynamicUniform, ERenderShaderStage::Fragment);
		OPAQUE_SHADER->AddInput(4, ERenderShaderInputType::ImageSampler, ERenderShaderStage::Fragment);
		OPAQUE_SHADER->AddInput(5, ERenderShaderInputType::ImageSampler, ERenderShaderStage::Fragment);

		OPAQUE_SHADER->Create();
	}


	// Shadow...
	{
		// Directional shadow for opaque. 
		SHADOW_DIR_SHADER[0] = Ptr<RenderShader>(new RenderShader());
		SHADOW_DIR_SHADER[0]->SetDomain(ERenderShaderDomain::Mesh);
		SHADOW_DIR_SHADER[0]->SetRenderPass(renderer->GetPipeline()->GetDirShadowPass());
		SHADOW_DIR_SHADER[0]->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "MeshVert_DirShadow.spv");
		SHADOW_DIR_SHADER[0]->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "MeshFrag_DirShadow.spv");
		SHADOW_DIR_SHADER[0]->SetViewport(0, 0, swExtent.width, swExtent.height);
		SHADOW_DIR_SHADER[0]->SetViewportDynamic(true);
		SHADOW_DIR_SHADER[0]->SetDepth(true, true);

		SHADOW_DIR_SHADER[0]->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		SHADOW_DIR_SHADER[0]->AddPushConstant(0, 0, sizeof(GUniform::ShadowConstantBlock),
			ERenderShaderStage::Vertex | ERenderShaderStage::Fragment);

		SHADOW_DIR_SHADER[0]->Create();


		VKIDescriptorSet* SHADOW_DIR_DESCSET = SHADOW_DIR_SHADER[0]->CreateDescriptorSet();
		SHADOW_DIR_DESCSET->SetLayout(SHADOW_DIR_SHADER[0]->GetLayout());
		SHADOW_DIR_DESCSET->CreateDescriptorSet(renderer->GetVKDevice(), Renderer::NUM_CONCURRENT_FRAMES);
		SHADOW_DIR_DESCSET->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_ALL, commonUniform->GetBuffers());
		SHADOW_DIR_DESCSET->UpdateSets();


		// Omni shadow for opaque. 
		SHADOW_OMNI_SHADER[0] = Ptr<RenderShader>(new RenderShader());
		SHADOW_OMNI_SHADER[0]->SetDomain(ERenderShaderDomain::Mesh);
		SHADOW_OMNI_SHADER[0]->SetRenderPass(renderer->GetPipeline()->GetDirShadowPass());
		SHADOW_OMNI_SHADER[0]->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "MeshVert_OmniShadow.spv");
		SHADOW_OMNI_SHADER[0]->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "MeshFrag_OmniShadow.spv");
		SHADOW_OMNI_SHADER[0]->SetViewport(0, 0, swExtent.width, swExtent.height);
		SHADOW_OMNI_SHADER[0]->SetViewportDynamic(true);
		SHADOW_OMNI_SHADER[0]->SetDepth(true, true);

		SHADOW_OMNI_SHADER[0]->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		SHADOW_OMNI_SHADER[0]->AddPushConstant(0, 0, sizeof(GUniform::ShadowConstantBlock),
			ERenderShaderStage::Vertex | ERenderShaderStage::Fragment);

		SHADOW_OMNI_SHADER[0]->Create();
	}



	// ...
	SetupSphereHelperShader(renderer);

}


void RenderMaterial::SetupSphereHelperShader(Renderer* renderer)
{
	VkExtent2D swExtent = renderer->GetVKSwapChain()->GetExtent();
	RenderUniform* commonUniform = renderer->GetPipeline()->GetUniforms().common.get();

	// Shader...
	SPHERE_HELPER_SHADER = UniquePtr<RenderShader>(new RenderShader());
	SPHERE_HELPER_SHADER->SetDomain(ERenderShaderDomain::Mesh);
	SPHERE_HELPER_SHADER->SetRenderPass(renderer->GetPipeline()->GetLightingPass());
	SPHERE_HELPER_SHADER->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "SphereVert_Helper.spv");
	SPHERE_HELPER_SHADER->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "SphereFrag_Helper.spv");
	SPHERE_HELPER_SHADER->SetViewport(glm::ivec4(0, 0, swExtent.width, swExtent.height));
	SPHERE_HELPER_SHADER->SetViewportDynamic(true);
	SPHERE_HELPER_SHADER->SetWireframe(false);
	SPHERE_HELPER_SHADER->SetBlendingEnabled(0, true);
	SPHERE_HELPER_SHADER->SetBlending(0, ERenderBlendFactor::SrcAlpha, ERenderBlendFactor::OneMinusSrcAlpha,
		ERenderBlendOp::Add);

	SPHERE_HELPER_SHADER->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
		ERenderShaderStage::AllStages);

	SPHERE_HELPER_SHADER->AddInput(1, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	SPHERE_HELPER_SHADER->AddInput(2, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	SPHERE_HELPER_SHADER->AddInput(3, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	SPHERE_HELPER_SHADER->AddInput(4, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	SPHERE_HELPER_SHADER->AddPushConstant(0, 0, sizeof(GUniform::SphereHelperBlock), 
		ERenderShaderStage::Vertex | ERenderShaderStage::Fragment);


	SPHERE_HELPER_SHADER->Create();

	// Descriptor Set
	VKIDescriptorSet* SPHERE_HELPER_DESCSET = SPHERE_HELPER_SHADER->CreateDescriptorSet();
	SPHERE_HELPER_DESCSET->SetLayout(SPHERE_HELPER_SHADER->GetLayout());

	SPHERE_HELPER_DESCSET->CreateDescriptorSet(renderer->GetVKDevice(), Renderer::NUM_CONCURRENT_FRAMES);

	renderer->GetPipeline()->AddGBufferToDescSet(SPHERE_HELPER_DESCSET);

	SPHERE_HELPER_DESCSET->UpdateSets();

}


void RenderMaterial::DestroyMaterialShaders()
{
	OPAQUE_SHADER->Destroy();

	SHADOW_DIR_SHADER[0]->Destroy();
	SHADOW_OMNI_SHADER[0]->Destroy();

	SPHERE_HELPER_SHADER->Destroy();
}


RenderShader* RenderMaterial::GetShader(ERenderMaterialType type)
{
	switch (type)
	{
	case ERenderMaterialType::Opaque: return OPAQUE_SHADER.get();
	}

	CHECK(0 && "Not Supported.");
	return nullptr;
}


RenderShader* RenderMaterial::GetDirShadowShader(ERenderMaterialType type)
{
	switch (type)
	{
	case ERenderMaterialType::Opaque: return SHADOW_DIR_SHADER[0].get();
	case ERenderMaterialType::Masked: return SHADOW_DIR_SHADER[1].get();
	}

	CHECK(0 && "Not Supported.");
	return nullptr;
}


RenderShader* RenderMaterial::GetLProbeShader(ERenderMaterialType type)
{
	//switch (type)
	//{
	//case ERenderShaderType::Opaque: return ;
	//}

	CHECK(0 && "Not Supported.");
	return nullptr;
}




// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 





RenderMaterial::RenderMaterial(ERenderMaterialType type)
	: mType(type)
	, mDynamicOffset(0)
{

}


RenderMaterial::~RenderMaterial()
{
	mDescriptorSet->Destroy();
}



void RenderMaterial::Setup(MaterialData* data, RenderImage* colorImage, RenderImage* roughnessMetallicImage)
{
	Renderer* renderer = Application::Get().GetRenderer();

	mMatData = data;
	mTextures[0] = colorImage ? colorImage : renderer->GetDefaultImage(0)->GetRenderImage();
	mTextures[1] = roughnessMetallicImage ? roughnessMetallicImage : renderer->GetDefaultImage(1)->GetRenderImage();

	switch (mType)
	{
	case ERenderMaterialType::Opaque:
	{
		mDescriptorSet = Ptr<VKIDescriptorSet>(new VKIDescriptorSet());
		mDescriptorSet->SetLayout(OPAQUE_SHADER->GetLayout());
		mDescriptorSet->CreateDescriptorSet(renderer->GetVKDevice(), Renderer::NUM_CONCURRENT_FRAMES);

		mDescriptorSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_ALL, renderer->GetPipeline()->GetUniforms().common->GetBuffers());

		mDescriptorSet->AddDescriptor(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			VK_SHADER_STAGE_FRAGMENT_BIT, renderer->GetMaterialUniform()->GetBuffers(),
			0, ALIGN_SIZE(sizeof(MaterialData), 64));

		mDescriptorSet->AddDescriptor(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mTextures[0]->GetView(), mTextures[0]->GetSampler());

		mDescriptorSet->AddDescriptor(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mTextures[1]->GetView(), mTextures[1]->GetSampler());

		mDescriptorSet->UpdateSets();
	}
		break;
	}
}


void RenderMaterial::Bind(VKICommandBuffer* cmdBuffer, uint32_t frame)
{
	VkPipelineLayout layout = VK_NULL_HANDLE;

	switch (mType)
	{
	case ERenderMaterialType::Opaque:
		layout = OPAQUE_SHADER->GetPipeline()->GetLayout();
		break;
	}

	std::array<uint32_t, 1> dynamicOffsets = { (uint32_t)mDynamicOffset * ALIGN_SIZE(sizeof(MaterialData), 64) };

	VkDescriptorSet descSet = mDescriptorSet->Get(frame);
	vkCmdBindDescriptorSets(cmdBuffer->GetCurrent(), VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout, 0, 1, &descSet, (uint32_t)dynamicOffsets.size(), dynamicOffsets.data());
}
