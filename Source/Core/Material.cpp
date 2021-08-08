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




#include "Material.h"
#include "Image2D.h"

#include "Render/RenderData/Shaders/RenderMaterial.h"





Material::Material()
	: mIsDirty(false)
{
	mData.color = glm::vec4(1.0f);
	mData.brdf = glm::vec4(1.0f);
	mData.emission = glm::vec4(0.0f);
}


Material::~Material()
{

}


void Material::SetColor(const glm::vec4& value)
{
	mData.color = value;
	mIsDirty = true;
}


const glm::vec4& Material::GetColor()
{
	return mData.color;
}


void Material::SetEmission(const glm::vec4& value)
{
	mData.emission = value;
}


const glm::vec4& Material::GetEmission()
{
	return mData.emission;
}


void Material::SetColorTexture(Ptr<Image2D> img)
{
	mColorTexture = img;
	mIsDirty = true;
}


Ptr<Image2D> Material::GetColorTexture()
{
	return mColorTexture;
}


void Material::SetRoughnessMetallic(Ptr<Image2D> img)
{
	mRoughnessMetallic = img;
	mIsDirty = true;
}


Ptr<Image2D> Material::GetRoughnessMetallic()
{
	return mRoughnessMetallic;
}


void Material::UpdateRenderMaterial()
{
	mRenderMaterial = UniquePtr<RenderMaterial>(new RenderMaterial(ERenderMaterialType::Opaque));

	RenderImage* rColorTex = nullptr, *rMetRoughTex = nullptr;

	if (mColorTexture)
	{
		if (!mColorTexture->GetRenderImage())
			mColorTexture->UpdateRenderImage();

		rColorTex = mColorTexture->GetRenderImage();
	}

	if (mRoughnessMetallic)
	{
		if (!mRoughnessMetallic->GetRenderImage())
			mRoughnessMetallic->UpdateRenderImage();

		rMetRoughTex = mRoughnessMetallic->GetRenderImage();
	}


	mRenderMaterial->Setup(&mData, rColorTex, rMetRoughTex);
}
