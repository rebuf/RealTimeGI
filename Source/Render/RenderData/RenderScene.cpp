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
#include "Application.h"
#include "Render/Renderer.h"
#include "Render/RendererPipeline.h"
#include "Scene/Scene.h"
#include "Scene/MeshNode.h"

#include "Render/Renderer.h"
#include "Render/RenderData/RenderShadow.h"
#include "Render/RenderData/Primitives/RenderMesh.h"
#include "Render/RenderData/Shaders/RenderMaterial.h"
#include "Render/RenderData/Shaders/RenderShader.h"
#include "Render/RenderData/Shaders/RenderUniform.h"
#include "Render/RenderData/Shaders/RenderShaderBlocks.h"

#include "Render/VKInterface/VKICommandBuffer.h"
#include "Render/VKInterface/VKIDescriptor.h"
#include "Render/VKInterface/VKIGraphicsPipeline.h"







void RDEnvironment::Reset()
{
	sunDir = glm::vec4(0.0f);
	sunColorAndPower = glm::vec4(0.0f);
}



// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 





RenderScene::RenderScene()
	: mScene(nullptr)
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


	mSunShadow = UniquePtr<RenderDirShadow>(new RenderDirShadow());
	mSunShadow->SetSize(glm::ivec2(1024, 1024));
	mSunShadow->Create();

}


void RenderScene::Destroy()
{
	mTransformUniform->Destroy();
	mSunShadow->Destroy();
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
	mEnvironment.Reset();

	for (size_t i = 0; i < mPrimitives.size(); ++i)
		delete mPrimitives[i];
	mPrimitives.clear();
}


RDScenePrimitive* RenderScene::AddNewPrimitive(IRenderPrimitives* primitive, const glm::mat4& transform)
{
	RDScenePrimitive* rdPrim = new RDScenePrimitive();
	rdPrim->primitive = primitive;
	rdPrim->transform = transform;
	mPrimitives.emplace_back(rdPrim);

	return rdPrim;
}


void RenderScene::CollectSceneView(Scene* scene)
{
	glm::mat4 view = scene->GetCamera().GetViewTransform();
	glm::mat4 proj = scene->GetCamera().GetProjection();
	mViewProj = proj * view;
	mViewProjInv = glm::inverse(mViewProj);
	scene->GetCamera().GetZPlane(mNearFar.x, mNearFar.y);

	mViewPos = view * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	mViewDir = view * glm::vec4(Transform::FORWARD, 0.0f);
}


void RenderScene::CollectSceneLights(Scene* scene)
{
	// The Sun.
	mEnvironment.sunColorAndPower.r = scene->GetGlobalSettings().sunColor.r;
	mEnvironment.sunColorAndPower.g = scene->GetGlobalSettings().sunColor.g;
	mEnvironment.sunColorAndPower.b = scene->GetGlobalSettings().sunColor.b;
	mEnvironment.sunColorAndPower.a = scene->GetGlobalSettings().sunPower;

	glm::vec3 sunDir = scene->GetGlobalSettings().sunDir;
	mEnvironment.sunDir = glm::vec4(sunDir, 0.0f);

	glm::mat4 sunView = Transform::LookAt(sunDir * -10000.0f, glm::vec3(0.0f), Transform::UP);
	glm::mat4 sunProj = Transform::Ortho(1000, -1000, 1000, -1000, 1.0f, 10000.0f);
	mSunShadow->SetShadowMatrix(sunProj * sunView);

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
				AddNewPrimitive(primitive, tr.GetMatrix());
			}
		}
			break;

		} // End of Node Type Switch.

	} // End of Renderable loop.



	// Collect Lights...
	for (const auto& light : scene->GetLights())
	{
		// TODO Lights....
	}

}


void RenderScene::DrawSceneDeferred(VKICommandBuffer* cmdBuffer, uint32_t frame)
{
	RenderShader* shader = RenderMaterial::GetShader(ERenderMaterialType::Opaque);
	shader->Bind(cmdBuffer);

	RenderMaterial::MAT_DESC_SET[0]->Bind(cmdBuffer, frame, shader->GetPipeline());

	for (uint32_t i = 0; i < mPrimitives.size(); ++i)
	{
		mPrimitives[i]->primitive->Draw(cmdBuffer);
	}

}


void RenderScene::DrawSceneShadow(VKICommandBuffer* cmdBuffer, uint32_t frame, IRenderShadow* shadow)
{
	RenderShader* shader = RenderMaterial::GetDirShadowShader(ERenderMaterialType::Opaque);
	shader->Bind(cmdBuffer);
	RenderMaterial::MAT_DESC_SET[1]->Bind(cmdBuffer, frame, shader->GetPipeline());

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
