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
#include "Node.h"
#include "Core/Box.h"




class RenderLightProbe;








// LightProbeNode:
//    - 
//
class LightProbeNode : public Node
{
public:
	// Construct.
	LightProbeNode();

	// Destruct.
	~LightProbeNode();

	// Set/Get Probe Radius.
	void SetRadius(float radius);
	inline float GetRadius() const { return mRadius; }

	// Return Probe position.
	glm::vec3 GetPosition() const;

	// Return the render light probe.
	inline RenderLightProbe* GetRenderLightProbe() { return mRenderLightProbe.get(); }

	// Create/Update render light probe data.
	void UpdateRenderLightProbe();

	// Set/Get Selected Flag.
	inline void SetSelected(bool val) { mIsSelected = val; }
	inline bool IsSelected() { return mIsSelected; }

protected:
	// Called when the node transform changes.
	virtual void OnTransform() override;

private:
	// The Probe Influence Radiuss.
	float mRadius;

	// The render data for this light probe.
	UniquePtr<RenderLightProbe> mRenderLightProbe;

	// True if the probe current selcted & active.
	bool mIsSelected;
};



