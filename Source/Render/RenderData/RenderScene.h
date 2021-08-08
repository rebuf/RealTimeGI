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
#include "glm/vec3.hpp"
#include "glm/matrix.hpp"

#include <vector>




class Scene;
class Node;
class IRenderPrimitives;
class RenderUniform;
class IRenderShadow;
class RenderDirShadow;
class RenderLightProbe;
class RenderIrradianceVolume;
class RenderMaterial;
class RenderSphere;
class VKICommandBuffer;
class VKIImage;
class VKIFramebuffer;
class VKIDescriptorSet;






// a single primitive referneced by the render scene.
struct RDScenePrimitive
{
	// The Primitive to draw.
	IRenderPrimitives* primitive;

	// Primitive World Transform.
	glm::mat4 transform;

	// The Primitive Materail.
	RenderMaterial* materail;
};



// Primitive Helper
struct RDScenePrimitiveHelper
{
	IRenderPrimitives* primitive;
	glm::vec4 position;
	glm::vec4 scale;
	glm::vec4 color;
};






// Scene Render Environment
struct RDEnvironment
{
	// The Sun Direction.
	glm::vec4 sunDir;

	// The Sun Color(RGB) & Power(A).
	glm::vec4 sunColorAndPower;

	// 
	RenderLightProbe* mSelectedLightProbe;

	//
	bool isLightProbeEnabled;
	bool isLightProbeHelpers;
	bool isLightProbeVisualize;

	// Reset the environment data.
	void Reset();
};







// RenderScene:
//   - render data of the scene used to render the scene.
//
class RenderScene
{
public:
	// Construct.
	RenderScene();

	// Destruct.
	~RenderScene();

	// Initialize the render scene.
	void Initialize();

	// Destroy the render scene.
	void Destroy();

	// Build the render scene data from scene.
	void BuildRenderScene(Scene* scene);

	// Reset the scene data.
	void Reset();

	// Draw the scene.
	void DrawSceneDeferred(VKICommandBuffer* cmdBuffer, uint32_t frame);

	// Draw the scene for shadow pass.
	void DrawSceneShadow(VKICommandBuffer* cmdBuffer, uint32_t frame, IRenderShadow* shadow);

	//
	void DrawHelpers(VKICommandBuffer* cmdBuffer, uint32_t frame);

	// Return Render Environment.
	inline RDEnvironment GetEnvironment() const { return mEnvironment; }

	// Return view projection matrix.
	inline const glm::mat4& GetViewProj() const { return mViewProj; }

	// Return inverse of view projection matrix.
	inline const glm::mat4& GetViewProjInv() const { return mViewProjInv; }

	// Return Near & Far clip plane.
	inline const glm::vec2& GetNearFar() const { return mNearFar; }

	// Return the current scene ViewPos.
	inline const glm::vec3& GetViewPos() const { return mViewPos; }

	// Return the current scene ViewDir.
	inline const glm::vec3& GetViewDir() const { return mViewDir; }

	// Return uniform.
	inline RenderUniform* GetTransformUniform() { return mTransformUniform.get(); }
	inline RenderUniform* GetMatUniform() { return mMaterialUniform.get(); }

	// Update Dynamic Uniforms.
	void UpdateUniforms(uint32_t frame);

	// Return the sun shadow.
	inline RenderDirShadow* GetSunShadow() { return mSunShadow.get(); }

	// Return if the scene has dirty light probes that needs to be updated before rendering.
	inline bool HasDirtyLightProbes() const { return mHasDirtyLightProbe; }
	inline bool HasDirtyIrradianceVolume() const { return mHasDirtyIrradianceVolume; }

	// Return the light probes in the render scene.
	inline const std::vector<RenderLightProbe*>& GetLightProbes() const { return mLightProbes; }

	// Return the irradiance volumes in the render scene.
	inline const std::vector<RenderIrradianceVolume*>& GetIrradianceVolumes() const { return mIrradianceVolumes; }

	// Return lighting descriptor set used for sun lighting shader.
	VKIDescriptorSet* GetSunLightDescSet() const { return mSunLightingSet.get(); }

private:
	// Add new primitive to be rendered by the scene.
	RDScenePrimitive* AddNewPrimitive(IRenderPrimitives* primitive, const glm::mat4& transform);

	// Add new primitive helper to be rendered by the scene.
	RDScenePrimitiveHelper* AddNewHelper(IRenderPrimitives* primitive, const glm::vec4& pos, const glm::vec4& scale,
		const glm::vec4& color);

	// Collect the view data from the scene.
	void CollectSceneView(Scene* scene);

	// Collect lights from the scene.
	void CollectSceneLights(Scene* scene);

	// Collect render data from nodes.
	void TraverseScene(Scene* scene);

	// Create the sun render data.
	void CreateSunData();

	// Add light probe to be rendered with the scene.
	void AddLightProbe(Node* node);

	// Add irradiance volume to be rendered with the scene.
	void AddIrradianceVolume(Node* node);

private:
	// The scene we want to render.
	Scene* mScene;

	// List of all primitive that will be drawn in the scene.
	std::vector<RDScenePrimitive*> mPrimitives;
	std::vector<RDScenePrimitiveHelper*> mPrimitivesHelpers;

	// The scene global environment data
	RDEnvironment mEnvironment;

	// View & Projection.
	glm::mat4 mViewProj;

	// Inverse of View & Projection.
	glm::mat4 mViewProjInv;

	// Near & Far clip plane.
	glm::vec2 mNearFar;

	// View Position.
	glm::vec3 mViewPos;

	// View Direction.
	glm::vec3 mViewDir;

	// Transfrom Uniform.
	UniquePtr<RenderUniform> mTransformUniform;

	// Material Uniform.
	UniquePtr<RenderUniform> mMaterialUniform;

	// The Sun Shadow.
	UniquePtr<RenderDirShadow> mSunShadow;

	// Light Probes to render in the scene.
	std::vector<RenderLightProbe*> mLightProbes;

	// Irradiance volumes to render in the scene.
	std::vector<RenderIrradianceVolume*> mIrradianceVolumes;

	// This render scene include a light probe that needs to be updated.
	bool mHasDirtyLightProbe;
	bool mHasDirtyIrradianceVolume;

	// Descriptor Set for sun lighting pass.
	UniquePtr<VKIDescriptorSet> mSunLightingSet;

	// Render Sphere.
	UniquePtr<RenderSphere> mRSphere;

	// Dynamic Materail Data.
	std::vector<uint8_t> mDynamicMatData;
	uint32_t mDynamicMatDataCount;

};
