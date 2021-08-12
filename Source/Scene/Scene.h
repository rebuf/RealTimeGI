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




#pragma once



#include "Core/Core.h"
#include "Core/Box.h"
#include "SceneGlobalSettings.h"
#include "Camera.h"

#include <vector>




class Node;








// Data set/used by the scene for each node.
struct NodeSceneData
{
	// The NODE.
	Ptr<Node> node;

	// The index of the node in the renderable list.
	uint32_t renderIndex;

	// The index of the node in the lights list.
	uint32_t lightIndex;

	// Construct.
	NodeSceneData()
		: renderIndex(INVALID_UINDEX)
		, lightIndex(INVALID_UINDEX)
	{

	}

};




// Scene:
//   - Manage the entire scene graph.
//
class Scene
{
public:
	// Construct.
	Scene();

	// Destruct.
	~Scene();

	// Add a new node to the scene.
	void AddNode(Ptr<Node> node);

	// Remove node from the scene.
	void RemoveNode(Node* node);

	// Start the scene, called the first time the scene has started.
	void Start();

	// Called by the application to destroy the scene and all its content.
	void Destroy();

	// Updat scene.
	void Update(float deltaTime);

	// Return all renderable nodes in the scene.
	const std::vector<Node*>& GetRenderable() const { return mRenderable; }
	
	// Return all light nodes in the scene.
	const std::vector<Node*>& GetLights() const { return mLights; }

	// The Scene Camera.
	inline Camera& GetCamera() { return mCamera; }
	inline const Camera& GetCamera() const { return mCamera; }

	// Return Scene Global Settings.
	inline SceneGlobalSettings& GetGlobal() { return mGlobal; }
	inline const SceneGlobalSettings& GetGlobal() const { return mGlobal; }

	// Reset camera view to look at the entire scene.
	void ResetView();

	// Compute Scene Bounding Box.
	Box ComputeBounds();

	// Return cached bounds.
	inline const Box& GetBounds() const { return mBounds; }


	//
	void SetSelectedLight(Node* node);

	//
	void UnselectLight();


private:
	// Register node to the scene and based on its type
	void RegisterNode(Node* node);

	// Unregister node from the scene and based on its type.
	void UnregisterNode(Node* node);

	// Add/Remove node from Renderable list.
	void AddRenderable(Node* node);
	void RemoveRenderable(Node* node);

	// Add/Remove node from Lights list.
	void AddLight(Node* node);
	void RemoveLight(Node* node);

private:
	// All the nodes in the scene.
	std::vector<NodeSceneData> mSceneNodes;

	// True if the scene started.
	bool mHasStarted;

	// True if the scene was destroyed.
	bool mIsDestroyed;

	// List of all renderable nodes.
	std::vector<Node*> mRenderable;

	// List of all lights in the scene.
	std::vector<Node*> mLights;

	// The scene main camera.
	Camera mCamera;

	// Global Settings.
	SceneGlobalSettings mGlobal;

	//
	Node* mSelectedLight;

	// The Cachced scene bounding box.
	Box mBounds;
};


