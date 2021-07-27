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




#include "MeshNode.h"
#include "Core/Mesh.h"



MeshNode::MeshNode()
	: Node()
{
	mType = ENodeType::MeshNode;

}


MeshNode::~MeshNode()
{

}


void MeshNode::SetMesh(uint32_t index, Ptr<Mesh> mesh)
{
	if (mMeshes.size() < index + 1)
		mMeshes.resize(index + 1);

	mMeshes[index] = mesh;
	UpdateBounds();
}


void MeshNode::SetMaterial(uint32_t index, Ptr<Material> mat)
{
	if (mMaterials.size() < index + 1)
		mMaterials.resize(index + 1);

	mMaterials[index] = mat;
}


void MeshNode::UpdateBounds()
{
	mBounds.Reset();

	for (size_t i = 0; i < mMeshes.size(); ++i)
	{
		mBounds.Add( mMeshes[i]->GetBounds() );
	}
}


Box MeshNode::GetBounds() const
{
	return mBounds;
}
