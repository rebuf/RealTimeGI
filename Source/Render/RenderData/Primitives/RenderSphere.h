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
#include "IRenderPrimitives.h"
#include "glm/vec3.hpp"



class VKIBuffer;
class RenderUniform;
class VKICommandBuffer;






// RenderMesh:
//    - Render data for a mesh.
//
class RenderSphere : public IRenderPrimitives
{
public:
	// Construct.
	RenderSphere();

	// Destruct.
	~RenderSphere();

	// Update The Sphere Render Data.
	void UpdateData();

	// Draw the mesh.
	virtual void Draw(VKICommandBuffer* cmdBuffer) override;

	// Return the sphere unifrom for rendering multiple view layers.
	inline RenderUniform* GetSphereUnifrom() const { return mSphereUnifrom.get(); }

private:
	// Vertex Buffer.
	UniquePtr<VKIBuffer> mVertBuffer;

	// Index Buffer.
	UniquePtr<VKIBuffer> mIdxBuffer;

	// The number of indicies in the index buffer.
	uint32_t mNumIndices;

	// Sphere Uniform for drawing sphere into 6 cubemap layers render pass.
	UniquePtr<RenderUniform> mSphereUnifrom;
};


