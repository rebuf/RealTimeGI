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
#include "glm/vec2.hpp"





class VKIImage;
class VKISampler;
class VKIImageView;
class VKIFramebuffer;
class VKIDescriptorSet;









// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 




// RenderLightProbe:
//     - render data for a single light probe in the scene.
//
class RenderLightProbe
{
public:
	// Construct.
	RenderLightProbe();

	// Destruct.
	~RenderLightProbe();

	// Create the light probe render data.
	void Create();

	// Destroy the light probe render data.
	void Destroy();

	// Flag this light probe dirty to be updated.
	inline void SetDirty(uint32_t val) { mIsDirty = val; }

	// Return true if this light probe is dirty.
	inline uint32_t GetDirty() const { return mIsDirty; }

	// Set/Get Position.
	inline void SetPosition(const glm::vec3& pos) { mPosition = pos; }
	inline const glm::vec3& GetPosition() const { return mPosition; }

	// Set/Get Radius.
	inline void SetRadius(float value) { mRadius = value; }
	inline float GetRadius() const { return mRadius; }

	// Return irradiance data...
	inline VKIImage* GetIrradiance() const { return mIrradiance.get(); }
	inline VKIFramebuffer* GetIrradianceFB() const { return mIrradianceFB.get(); }
	inline VKIImageView* GetIrradianceView() const { return mView[0].get(); }
	inline VKISampler* GetIrradianceSampler() const { return mSampler[0].get(); }

	// Return irradiance data...
	inline VKIImage* GetRadiance() const { return mRadiance.get(); }
	inline VKIFramebuffer* GetRadianceFB() const { return mRadianceFB.get(); }
	inline VKIImageView* GetRadianceView() const { return mView[1].get(); }
	inline VKISampler* GetRadianceSampler() const { return mSampler[1].get(); }

	// Return lighting descriptor set used for light proble lighting shader.
	VKIDescriptorSet* GetLightingDescSet() const { return mLightingSet.get(); }

	// Return descriptor set used for visualizing this light probe.
	VKIDescriptorSet* GetVisualizeDescSet() const { return mVisualizeSet.get(); }

	// Return descriptor set used for rendering radiance cube map.
	VKIDescriptorSet* GetRadianceDescSet() const { return mIrradianceFilterSet.get(); }

private:
	// Flag used to check if its dirty and need updating.
	uint32_t mIsDirty;

	// Radiance Image.
	UniquePtr<VKIImage> mRadiance;

	// Irradiance Image.
	UniquePtr<VKIImage> mIrradiance;

	// Sampler for Irradiance[1] & Radiance[0].
	UniquePtr<VKISampler> mSampler[2];

	// Image View for Irradiance[1] & Radiance[0].
	UniquePtr<VKIImageView> mView[2];

	// Framebuffer for irradiance target.
	UniquePtr<VKIFramebuffer> mIrradianceFB;

	// Framebuffer for Radiance target.
	UniquePtr<VKIFramebuffer> mRadianceFB;

	// The Position of the light probe.
	glm::vec3 mPosition;

	// The Radius of the light probe.
	float mRadius;

	// Descriptor Set for lighting pass.
	UniquePtr<VKIDescriptorSet> mLightingSet;

	// Descriptor Set for visualize pass.
	UniquePtr<VKIDescriptorSet> mVisualizeSet;

	// Descriptor Set for Irradiance filter pass.
	UniquePtr<VKIDescriptorSet> mIrradianceFilterSet;
};




// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 





// RenderIrradianceVolume:
//     - render data for a irradiance volume made of a grid of light probes.
//
class RenderIrradianceVolume
{
public:
	// Construct.
	RenderIrradianceVolume();

	// Destruct.
	~RenderIrradianceVolume();

	// Create the light probe render data.
	void Create();

	// Destroy the light probe render data.
	void Destroy();

	// Flag this light probe dirty to be updated.
	inline void SetDirty(uint32_t val) { mIsDirty = val; }

	// Return true if this light probe is dirty.
	inline uint32_t GetDirty() const { return mIsDirty; }

	// Set/Get Irradiance Volume Info.
	void SetVolume(const glm::vec3& start, const glm::vec3& extent, const glm::ivec3& count);
	inline glm::vec3 GetVolumeStart() const { return mStart; }
	inline glm::vec3 GetVolumeExtent() const { return mExtent; }
	inline glm::ivec3 GetVolumeCount() const { return mCount; }

	// Return irradiance data...
	inline VKIImage* GetIrradiance() const { return mIrradiance.get(); }
	inline VKIFramebuffer* GetIrradianceFB() const { return mIrradianceFB.get(); }
	inline VKIImageView* GetIrradianceView() const { return mView[0].get(); }
	inline VKISampler* GetIrradianceSampler() const { return mSampler[0].get(); }

	// Return irradiance data...
	inline VKIImage* GetRadiance() const { return mRadiance.get(); }
	inline VKIFramebuffer* GetRadianceFB() const { return mRadianceFB.get(); }
	inline VKIImageView* GetRadianceView() const { return mView[1].get(); }
	inline VKISampler* GetRadianceSampler() const { return mSampler[1].get(); }

	// Return lighting descriptor set used for light proble lighting shader.
	VKIDescriptorSet* GetLightingDescSet() const { return mLightingSet.get(); }

	// Return descriptor set used for rendering radiance cube map.
	VKIDescriptorSet* GetRadianceDescSet() const { return mIrradianceFilterSet.get(); }

	// Return the total number of light probes in the volume.
	uint32_t GetNumProbes();

	// Return the probe location in the volume.
	glm::ivec3 GetProbeGridCoord(uint32_t index);

	// Return the probe location in the volume.
	glm::vec3 GetProbePosition(uint32_t index);

	// Return the probe layer index.
	uint32_t GetProbeLayer(uint32_t index, uint32_t face);

	// Volume Shape...
	inline glm::vec3 GetStart() const { return mStart; }
	inline glm::vec3 GetExtent() const { return mExtent; }
	inline glm::ivec3 GetCount() const { return mCount; }

	// Set/Get Volume Attenuation.
	inline void SetAtten(const glm::vec3& atten) { mAtten = atten; }
	inline glm::vec3 GetAtten() const { return mAtten; }

private:
	// Flag used to check if its dirty and need updating.
	uint32_t mIsDirty;

	// Radiance Image.
	UniquePtr<VKIImage> mRadiance;

	// Irradiance Image.
	UniquePtr<VKIImage> mIrradiance;

	// Sampler for Irradiance[1] & Radiance[0].
	UniquePtr<VKISampler> mSampler[2];

	// Image View for Irradiance[1] & Radiance[0].
	UniquePtr<VKIImageView> mView[2];

	// Framebuffer for irradiance target.
	UniquePtr<VKIFramebuffer> mIrradianceFB;

	// Framebuffer for Radiance target.
	UniquePtr<VKIFramebuffer> mRadianceFB;

	// The Start of the volume.
	glm::vec3 mStart;

	// The Extent of the volume.
	glm::vec3 mExtent;

	// Number of grid voxels on each axis.
	glm::ivec3 mCount;

	// Volume Attenuation.
	glm::vec3 mAtten;

	// Descriptor Set for lighting pass.
	UniquePtr<VKIDescriptorSet> mLightingSet;

	// Descriptor Set for Irradiance filter pass.
	UniquePtr<VKIDescriptorSet> mIrradianceFilterSet;
};
