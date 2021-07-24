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






#include "RenderShader.h"
#include "Render/Renderer.h"
#include "Application.h"

#include "Render/VKInterface/VKITypes.h"
#include "Render/VKInterface/VKIGraphicsPipeline.h"
#include "Render/VKInterface/VKIDescriptor.h"
#include "Render/VKInterface/VKICommandBuffer.h"




#include "Core/Mesh.h"




const uint32_t RenderShader::COMMON_BLOCK_BINDING = 0;
const uint32_t RenderShader::TRANSFORM_BLOCK_BINDING = 1;






// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- ---




VkShaderStageFlagBits ToStageBits(ERenderShaderStage stage)
{
	uint32_t stageBits = 0;

	if ((stage & ERenderShaderStage::Vertex) == ERenderShaderStage::Vertex)
		stageBits |= VK_SHADER_STAGE_VERTEX_BIT;

	if ((stage & ERenderShaderStage::Fragment) == ERenderShaderStage::Fragment)
		stageBits |= VK_SHADER_STAGE_FRAGMENT_BIT;

	if ((stage & ERenderShaderStage::Geometry) == ERenderShaderStage::Geometry)
		stageBits |= VK_SHADER_STAGE_GEOMETRY_BIT;

	return static_cast<VkShaderStageFlagBits>(stageBits);
}





// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- ---







RenderShader::RenderShader()
	: mDomain(ERenderShaderDomain::None)
	, mIsDynamicViewport(false)
{
	mDescLayout = UniquePtr<VKIDescriptorLayout>(new VKIDescriptorLayout());

	mPipeline = UniquePtr<VKIGraphicsPipeline>(new VKIGraphicsPipeline());
	mPipeline->SetDescriptorLayout(mDescLayout.get());
}


RenderShader::~RenderShader()
{

}


void RenderShader::SetRenderPass(VKIRenderPass* pass)
{
	mRenderPass = pass;
}


void RenderShader::SetShader(ERenderShaderStage stage, const std::string& srcPath)
{
	CHECK(!mPipeline->IsValid());
	mPipeline->SetShader(ToStageBits(stage), srcPath);
}


void RenderShader::SetDomain(ERenderShaderDomain domain)
{
	CHECK(!mPipeline->IsValid());
	mDomain = domain;
}


void RenderShader::Create()
{
	CHECK(!mDescLayout->IsValid() && !mPipeline->IsValid());
	Renderer* rd = Application::Get().GetRenderer();

	SetupPipelineDomain();

	// Layout...
	mDescLayout->CreateLayout(rd->GetVKDevice());
	mPipeline->SetDescriptorLayout(mDescLayout.get());

	// Pipeline...
	mPipeline->CreatePipeline(rd->GetVKDevice(), mRenderPass);

}


void RenderShader::Destroy()
{
	if (mDescriptorSet && mDescriptorSet->IsValid())
		mDescriptorSet->Destroy();

	// Destroy layout if valid.
	if (mDescLayout->IsValid())
		mDescLayout->Destroy();

	// Destroy Pipeline if valid.
	if (mPipeline->IsValid())
		mPipeline->Destroy();

}


void RenderShader::AddInput(uint32_t binding, ERenderShaderInputType inputType, ERenderShaderStage stages)
{
	AddInput(binding, inputType, stages, 1);
}


void RenderShader::AddInput(uint32_t binding, ERenderShaderInputType inputType, ERenderShaderStage stages, uint32_t count)
{
	switch (inputType)
	{
	case ERenderShaderInputType::None:
		CHECK(0 && "Invalid Type.");
		break;

	case ERenderShaderInputType::ImageSampler:
		mDescLayout->AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ToStageBits(stages), count);
		break;

	case ERenderShaderInputType::Uniform:
		mDescLayout->AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ToStageBits(stages), count);
		break;

	case ERenderShaderInputType::DynamicUniform:
		mDescLayout->AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, ToStageBits(stages), count);
		break;
	}
}


void RenderShader::CleareInput()
{
	mDescLayout->ClearBindings();
}


void RenderShader::SetupPipelineDomain()
{
	mPipeline->SetPolygonMode(VK_POLYGON_MODE_FILL);
	mPipeline->SetLineWidth(1.0f);
	mPipeline->SetCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	mPipeline->SetDepthOp(VK_COMPARE_OP_LESS);
	mPipeline->SetStencil(false);


	// Per-Domain properties
	switch (mDomain)
	{
	case ERenderShaderDomain::Mesh:
	{
		mPipeline->SetVertexInput(0, 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MeshVert, position));
		mPipeline->SetVertexInput(1, 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MeshVert, normal));
		mPipeline->SetVertexInput(2, 0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(MeshVert, texCoord));
		mPipeline->SetVertexBinding(0, 0, sizeof(MeshVert));
		mPipeline->SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	}
		break;

	case ERenderShaderDomain::Screen:
	{
		mPipeline->ClearVertexInput();
		mPipeline->SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	}
		break;

	default:
		CHECK(0 && "Not Supported.");
		break;
	}


}


void RenderShader::SetViewport(const glm::ivec4& viewport)
{
	SetViewport(viewport.x, viewport.y, viewport.z, viewport.w);
}


void RenderShader::SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	VkViewport viewport;
	viewport.x = (float)x;
	viewport.y = (float)y;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissors;
	scissors.offset = { x, y };
	scissors.extent = { width, height };

	mPipeline->SetViewport(viewport, scissors);
}


void RenderShader::SetViewportDynamic(bool value)
{
	mIsDynamicViewport = value;

	if (mIsDynamicViewport)
	{
		mPipeline->AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		mPipeline->AddDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	}
	else
	{
		mPipeline->RemoveDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
		mPipeline->RemoveDynamicState(VK_DYNAMIC_STATE_SCISSOR);
	}

}


void RenderShader::Resize(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	CHECK(mPipeline->IsValid());

	SetViewport(x, y, width, height);
	Recreate();
}


void RenderShader::Recreate()
{
	mPipeline->Recreate();
}


void RenderShader::SetDepth(bool isEnabled, bool isWrite)
{
	mPipeline->SetDepth(isEnabled, isWrite);
}


void RenderShader::SetBlendingEnabled(uint32_t index, bool isEnabled)
{
	mPipeline->SetBlendingEnabled(index, isEnabled, VKI_COLOR_ALL_COMPONENTS);

}


void RenderShader::SetBlending(uint32_t index, ERenderBlendFactor src, ERenderBlendFactor dst, ERenderBlendOp op)
{
	VkBlendFactor vksrc = (VkBlendFactor)src, vkdst = (VkBlendFactor)dst;
	VkBlendOp vkop = (VkBlendOp)op;
	mPipeline->SetBlending(index, vksrc, vkdst, vkop);
}


void RenderShader::Bind(VKICommandBuffer* cmdBuffer) const
{
	vkCmdBindPipeline(cmdBuffer->GetCurrent(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->Get());
}


VKIDescriptorSet* RenderShader::CreateDescriptorSet()
{
	CHECK(!mDescriptorSet && "Can't Recreate the DescriptorSet.");
	mDescriptorSet = UniquePtr<VKIDescriptorSet>(new VKIDescriptorSet());
	return mDescriptorSet.get();
}


void RenderShader::AddPushConstant(uint32_t index, uint32_t offset, uint32_t size, ERenderShaderStage stage)
{
	mPipeline->SetPushConstant(index, offset, size, ToStageBits(stage));
}
