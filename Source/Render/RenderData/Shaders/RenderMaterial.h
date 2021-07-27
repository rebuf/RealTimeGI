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
#include "Render/RenderData/RenderTypes.h"



class Renderer;
class RenderShader;
class RenderUniform;
class RenderImage;
class VKIDescriptorSet;
class VKIImageView;
class VKISampler;
class VKICommandBuffer;
struct MaterialData;








// RenderMaterial:
//    - 
//
class RenderMaterial
{
public:
	// Construct.
	RenderMaterial(ERenderMaterialType type);

	// Destruct.
	~RenderMaterial();

	// Setup the render material.
	void Setup(MaterialData* data, RenderImage* colorImage, RenderImage* roughnessMetallicImage);

	//
	void Bind(VKICommandBuffer* cmdBuffer, uint32_t frame);

public:
	// Setup The material shaders used by the material system.
	static void SetupMaterialShaders(Renderer* renderer, RenderUniform* transformUniform);

	// Destroy the material shaders setup/maanged by RenderMaterial.
	static void DestroyMaterialShaders();

	// Return the material render shader for a specific shader type.
	static RenderShader* GetShader(ERenderMaterialType type);
	static RenderShader* GetDirShadowShader(ERenderMaterialType type);
	static RenderShader* GetLProbeShader(ERenderMaterialType type);

private:
	//
	static void SetupSphereHelperShader(Renderer* renderer);

private:
	// Opaque Material Shader.
	static Ptr<RenderShader> OPAQUE_SHADER;

	// Opaque[0]/Masked[1] Material Shader for shadow passes.
	static Ptr<RenderShader> SHADOW_DIR_SHADER[2];
	static Ptr<RenderShader> SHADOW_OMNI_SHADER[2];



public:
	// Update Material Uniform Buffer.
	static Ptr<RenderUniform> MATERAIL_UNIFORM;

	//
	static Ptr<RenderShader> SPHERE_HELPER_SHADER;

private:
	// The Shader type.
	ERenderMaterialType mType;

	// Material Descriptor Set.
	Ptr<VKIDescriptorSet> mDescriptorSet;

	// Materail Data, used to update the materail unifrom.
	MaterialData* mMatData;

	// Materail Textures.
	//  [0] Color Image
	//  [1] Roughness & Metallic.
	RenderImage* mTextures[2];

};

