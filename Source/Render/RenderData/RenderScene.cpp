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





#include "RenderScene.h"
#include "Core/Mesh.h"
#include "Core/Material.h"
#include "Application.h"

#include "Scene/Scene.h"
#include "Scene/MeshNode.h"
#include "Scene/LightProbeNode.h"
#include "Scene/IrradianceVolumeNode.h"

#include "Render/Renderer.h"
#include "Render/RendererPipeline.h"
#include "Render/RenderStageLightProbes.h"
#include "Render/RenderData/RenderShadow.h"
#include "Render/RenderData/Primitives/RenderMesh.h"
#include "Render/RenderData/Primitives/RenderSphere.h"
#include "Render/RenderData/Shaders/RenderMaterial.h"
#include "Render/RenderData/Shaders/RenderShader.h"
#include "Render/RenderData/Shaders/RenderUniform.h"
#include "Render/RenderData/Shaders/RenderShaderBlocks.h"
#include "Render/RenderData/RenderLight.h"


#include "Render/VKInterface/VKICommandBuffer.h"
#include "Render/VKInterface/VKIDescriptor.h"
#include "Render/VKInterface/VKIGraphicsPipeline.h"




#define MAX_NUM_MATERIAL_UNIFORMS 512








void RDEnvironment::Reset()
{
	sunDir = glm::vec4(0.0f);
	sunColorAndPower = glm::vec4(0.0f);
	mSelectedLightProbe = nullptr;
	isLightProbeEnabled = false;
	isLightProbeHelpers = false;
	isLightProbeVisualize = false;
}



// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 





RenderScene::RenderScene()
	: mScene(nullptr)
	, mHasDirtyLightProbe(false)
	, mHasDirtyIrradianceVolume(false)
	, mDynamicMatDataCount(0)
{

}


RenderScene::~RenderScene()
{

}


void RenderScene::Initialize()
{
	Renderer* renderer = Application::Get().GetRenderer();

	mTransformUniform = UniquePtr<RenderUniform>(new RenderUniform());
	mTransformUniform->Create(renderer, sizeof(GUniform::CommonBlock), true);

	uint32_t matUniformSize = ALIGN_SIZE(sizeof(MaterialData), 64) * MAX_NUM_MATERIAL_UNIFORMS;
	mMaterialUniform = UniquePtr<RenderUniform>(new RenderUniform());
	mMaterialUniform->Create(renderer, matUniformSize, true);
	mDynamicMatData.resize(matUniformSize);

	mRSphere = UniquePtr<RenderSphere>(new RenderSphere());
	mRSphere->UpdateData(8);

	CreateSunData();

}

void RenderScene::CreateSunData()
{
	Renderer* renderer = Application::Get().GetRenderer();
	RendererPipeline* rpipeline = Application::Get().GetRenderer()->GetPipeline();

	mSunShadow = UniquePtr<RenderDirShadow>(new RenderDirShadow());
	mSunShadow->SetSize(glm::ivec2(2048, 2048));
	mSunShadow->Create();


	// Sun Pass Descriptor Set...
	{
		mSunLightingSet = UniquePtr<VKIDescriptorSet>(new VKIDescriptorSet());
		mSunLightingSet->SetLayout(rpipeline->GetSunLightingShader()->GetLayout());
		mSunLightingSet->CreateDescriptorSet(renderer->GetVKDevice(), Renderer::NUM_CONCURRENT_FRAMES);

		rpipeline->AddGBufferToDescSet(mSunLightingSet.get());

		mSunLightingSet->AddDescriptor(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			VK_SHADER_STAGE_FRAGMENT_BIT, mSunShadow->GetView(), mSunShadow->GetSampler());

		mSunLightingSet->UpdateSets();
	}

}


void RenderScene::Destroy()
{
	mTransformUniform->Destroy();
	mMaterialUniform->Destroy();
	mSunShadow->Destroy();
	mSunLightingSet->Destroy();
	mRSphere.reset();
}


void RenderScene::BuildRenderScene(Scene* scene)
{
	// View...
	CollectSceneView(scene);

	// Lights
	CollectSceneLights(scene);

	// Collect Renderable Nodes.
	TraverseScene(scene);

}


void RenderScene::Reset()
{
	mScene = nullptr;
	mHasDirtyLightProbe = false;
	mHasDirtyIrradianceVolume = false;
	mEnvironment.Reset();
	mLightProbes.clear();
	mIrradianceVolumes.clear();
	mDynamicMatDataCount = 0;


	for (size_t i = 0; i < mPrimitives.size(); ++i)
		delete mPrimitives[i];
	mPrimitives.clear();

	for (size_t i = 0; i < mPrimitivesHelpers.size(); ++i)
		delete mPrimitivesHelpers[i];
	mPrimitivesHelpers.clear();
}


RDScenePrimitive* RenderScene::AddNewPrimitive(IRenderPrimitives* primitive, const glm::mat4& transform)
{
	RDScenePrimitive* rdPrim = new RDScenePrimitive();
	rdPrim->primitive = primitive;
	rdPrim->transform = transform;
	mPrimitives.emplace_back(rdPrim);

	return rdPrim;
}


RDScenePrimitiveHelper* RenderScene::AddNewHelper(IRenderPrimitives* primitive, const glm::vec4& pos,
	const glm::vec4& scale, const glm::vec4& color)
{
	RDScenePrimitiveHelper* rdPrim = new RDScenePrimitiveHelper();
	rdPrim->primitive = primitive;
	rdPrim->position = pos;
	rdPrim->scale = scale;
	rdPrim->color = color;
	mPrimitivesHelpers.emplace_back(rdPrim);

	return rdPrim;
}


void RenderScene::CollectSceneView(Scene* scene)
{
	glm::mat4 view = scene->GetCamera().GetViewTransform();
	glm::mat4 proj = scene->GetCamera().GetProjection();
	mViewProj = proj * view;
	mViewProjInv = glm::inverse(mViewProj);
	scene->GetCamera().GetZPlane(mNearFar.x, mNearFar.y);

	mViewPos = scene->GetCamera().GetViewPos();
	mViewDir = scene->GetCamera().GetViewDir();
}


void RenderScene::AddLightProbe(Node* node)
{
	if (!mEnvironment.isLightProbeEnabled)
		return;

	LightProbeNode* probe = static_cast<LightProbeNode*>(node);
	RenderLightProbe* rprobe = probe->GetRenderLightProbe();

	// Not Invalid LightProbe?
	if (rprobe->GetDirty() != INVALID_INDEX)
	{
		mLightProbes.emplace_back(rprobe);
		mHasDirtyLightProbe = mHasDirtyLightProbe || rprobe->GetDirty() != 0;
	}


	if (mEnvironment.isLightProbeHelpers)
	{
		AddNewHelper(mRSphere.get(),
			glm::vec4(rprobe->GetPosition(), 0.0f),
			glm::vec4(0.1f),
			probe->IsSelected() ? glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		if (probe->IsSelected())
		{
			mEnvironment.mSelectedLightProbe = rprobe;

			AddNewHelper(mRSphere.get(),
				glm::vec4(rprobe->GetPosition(), 1.0f),
				glm::vec4(rprobe->GetRadius() * 0.02f),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		}
	}

}


void RenderScene::AddIrradianceVolume(Node* node)
{
	if (!mEnvironment.isLightProbeEnabled)
		return;


	IrradianceVolumeNode* irrVolume = static_cast<IrradianceVolumeNode*>(node);
	RenderIrradianceVolume* rVolume = irrVolume->GetRenderIrradianceVolume();

	// Not Invalid Irradiance Volume?
	if (rVolume->GetDirty() != INVALID_INDEX)
	{
		mIrradianceVolumes.emplace_back(rVolume);
		mHasDirtyIrradianceVolume = mHasDirtyIrradianceVolume || rVolume->GetDirty() != 0;
	}


	if (mEnvironment.isLightProbeHelpers)
	{
		uint32_t np = rVolume->GetNumProbes();

		// Iterate over all probes in the volume.
		for (uint32_t iP = 0; iP < np; ++iP)
		{
			glm::vec3 pos = rVolume->GetProbePosition(iP);

			AddNewHelper(mRSphere.get(), glm::vec4(pos, 0.0f), glm::vec4(0.1f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
		}
	}
}


void RenderScene::CollectSceneLights(Scene* scene)
{
	// The Sun...
	glm::vec3 sunDir = scene->GetGlobal().GetSunDir();
	mEnvironment.sunDir = glm::vec4(sunDir, 0.0f);

	mEnvironment.sunColorAndPower = glm::vec4(scene->GetGlobal().GetSunColor(),
		scene->GetGlobal().GetSunPower());

	glm::mat4 sunView = Transform::LookAt(-sunDir, glm::vec3(0.0f), abs(sunDir.z) < 0.9 ? Transform::UP : Transform::FORWARD * glm::sign(sunDir.z));
	glm::mat4 sunProj = Transform::Ortho(1200.0f, -1200.0f, 1200.0f, -1200.0f, -10000.0f, 2000.0f);
	mSunShadow->SetShadowMatrix(sunProj * sunView);

	// Flag sun shadow as dirty.
	mSunShadow->SetDirty(scene->GetGlobal().HasDirtyFlag(ESceneGlobalDirtyFlag::DirtySun));
	scene->GetGlobal().ClearDirtyFlag(ESceneGlobalDirtyFlag::DirtySun);


	mEnvironment.isLightProbeEnabled = scene->GetGlobal().isLightProbeEnabled;
	mEnvironment.isLightProbeHelpers = scene->GetGlobal().isLightProbeHelpers;
	mEnvironment.isLightProbeVisualize = scene->GetGlobal().isLightProbeVisualize;


	// Lights & Light Probes...
	for (auto& node : scene->GetLights())
	{
		switch (node->GetType())
		{
		case ENodeType::LightProbe:
			AddLightProbe(node);
			break;

		case ENodeType::IrradianceVolume:
			AddIrradianceVolume(node);
			break;
		}
	}


}


void RenderScene::TraverseScene(Scene* scene)
{
	// Collect Render Primitives...
	for (const auto& node : scene->GetRenderable())
	{
		const Transform& tr = node->GetTransform();

		// Based Renderable Type...
		switch (node->GetType())
		{
		case ENodeType::MeshNode:
		{
			const MeshNode* meshNode = static_cast<const MeshNode*>(node);

			for (uint32_t i = 0; i < meshNode->GetNumMeshes(); ++i)
			{
				Mesh* mesh = meshNode->GetMesh(i);

				// New...
				IRenderPrimitives* primitive = mesh->GetRenderMesh();
				auto newPrim = AddNewPrimitive(primitive, tr.GetMatrix());

				//
				newPrim->materail = meshNode->GetMaterial(i)->GetRenderMaterial();
				newPrim->materail->mDynamicOffset = (int32_t)mDynamicMatDataCount;
				*((MaterialData*)(mDynamicMatData.data() + ALIGN_SIZE(sizeof(MaterialData), 64) * mDynamicMatDataCount)) = *newPrim->materail->mMatData;

				++mDynamicMatDataCount;
			}
		}
			break;

		} // End of Node Type Switch.

	} // End of Renderable loop.

}


void RenderScene::UpdateUniforms(uint32_t frame)
{
	if (!mDynamicMatData.empty())
	{
		mMaterialUniform->Update(frame, 0,
			mDynamicMatDataCount * ALIGN_SIZE(sizeof(MaterialData), 64),
			mDynamicMatData.data());
	}

}


void RenderScene::DrawSceneDeferred(VKICommandBuffer* cmdBuffer, uint32_t frame)
{
	RenderShader* shader = RenderMaterial::GetShader(ERenderMaterialType::Opaque);
	shader->Bind(cmdBuffer);

	for (uint32_t i = 0; i < mPrimitives.size(); ++i)
	{
		mPrimitives[i]->materail->Bind(cmdBuffer, frame);
		mPrimitives[i]->primitive->Draw(cmdBuffer);
	}

}


void RenderScene::DrawSceneShadow(VKICommandBuffer* cmdBuffer, uint32_t frame, IRenderShadow* shadow)
{
	RenderShader* shader = RenderMaterial::GetDirShadowShader(ERenderMaterialType::Opaque);
	shader->Bind(cmdBuffer);
	shader->GetDescriptorSet()->Bind(cmdBuffer, frame, shader->GetPipeline());

	// Shadow Input Constants...
	GUniform::ShadowConstantBlock shadowConstant;
	shadowConstant.shadowMatrix = shadow->GetShadowMatrix();
	shadowConstant.lightPos = glm::vec4(shadow->GetLightPos(), 1.0f);

	vkCmdPushConstants(cmdBuffer->GetCurrent(), 
		shader->GetPipeline()->GetLayout(),
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		0, sizeof(GUniform::ShadowConstantBlock), &shadowConstant);


	for (uint32_t i = 0; i < mPrimitives.size(); ++i)
	{
		mPrimitives[i]->primitive->Draw(cmdBuffer);
	}

}


void RenderScene::DrawHelpers(VKICommandBuffer* cmdBuffer, uint32_t frame)
{
	if (mPrimitivesHelpers.empty())
		return;

	RenderShader* shader = RenderMaterial::SPHERE_HELPER_SHADER.get();
	shader->Bind(cmdBuffer);
	shader->GetDescriptorSet()->Bind(cmdBuffer, frame, shader->GetPipeline());

	GUniform::SphereHelperBlock helperBlock;

	for (uint32_t i = 0; i < mPrimitivesHelpers.size(); ++i)
	{
		helperBlock.position = mPrimitivesHelpers[i]->position;
		helperBlock.scale = mPrimitivesHelpers[i]->scale;
		helperBlock.color = mPrimitivesHelpers[i]->color;

		vkCmdPushConstants(cmdBuffer->GetCurrent(), shader->GetPipeline()->GetLayout(),
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(helperBlock), &helperBlock);

		mPrimitivesHelpers[i]->primitive->Draw(cmdBuffer);
	}
}
