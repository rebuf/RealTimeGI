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
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"




class RenderMaterial;
class Image2D;





// Basic Material Data.
struct MaterialData
{
	// The Base Color.
	glm::vec4 color;

	// x[Roughness], y[Metallic].
	glm::vec4 brdf;

};






// Material:
//    - 
class Material
{
public:
	// Construct.
	Material();

	// Destruct.
	~Material();

	// Set/Get Material Base Color.
	void SetColor(const glm::vec4& value);
	const glm::vec4& GetColor();

	// Set/Get Material Color Texture.
	void SetColorTexture(Ptr<Image2D> img);
	Ptr<Image2D> GetColorTexture();

	// Set/Get Material Roughness Metallic Texture.
	void SetRoughnessMetallic(Ptr<Image2D> img);
	Ptr<Image2D> GetRoughnessMetallic();

	// Retrun true if the material is dirty and need updating.
	inline bool IsDirty() const { return mIsDirty; }

	// If true update the material render data.
	void UpdateRenderMaterial();

	// Return render materail.
	RenderMaterial* GetRenderMaterial() { return mRenderMaterial.get(); }

private:
	// Materail Data.
	MaterialData mData;

	// Color Texture.
	Ptr<Image2D> mColorTexture;

	// Roughness Metallic Texture.
	Ptr<Image2D> mRoughnessMetallic;

	// If true the materail need to be updated.
	bool mIsDirty;

	// Render Material Data.
	UniquePtr<RenderMaterial> mRenderMaterial;

};

