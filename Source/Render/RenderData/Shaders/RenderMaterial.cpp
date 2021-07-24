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
#include "RenderShader.h"
#include "RenderUniform.h"
#include "RenderShaderBlocks.h"


#include "Render/VKInterface/VKISwapChain.h"
#include "Render/VKInterface/VKIDescriptor.h"




Ptr<RenderShader> RenderMaterial::OPAQUE_SHADER;
Ptr<RenderShader> RenderMaterial::SHADOW_DIR_SHADER[2];
Ptr<RenderShader> RenderMaterial::SHADOW_OMNI_SHADER[2];
Ptr<VKIDescriptorSet> RenderMaterial::MAT_DESC_SET[2];





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
		OPAQUE_SHADER->SetViewport(0, 0, swExtent.width, swExtent.height);
		OPAQUE_SHADER->SetViewportDynamic(true);
		OPAQUE_SHADER->SetDepth(true, true);

		OPAQUE_SHADER->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
			ERenderShaderStage::AllStages);

		OPAQUE_SHADER->Create();

	}


	// Shadow...
	{
		// Shadow for opaque objects.
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
	}



	// Descriptor Set
	{
		MAT_DESC_SET[0] = Ptr<VKIDescriptorSet>(new VKIDescriptorSet());
		MAT_DESC_SET[0]->SetLayout(OPAQUE_SHADER->GetLayout());
		MAT_DESC_SET[0]->CreateDescriptorSet(renderer->GetVKDevice(), Renderer::NUM_CONCURRENT_FRAMES);
		MAT_DESC_SET[0]->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_ALL, commonUniform->GetBuffers());
		MAT_DESC_SET[0]->UpdateSets();


		MAT_DESC_SET[1] = Ptr<VKIDescriptorSet>(new VKIDescriptorSet());
		MAT_DESC_SET[1]->SetLayout(OPAQUE_SHADER->GetLayout());
		MAT_DESC_SET[1]->CreateDescriptorSet(renderer->GetVKDevice(), Renderer::NUM_CONCURRENT_FRAMES);
		MAT_DESC_SET[1]->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_SHADER_STAGE_ALL, commonUniform->GetBuffers());
		MAT_DESC_SET[1]->UpdateSets();
	}
}


void RenderMaterial::DestroyMaterialShaders()
{
	OPAQUE_SHADER->Destroy();

	SHADOW_DIR_SHADER[0]->Destroy();

	MAT_DESC_SET[0]->Destroy();
	MAT_DESC_SET[1]->Destroy();
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
{

}


RenderMaterial::~RenderMaterial()
{

}
