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




#include "Scene.h"
#include "Core/Box.h"
#include "Application.h"
#include "AppWindow.h"
#include "Node.h"
#include "LightProbeNode.h"





Scene::Scene()
	: mHasStarted(false)
	, mIsDestroyed(false)
	, mSelectedLight(nullptr)
{

}


Scene::~Scene()
{

}


void Scene::AddNode(Ptr<Node> node)
{
	CHECK(node->mIndexInScene == INVALID_UINDEX && "Already in the scene.");
	node->mIndexInScene = (uint32_t)mSceneNodes.size(); // End Index.

	// Add to the back.
	NodeSceneData data;
	data.node = node;
	mSceneNodes.emplace_back(data);

	// Register based on type.
	RegisterNode(node.get());

	// Add Event.
	node->OnAdd(this);
}


void Scene::RemoveNode(Node* node)
{
	CHECK(node && node->mIndexInScene != INVALID_UINDEX && "Invalid Node.");

	// Remove Event.
	node->OnRemove(this);

	if (mSceneNodes.size() == 1)
	{
		mSceneNodes.clear();
	}
	else
	{
		mSceneNodes.back().node->mIndexInScene = node->mIndexInScene;
		mSceneNodes[node->mIndexInScene] = mSceneNodes.back();
		mSceneNodes.pop_back();
	}

	// Unregister based on type.
	UnregisterNode(node);

}


void Scene::Start()
{
	mHasStarted = true;

}


void Scene::Destroy()
{
	mIsDestroyed = true;

}


void Scene::Update(float deltaTime)
{
	// Update Camera Aspect.
	float aspect = Application::Get().GetMainWindow()->GetFrameBufferAspect();
	mCamera.SetAspect(aspect);


}


void Scene::RegisterNode(Node* node)
{
	switch (node->GetType())
	{
	case ENodeType::Node:
		//...
		break;

	case ENodeType::MeshNode:
		AddRenderable(node);
		break;

	case ENodeType::LightProbe:
		AddLight(node);
		break;
	}
}


void Scene::UnregisterNode(Node* node)
{
	switch (node->GetType())
	{
	case ENodeType::Node:
		//...
		break;

	case ENodeType::MeshNode:
		RemoveRenderable(node);
		break;

	case ENodeType::LightProbe:
		RemoveLight(node);
		break;
	}
}


void Scene::AddRenderable(Node* node)
{
	NodeSceneData& data = mSceneNodes[node->mIndexInScene];
	data.renderIndex = (uint32_t)mRenderable.size();
	mRenderable.emplace_back(node);
}


void Scene::RemoveRenderable(Node* node)
{
	NodeSceneData& data = mSceneNodes[node->mIndexInScene];

	if (mRenderable.size() > 1)
	{
		Node* rp = mRenderable.back();
		NodeSceneData& rpdata = mSceneNodes[rp->mIndexInScene];
		rpdata.renderIndex = data.renderIndex;

		mRenderable[rpdata.renderIndex] = rp;
	}

	mRenderable.pop_back();
}


void Scene::AddLight(Node* node)
{
	NodeSceneData& data = mSceneNodes[node->mIndexInScene];
	data.lightIndex = (uint32_t)mLights.size();
	mLights.emplace_back(node);
}


void Scene::RemoveLight(Node* node)
{
	NodeSceneData& data = mSceneNodes[node->mIndexInScene];

	if (mLights.size() > 1)
	{
		Node* rp = mLights.back();
		NodeSceneData& rpdata = mSceneNodes[rp->mIndexInScene];
		rpdata.lightIndex = data.lightIndex;

		mLights[rpdata.lightIndex] = rp;
	}

	mLights.pop_back();
}


void Scene::ResetView()
{
	Box bounds = ComputeBounds();
	if (!bounds.IsValid())
	{
		mCamera.SetViewTarget(glm::vec3(0.0f));
		mCamera.SetViewPos(glm::vec3(5.0f));
		return;
	}


	glm::vec3 center = bounds.Center();
	glm::vec3 offset = bounds.Extent();

	mCamera.SetViewTarget(center - glm::vec3(0.0f, 0.0f, 100.0f));
	mCamera.SetViewPos(center + glm::vec3(-550.0f, -50.0f, 70.0f));
	mCamera.RecomputeUp();
}


Box Scene::ComputeBounds()
{
	Box bounds;

	for (size_t i = 0; i < mSceneNodes.size(); ++i)
		bounds.Add(mSceneNodes[i].node->GetBounds());

	return bounds;
}


void Scene::SetSelectedLight(Node* node)
{
	UnselectLight();
	mSelectedLight = node;

	if (!mSelectedLight)
		return;

	if (mSelectedLight->GetType() == ENodeType::LightProbe)
	{
		LightProbeNode* probe = static_cast<LightProbeNode*>(mSelectedLight);
		probe->SetSelected(true);
	}
}


void Scene::UnselectLight()
{
	if (!mSelectedLight)
		return;

	if (mSelectedLight->GetType() == ENodeType::LightProbe)
	{
		LightProbeNode* probe = static_cast<LightProbeNode*>(mSelectedLight);
		probe->SetSelected(false);
	}


	mSelectedLight = nullptr;
}
