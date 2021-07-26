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




#include "Mesh.h"
#include "Render/RenderData/Primitives/RenderMesh.h"


#include "glm/geometric.hpp"





Mesh::Mesh()
{

}


Mesh::~Mesh()
{

}


void Mesh::UpdateRenderMesh()
{
	if (!mRenderMesh)
	{
		mRenderMesh = UniquePtr<RenderMesh>(new RenderMesh());
		mRenderMesh->SetData(this);
	}
	else
	{
		CHECK(0 && "TODO: Implement Update.");
	}
}


Mesh* Mesh::MakeSphere(uint32_t seg, float radius)
{
	float H_SEG = static_cast<float>(seg);
	float V_SEG = static_cast<float>(seg);
	float sectorStep = 2.0F * PI / H_SEG;
	float stackStep = PI / V_SEG;

	Mesh* mesh = new Mesh();

	for (int32_t i = 0; i <= V_SEG; ++i)
	{
		float stackAngle = PI / 2.0f - (float)i * stackStep; 
		float xy = radius * cos(stackAngle);
		float z = radius * sin(stackAngle); 

		for (int32_t j = 0; j <= H_SEG; ++j)
		{
			float sectorAngle = j * sectorStep;

			// Position.
			float x = xy * cosf(sectorAngle);
			float y = xy * sinf(sectorAngle);

			// Texture Coordinate.
			float s = static_cast<float>(j / H_SEG);
			float t = static_cast<float>(i / V_SEG);

			// Vertex.
			MeshVert vert;
			vert.position = glm::vec3(x, y, z);
			vert.normal = glm::normalize(vert.position);
			vert.texCoord = glm::vec2(s, t);
			mesh->GetVertices().emplace_back(vert);

			// Compute Bounds.
			mesh->GetBounds().Add(vert.position);
		}
	}


	uint32_t iH1, iH2;

	for (uint32_t i = 0; i < V_SEG; ++i)
	{
		iH1 = i * (static_cast<uint32_t>(H_SEG) + 1U);  // Start of Horizontal Seg
		iH2 = iH1 + static_cast<uint32_t>(H_SEG) + 1U;  // Next Start of Horizontal Seg

		for (uint32_t j = 0; j < H_SEG; ++j, ++iH1, ++iH2)
		{
			if (i != 0)
			{
				mesh->GetIndices().emplace_back(iH1);
				mesh->GetIndices().emplace_back(iH2);
				mesh->GetIndices().emplace_back(iH1 + 1);
			}

			// Last?
			if (i != (V_SEG - 1))
			{
				mesh->GetIndices().emplace_back(iH1 + 1);
				mesh->GetIndices().emplace_back(iH2);
				mesh->GetIndices().emplace_back(iH2 + 1);
			}
		}
	}

	return mesh;
}
