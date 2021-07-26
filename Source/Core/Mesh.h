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



#include "Core.h"
#include "Box.h"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"



#include <vector>




class RenderMesh;




// Mesh Vertices.
struct MeshVert
{
	// Vertex Position.
	glm::vec3 position;

	// Vertex Normal.
	glm::vec3 normal;

	// Vertex texture coordinate.
	glm::vec2 texCoord;
};






// Mesh:
//  - Basic Mesh Geometry Data.
//
class Mesh
{
public:
	// Construct.
	Mesh();

	// Destruct.
	~Mesh();

	// Return mesh vertices.
	inline std::vector<MeshVert>& GetVertices() { return mVertices; }
	inline const std::vector<MeshVert>& GetVertices() const { return mVertices; }

	// Return mesh indices.
	inline std::vector<uint32_t>& GetIndices() { return mIndices; }
	inline const std::vector<uint32_t>& GetIndices() const { return mIndices; }

	// Return mesh bounding box.
	inline Box& GetBounds() { return mBounds; }
	inline const Box& GetBounds() const { return mBounds; }

	// Create/Update render mesh data.
	void UpdateRenderMesh();

	// Return the render mesh.
	RenderMesh* GetRenderMesh() { return mRenderMesh.get(); }
	const RenderMesh* GetRenderMesh() const { return mRenderMesh.get(); }

public:
	// Create Sphere Mesh.
	static Mesh* MakeSphere(uint32_t seg, float radius);

private:
	// Mesh Vertices.
	std::vector<MeshVert> mVertices;

	// Mesh Indices.
	std::vector<uint32_t> mIndices;

	// Mesh Bounds.
	Box mBounds;

	// The render data for this mesh.
	Ptr<RenderMesh> mRenderMesh;
};
