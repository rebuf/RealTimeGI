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
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"




class Node;





// Flags used to mark the scene dirty.
enum class ESceneGlobalDirtyFlag : uint32_t
{
	// No Dirty Flags.
	None = 0,

	// The Sun in the scene has changed needs updating.
	DirtySun = 1
};


// ESceneGlobalDirtyFlag Bit Operators
DEFINE_ENUM_CLASS_BIT_OPERATORS(ESceneGlobalDirtyFlag, uint32_t)






// SceneGlobalSettings:
//   - Holds the data for the scene global settings.
//
class SceneGlobalSettings
{
public:
	// Default Construct.
	SceneGlobalSettings()
		: mSunDir(0.0f)
		, mSunColor(1.0f)
		, mSunPower(1.0f)
		, isLightProbeEnabled(false)
		, isLightProbeHelpers(false)
		, isLightProbeVisualize(false)
	{

	}


	// Clear all dirty flags.
	inline void ClearDirtyFlags() { mDirtyFlags = ESceneGlobalDirtyFlag::None; }
	inline void ClearDirtyFlag(ESceneGlobalDirtyFlag flag) { mDirtyFlags = mDirtyFlags & (~flag); }

	// Set drity flag.
	inline void SetDirtyFlag(ESceneGlobalDirtyFlag flag) { mDirtyFlags = mDirtyFlags | flag; }

	// Return true if dirty flag is set.
	inline bool HasDirtyFlag(ESceneGlobalDirtyFlag flag) { return (mDirtyFlags & flag) != ESceneGlobalDirtyFlag::None; }

	// Set Sun Properties.
	inline void SetSunDir(const glm::vec3& dir) { mSunDir = dir; SetDirtyFlag(ESceneGlobalDirtyFlag::DirtySun); }
	inline void SetSunColor(const glm::vec3& color) { mSunColor = color; SetDirtyFlag(ESceneGlobalDirtyFlag::DirtySun); }
	inline void SetSunPower(float power) { mSunPower = power; SetDirtyFlag(ESceneGlobalDirtyFlag::DirtySun); }

	// Set Sun Properties.
	inline const glm::vec3& GetSunDir() { return mSunDir; }
	inline const glm::vec3& GetSunColor() { return mSunColor; }
	inline float GetSunPower() { return mSunPower; }


private:
	// The Sun Direction.
	glm::vec3 mSunDir;

	// The Sun Color(RGB) & Power(A).
	glm::vec3 mSunColor;

	// The Sun Power.
	float mSunPower;

	// The global settings dirty flags.
	ESceneGlobalDirtyFlag mDirtyFlags;

public:
	//
	bool isLightProbeEnabled;
	bool isLightProbeHelpers;
	bool isLightProbeVisualize;

};
