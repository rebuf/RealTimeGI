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




#include "RenderMesh.h"
#include "Core/Mesh.h"
#include "Application.h"
#include "Render/Renderer.h"


#include "Render/VKInterface/VKIBuffer.h"
#include "Render/VKInterface/VKICommandBuffer.h"


#include <array>






RenderMesh::RenderMesh()
{

}


RenderMesh::~RenderMesh()
{
	mVertBuffer->Destroy();
	mIdxBuffer->Destroy();
}


void RenderMesh::Draw(VKICommandBuffer* cmdBuffer)
{
	VkCommandBuffer cmd = cmdBuffer->GetCurrent();

	// Bind Vertex Buffer.
	VkBuffer buffer = mVertBuffer->Get();
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);

	// Bind Index Buffer.
	vkCmdBindIndexBuffer(cmd, mIdxBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);

	// Draw...
	vkCmdDrawIndexed(cmd, mNumIndices, 1, 0, 0, 0);
}


void RenderMesh::SetData(Mesh* mesh)
{
	Renderer* renderer = Application::Get().GetRenderer();

	// Vertex Buffer...
	mVertBuffer = UniquePtr<VKIBuffer>(new VKIBuffer());
	mVertBuffer->SetSize(mesh->GetVertices().size() * sizeof(MeshVert));
	mVertBuffer->SetUsage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	mVertBuffer->SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	mVertBuffer->CreateBuffer(renderer->GetVKDevice());
	mVertBuffer->UpdateDataStaging(mesh->GetVertices().data());

	// Index Buffer...
	mIdxBuffer = UniquePtr<VKIBuffer>(new VKIBuffer());
	mIdxBuffer->SetSize(mesh->GetIndices().size() * sizeof(uint32_t));
	mIdxBuffer->SetUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	mIdxBuffer->SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	mIdxBuffer->CreateBuffer(renderer->GetVKDevice());
	mIdxBuffer->UpdateDataStaging(mesh->GetIndices().data());

	mNumIndices = (uint32_t)mesh->GetIndices().size();

}
