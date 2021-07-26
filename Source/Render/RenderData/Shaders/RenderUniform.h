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


#include <vector>




class Renderer;
class VKIBuffer;
class VKICommandBuffer;





// RenderUniform:
//    - Manage uniform buffers.
//    - The number of buffers created are based on the number of concurrent frames.
//
class RenderUniform
{
public:
	// Construct.
	RenderUniform();

	// Destruct.
	~RenderUniform();

	// Set this uniform buffer usage as transfer dst.
	void SetTransferDst(bool value);

	// Create Uniform.
	void Create(Renderer* owner, uint32_t size, bool isDynamic);

	// Destroy Buffers.
	void Destroy();

	// Update the buffer with data.
	void Update(uint32_t frame, uint32_t offset, uint32_t size, const void* data);

	// Update the buffer with data with the same size as the uniform buffer.
	void Update(uint32_t frame, const void* data);

	// Update the buffer using command.
	void CmdUpdate(VKICommandBuffer* cmdBuffer, int32_t frame, uint32_t offset, uint32_t size, const void* data);

	// Return the buffers.
	std::vector<VKIBuffer*> GetBuffers() const;

private:
	// The Uniform Buffer.
	std::vector< UniquePtr<VKIBuffer> > mBuffers;

	// Is this uniform dynamic.
	bool mIsDynamic;

	// if true the buffer usage include transfer dst.
	bool mIsTransferDst;
};

