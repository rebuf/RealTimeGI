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
#include "Node.h"
#include "Core/Box.h"


#include <vector>




class Mesh;





// MeshNode:
//   - Node that represent a mesh in the scene.
//
class MeshNode : public Node
{
public:
	// Construct.
	MeshNode();

	// Destruct.
	~MeshNode();

	// Set the mesh of this mesh node.
	void SetMesh(uint32_t index, Ptr<Mesh> mesh);

	// Return this mesh MeshNode.
	inline Mesh* GetMesh(uint32_t index) const { return mMeshes[index].get(); }

	// Return the number of meshes in this mesh node.
	uint32_t GetNumMeshes() const { return (uint32_t)mMeshes.size(); }

	// Return the mesh node bounds.
	virtual Box GetBounds() const override;

private:
	// Update nodes bounds.
	void UpdateBounds();

private:
	// The Mesh.
	std::vector< Ptr<Mesh> > mMeshes;

	// The mesh bounds in world coordinate.
	Box mBounds;

};


