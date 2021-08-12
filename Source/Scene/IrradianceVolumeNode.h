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




class RenderIrradianceVolume;








// LightProbeNode:
//    - 
//
class IrradianceVolumeNode : public Node
{
public:
	// Construct.
	IrradianceVolumeNode();

	// Destruct.
	~IrradianceVolumeNode();

	// Set/Get Probe Radius.
	void SetVolume(const glm::vec3& start, const glm::vec3& extent, const glm::ivec3& count);
	void GetVolume(glm::vec3& start, glm::vec3& extent, glm::ivec3& count) const;

	// Return the render light probe.
	inline RenderIrradianceVolume* GetRenderIrradianceVolume() { return mRenderIrradianceVolume.get(); }

	// Create/Update render light probe data.
	void UpdateIrradianceVolumeNode();

	// Bounds.
	virtual Box GetBounds() const override;

	// Set light probe dirty to get updated.
	void SetDirty();

	// Set/Get Volume Attenuation.
	inline void SetAtten(const glm::vec3& atten) { mAtten = atten; }
	inline glm::vec3 GetAtten() const { return mAtten; }

protected:
	// Called when the node transform changes.
	virtual void OnTransform() override;

private:
	// The render data for this irradiance volume.
	UniquePtr<RenderIrradianceVolume> mRenderIrradianceVolume;

	// Volume Start.
	glm::vec3 mStart;

	// Volume Extent.
	glm::vec3 mExtent;
	
	// Volume Count.
	glm::ivec3 mCount;

	// Volume Attenuation.
	glm::vec3 mAtten;
};



