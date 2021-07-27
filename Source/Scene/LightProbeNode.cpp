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






#include "LightProbeNode.h"
#include "Render/RenderData/RenderLight.h"







LightProbeNode::LightProbeNode()
	: mRadius(100.0f)
	, mIsSelected(false)
{
	mType = ENodeType::LightProbe;

}


LightProbeNode::~LightProbeNode()
{
	if (mRenderLightProbe)
		mRenderLightProbe->Destroy();
}


void LightProbeNode::SetRadius(float radius)
{
	mRadius = radius;

	if (mRenderLightProbe)
	{
		mRenderLightProbe->SetRadius(mRadius);
		mRenderLightProbe->SetDirty(true);
	}

}


glm::vec3 LightProbeNode::GetPosition() const
{
	return GetTransform().GetTranslate();
}


void LightProbeNode::OnTransform()
{
	if (mRenderLightProbe)
	{
		mRenderLightProbe->SetPosition(GetTransform().GetTranslate());
		mRenderLightProbe->SetDirty(true);
	}
}


void LightProbeNode::UpdateRenderLightProbe()
{
	CHECK(!mRenderLightProbe);
	mRenderLightProbe = UniquePtr<RenderLightProbe>(new RenderLightProbe());
	mRenderLightProbe->Create();

	mRenderLightProbe->SetDirty(2);
	mRenderLightProbe->SetRadius(mRadius);
	mRenderLightProbe->SetPosition(GetTransform().GetTranslate());

}
