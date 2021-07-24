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



#include "VKIGraphicsPipeline.h"
#include "VKIDevice.h"
#include "VKIRenderPass.h"
#include "VKIDescriptor.h"



#include <fstream>





VKIGraphicsPipeline::VKIGraphicsPipeline()
	: mVKDevice(nullptr)
  , mRenderPass(nullptr)
	, mHandle(VK_NULL_HANDLE)
  , mSubpass(0)
  , mTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
  , mDescriptorLayout(nullptr)
  , mLayout(VK_NULL_HANDLE)
{
  mViewport = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
  mScissors = { { 0, 0 }, { 0, 0} };

  mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  mRasterizer.cullMode = VK_CULL_MODE_NONE;
  mRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  mRasterizer.lineWidth = 1.0f;

  mDepthStencil.isDepth = VK_FALSE;
  mDepthStencil.isDepthWrite = VK_FALSE;
  mDepthStencil.compareOp = VK_COMPARE_OP_LESS;

  VkStencilOpState stencilOpDefault{};
  mDepthStencil.stencilOpFront = stencilOpDefault;
  mDepthStencil.stencilOpBack = stencilOpDefault;
  mDepthStencil.isStencil = VK_FALSE;

}


VKIGraphicsPipeline::~VKIGraphicsPipeline()
{

}


void VKIGraphicsPipeline::SetShader(VkShaderStageFlagBits stage, const std::string& src)
{
  SetShader(stage, src, "main");
}


void VKIGraphicsPipeline::SetShader(VkShaderStageFlagBits stage, const std::string& src, const std::string& entry)
{
  VKIGfxPipelineSource gfxSource;
  gfxSource.stage = stage;
  gfxSource.source = src;
  gfxSource.entry = entry;
  mSource[stage] = gfxSource;
}


void VKIGraphicsPipeline::SetVertexInput(uint32_t index, uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
  if (mVertexInput.attributes.size() < index + 1)
    mVertexInput.attributes.resize(index + 1);

  mVertexInput.attributes[index].binding = binding;
  mVertexInput.attributes[index].location = location;
  mVertexInput.attributes[index].format = format;
  mVertexInput.attributes[index].offset = offset;
}


void VKIGraphicsPipeline::SetVertexBinding(uint32_t index, uint32_t binding, uint32_t stride, bool isInstance)
{
  if (mVertexInput.bindings.size() < index + 1)
    mVertexInput.bindings.resize(index + 1);

  mVertexInput.bindings[index].binding = binding;
  mVertexInput.bindings[index].stride = stride;
  mVertexInput.bindings[index].inputRate = isInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
}


void VKIGraphicsPipeline::ClearVertexInput()
{
  mVertexInput.attributes.clear();
  mVertexInput.bindings.clear();
}


void VKIGraphicsPipeline::SetTopology(VkPrimitiveTopology topology)
{
  mTopology = topology;
}


void VKIGraphicsPipeline::SetPolygonMode(VkPolygonMode mode)
{
  mRasterizer.polygonMode = mode;
}


void VKIGraphicsPipeline::SetCulling(VkCullModeFlagBits mode, VkFrontFace face)
{
  mRasterizer.cullMode = mode;
  mRasterizer.frontFace = face;
}


void VKIGraphicsPipeline::SetLineWidth(float width)
{
  mRasterizer.lineWidth = width;
}


void VKIGraphicsPipeline::SetDepth(bool isEnabled, bool isWriteEnabled)
{
  mDepthStencil.isDepth = isEnabled ? VK_TRUE : VK_FALSE;
  mDepthStencil.isDepthWrite = isWriteEnabled ? VK_TRUE : VK_FALSE;
}


void VKIGraphicsPipeline::SetDepthOp(VkCompareOp op)
{
  mDepthStencil.compareOp = op;
}


void VKIGraphicsPipeline::SetStencil(bool isEnabled)
{
  mDepthStencil.isStencil = isEnabled ? VK_TRUE : VK_FALSE;
}


void VKIGraphicsPipeline::SetStencil(bool isEnabled, VkStencilOpState front, VkStencilOpState back)
{
  mDepthStencil.isStencil = isEnabled ? VK_TRUE : VK_FALSE;
  mDepthStencil.stencilOpFront = front;
  mDepthStencil.stencilOpBack = back;
}


void VKIGraphicsPipeline::SetBlendingEnabled(uint32_t index, bool isEnabled, VkColorComponentFlags mask)
{
  if (mBlending.size() < index + 1)
    mBlending.resize(index + 1);

  mBlending[index].isBlending = isEnabled ? VK_TRUE : VK_FALSE;
  mBlending[index].mask = mask;
}


void VKIGraphicsPipeline::SetBlending(uint32_t index, VkBlendFactor src, VkBlendFactor dst, VkBlendOp op)
{
  if (mBlending.size() < index + 1)
    mBlending.resize(index + 1);

  mBlending[index].srcColor = src;
  mBlending[index].dstColor = dst;
  mBlending[index].opColor = op;

  mBlending[index].srcAlpha = src;
  mBlending[index].dstAlpha = dst;
  mBlending[index].opAlpha = op;
}


void VKIGraphicsPipeline::SetBlending(uint32_t index, VkBlendFactor srcColor, VkBlendFactor dstColor, VkBlendOp opColor,
  VkBlendFactor srcAlpha, VkBlendFactor dstAlpha, VkBlendOp opAlpha)
{
  if (mBlending.size() < index + 1)
    mBlending.resize(index + 1);

  mBlending[index].srcColor = srcColor;
  mBlending[index].dstColor = dstColor;
  mBlending[index].opColor = opColor;

  mBlending[index].srcAlpha = srcAlpha;
  mBlending[index].dstAlpha = dstAlpha;
  mBlending[index].opAlpha = opAlpha;
}


void VKIGraphicsPipeline::SetDescriptorLayout(VKIDescriptorLayout* descriptorLayout)
{
  mDescriptorLayout = descriptorLayout;
}


void VKIGraphicsPipeline::SetPushConstant(uint32_t index, uint32_t offset, uint32_t size, VkShaderStageFlags stage)
{
  if (mPushConstant.size() < index + 1)
    mPushConstant.resize(index + 1);

  mPushConstant[index].stageFlags = stage;
  mPushConstant[index].offset = offset;
  mPushConstant[index].size = size;
}


void VKIGraphicsPipeline::ClearPushConstants()
{
  mPushConstant.clear();
}


void VKIGraphicsPipeline::CreatePipeline(VKIDevice* owner, VKIRenderPass* renderPass)
{
  CHECK(!mSource.empty() 
    && mBlending.size() == renderPass->GetNumColorAttachments()
    && "Invalid Pipeline Data.");

	VkResult result = VK_SUCCESS;
  mVKDevice = owner;
  mRenderPass = renderPass;


  // Pipeline Shaders...
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  SetupPipelineShaders(shaderStages);

  // Pipeline Vertex Input...
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)mVertexInput.attributes.size();
  vertexInputInfo.pVertexAttributeDescriptions = mVertexInput.attributes.data();
  vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)mVertexInput.bindings.size();
  vertexInputInfo.pVertexBindingDescriptions = mVertexInput.bindings.data();

  // Pipeline Assembly...
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
  inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyInfo.topology = mTopology;
  inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

  // Pipeline Viewport...
  VkPipelineViewportStateCreateInfo viewportInfo{};
  viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportInfo.viewportCount = 1;
  viewportInfo.pViewports = &mViewport;
  viewportInfo.scissorCount = 1;
  viewportInfo.pScissors = &mScissors;

  // Pipeline Rasterizer...
  VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
  rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizerInfo.depthClampEnable;
  rasterizerInfo.rasterizerDiscardEnable;
  rasterizerInfo.polygonMode = mRasterizer.polygonMode;
  rasterizerInfo.cullMode = mRasterizer.cullMode;
  rasterizerInfo.frontFace = mRasterizer.frontFace;
  rasterizerInfo.lineWidth = mRasterizer.lineWidth;
  rasterizerInfo.depthBiasEnable = VK_FALSE;
  rasterizerInfo.depthBiasConstantFactor = 0.0f;
  rasterizerInfo.depthBiasClamp = 0.0f;
  rasterizerInfo.depthBiasSlopeFactor = 0.0f;

  // Pipeline Multisampling...
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE; // WE DON'T WANT MULTISAMPLING.
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  // Pipeline Depth & Stencil...
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
  depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilInfo.depthTestEnable = mDepthStencil.isDepth;
  depthStencilInfo.depthWriteEnable = mDepthStencil.isDepthWrite;
  depthStencilInfo.depthCompareOp = mDepthStencil.compareOp;
  depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  depthStencilInfo.stencilTestEnable = mDepthStencil.isStencil;
  depthStencilInfo.front = mDepthStencil.stencilOpFront;
  depthStencilInfo.back = mDepthStencil.stencilOpBack;
  depthStencilInfo.minDepthBounds = 0.0f;
  depthStencilInfo.maxDepthBounds = 1.0f;


  // Pipeline Blending...
  std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
  blendAttachments.resize(mRenderPass->GetNumColorAttachments());

  // Create a blending state for each attachment...
  for (uint32_t i = 0; i < mRenderPass->GetNumColorAttachments(); ++i)
  {
    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.blendEnable = mBlending[i].isBlending;
    blendAttachment.srcColorBlendFactor = mBlending[i].srcColor;
    blendAttachment.dstColorBlendFactor = mBlending[i].dstColor;
    blendAttachment.colorBlendOp = mBlending[i].opColor;
    blendAttachment.srcAlphaBlendFactor = mBlending[i].srcAlpha;
    blendAttachment.dstAlphaBlendFactor = mBlending[i].dstAlpha;
    blendAttachment.alphaBlendOp = mBlending[i].opAlpha;
    blendAttachment.colorWriteMask = mBlending[i].mask;

    blendAttachments[i] = blendAttachment;
  }

  VkPipelineColorBlendStateCreateInfo blendInfo{};
  blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blendInfo.attachmentCount = (uint32_t)blendAttachments.size();
  blendInfo.pAttachments = blendAttachments.data();
  blendInfo.logicOpEnable = VK_FALSE;
  blendInfo.logicOp = VK_LOGIC_OP_COPY;
  blendInfo.blendConstants[0] = 0.0f;
  blendInfo.blendConstants[1] = 0.0f;
  blendInfo.blendConstants[2] = 0.0f;
  blendInfo.blendConstants[3] = 0.0f;


  // Pipeline Layout...
  CreatePiplineLayout();


  // Dynamic State...
  std::vector<VkDynamicState> dynamicStatesVector(mDynamicStates.size());
  std::copy(mDynamicStates.begin(), mDynamicStates.end(), dynamicStatesVector.begin());

  VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
  dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStatesVector.size();
  dynamicStateInfo.pDynamicStates = dynamicStatesVector.data();


	// Create Pipeline...
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = (uint32_t)shaderStages.size();
  pipelineInfo.pStages = shaderStages.data();
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
  pipelineInfo.pViewportState = &viewportInfo;
  pipelineInfo.pRasterizationState = &rasterizerInfo;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencilInfo;
  pipelineInfo.pColorBlendState = &blendInfo;
  pipelineInfo.pDynamicState = &dynamicStateInfo;
  pipelineInfo.layout = mLayout;
  pipelineInfo.renderPass = mRenderPass->Get();
  pipelineInfo.subpass = mSubpass;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;


	result = vkCreateGraphicsPipelines(mVKDevice->Get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mHandle);
	CHECK(result == VK_SUCCESS);



  // Destroy the shader modules they are no longer needed.
  for (auto& stage : shaderStages)
  {
    vkDestroyShaderModule(mVKDevice->Get(), stage.module, nullptr);
  }
}


void VKIGraphicsPipeline::Recreate()
{
  Destroy();
  CreatePipeline(mVKDevice, mRenderPass);
}


void VKIGraphicsPipeline::Destroy()
{
  // Destroy Lyaout.
  vkDestroyPipelineLayout(mVKDevice->Get(), mLayout, nullptr);

  // Destroy Graphics Pipeline...
  vkDestroyPipeline(mVKDevice->Get(), mHandle, nullptr);

  //...
  mHandle = VK_NULL_HANDLE;
  mLayout = VK_NULL_HANDLE;
}


void VKIGraphicsPipeline::ReadShaderFile(const std::string& filename, std::vector<uint8_t>& outCode)
{
  // Open a file for reading as binary and start from the end.
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  // Is Valid?
  if (!file.is_open())
  {
    // Empty...
    outCode.clear();
    LOGE("Can't open shader file (%s)", filename.c_str());
    return;
  }

  // Read the file...
  size_t fileSize = (size_t)file.tellg();
  outCode.resize(fileSize);

  file.seekg(0);
  file.read(reinterpret_cast<char*>(outCode.data()), fileSize);

  file.close();
}


void VKIGraphicsPipeline::SetupPipelineShaders(std::vector<VkPipelineShaderStageCreateInfo>& outPipelineShaders)
{
  // Setup all shaders added to the 
  for (const auto& srcData : mSource)
  {
    const VKIGfxPipelineSource& gfxSource = srcData.second;

    // Read source from file.
    std::vector<uint8_t> code;
    ReadShaderFile(gfxSource.source, code);

    // Structure for creating a shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();  // The size of the shader code
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());  // The shader code

    VkShaderModule shaderModule;

    VkResult result = vkCreateShaderModule(mVKDevice->Get(), &createInfo, nullptr, &shaderModule);
    CHECK(result == VK_SUCCESS && "Failed to create shader module!");

    // Structure used by the pipeline to define a shader stage and its entry point.
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = gfxSource.stage;
    vertShaderStageInfo.module = shaderModule;
    vertShaderStageInfo.pName = gfxSource.entry.c_str();
    outPipelineShaders.push_back(vertShaderStageInfo);
  }

}


void VKIGraphicsPipeline::CreatePiplineLayout()
{
  VkDescriptorSetLayout descLayout = VK_NULL_HANDLE;


  // Create Pipeline Layout...
  VkPipelineLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;


  // Has Descriptors?
  if (mDescriptorLayout)
  {
    descLayout = mDescriptorLayout->Get();
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descLayout;
  }


  // Has Push Constants?
  if (!mPushConstant.empty())
  {
    layoutInfo.pushConstantRangeCount = (uint32_t)mPushConstant.size();
    layoutInfo.pPushConstantRanges = mPushConstant.data();
  }


  // Create the pipeline layout and test if sucess.
  VkResult result = vkCreatePipelineLayout(mVKDevice->Get(), &layoutInfo, nullptr, &mLayout);
  CHECK(result == VK_SUCCESS && "failed to create pipeline layout!");
}


void VKIGraphicsPipeline::SetViewport(VkViewport viewport, VkRect2D scissors)
{
  mViewport = viewport;
  mScissors = scissors;
}


void VKIGraphicsPipeline::AddDynamicState(VkDynamicState state)
{
  mDynamicStates.insert(state);
}


void VKIGraphicsPipeline::RemoveDynamicState(VkDynamicState state)
{
  mDynamicStates.erase(state);
}
