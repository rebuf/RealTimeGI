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








#include "IrradianceVolumeNode.h"
#include "Render/RenderData/RenderLight.h"
#include "Render/RenderData/RenderTypes.h"





IrradianceVolumeNode::IrradianceVolumeNode()
	: mStart(0.0f)
	, mExtent(0.0f)
	, mCount(0)
{
	mType = ENodeType::IrradianceVolume;
}


IrradianceVolumeNode::~IrradianceVolumeNode()
{
	if (mRenderIrradianceVolume)
		mRenderIrradianceVolume->Destroy();
}



void IrradianceVolumeNode::OnTransform()
{

}


void IrradianceVolumeNode::UpdateIrradianceVolumeNode()
{
	CHECK(!mRenderIrradianceVolume);
	mRenderIrradianceVolume = UniquePtr<RenderIrradianceVolume>(new RenderIrradianceVolume());
	mRenderIrradianceVolume->SetDirty(LIGHT_PROBES_BOUNCES);
	mRenderIrradianceVolume->SetVolume(mStart, mExtent, mCount);
	mRenderIrradianceVolume->Create();

}


void IrradianceVolumeNode::SetVolume(const glm::vec3& start, const glm::vec3& extent, const glm::ivec3& count)
{
	mStart = start;
	mExtent = extent;
	mCount = count;
}


void IrradianceVolumeNode::GetVolume(glm::vec3& start, glm::vec3& extent, glm::ivec3& count) const
{
	start = mStart;
	extent = mExtent;
	count = mCount;
}


Box IrradianceVolumeNode::GetBounds() const
{
	Box bounds;
	bounds.Add(mStart);
	bounds.Add(mStart + mExtent);
	return bounds;
}
