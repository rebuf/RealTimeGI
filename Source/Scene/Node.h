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
#include "Core/Transform.h"


#include <string>




class Scene;


// Node Types that are part of the scene.
enum class ENodeType
{
	// Default.
	Node,

	// Mesh Node.
	MeshNode,

	// Light Node
	Light,

	// Light Probe.
	LightProbe,

	// Irradiance Volume.
	IrradianceVolume
};






// Node:
//   - Parent class for all nodes in the scene graph.
//   - Contain the transformation as a matrix.
//
class Node
{
	// Friend...
	friend class Scene;

public:
	// Construct.
	Node();

	// Destruct.
	~Node();

	// Set/Get node transformation.
	void SetTransform(const Transform& mtx);
	inline const Transform& GetTransform() const { return mTransform; }

	// Set node transformation per component.
	void SetTranslate(const glm::vec3& translate);
	void SetScale(const glm::vec3& scale);
	void SetRotate(const glm::quat& rotate);

	// Return the type of the node.
	inline ENodeType GetType() { return mType; }

	// Return this nodes bounds.
	virtual Box GetBounds() const;

	// Set/Get Selected Flag.
	inline void SetSelected(bool val) { mIsSelected = val; }
	inline bool IsSelected() { return mIsSelected; }

protected:
	// Called when the node has been added to the scene.
	virtual void OnAdd(Scene* scene);

	// Called when the node has been removed from the scene.
	virtual void OnRemove(Scene* scene);

	// Called when the node transform changes.
	virtual void OnTransform();

protected:
	// The Node Type.
	ENodeType mType;

private:
	// The node name.
	std::string mName;

	// The node local transformation.
	Transform mTransform;

	// The Index of this node in the scene.
	uint32_t mIndexInScene;

	// True if the probe current selcted & active.
	bool mIsSelected;
};


