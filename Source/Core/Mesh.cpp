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



Mesh* Mesh::MakeBox()
{

	//    v6----- v5
	//   /|      /|
	//  v1------v0|
	//  | |     | |
	//  | |v7---|-|v4
	//  |/      |/
	//  v2------v3
	Mesh* mesh = new Mesh();
	mesh->GetVertices().resize(24);

	mesh->GetVertices()[0].position = glm::vec3(1.0f, 1.0f, 1.0f);
	mesh->GetVertices()[0].normal = glm::vec3(0.0f, 0.0f, 1.0f);

	mesh->GetVertices()[1].position = glm::vec3(-1.0f, 1.0f, 1.0f);
	mesh->GetVertices()[1].normal = glm::vec3(0.0f, 0.0f, 1.0f);

	mesh->GetVertices()[2].position = glm::vec3(-1.0f, -1.0f, 1.0f);
	mesh->GetVertices()[2].normal = glm::vec3(0.0f, 0.0f, 1.0f);

	mesh->GetVertices()[3].position = glm::vec3(1.0f, -1.0f, 1.0f);
	mesh->GetVertices()[3].normal = glm::vec3(0.0f, 0.0f, 1.0f);

	mesh->GetVertices()[4].position = glm::vec3(1.0f, 1.0f, 1.0f);
	mesh->GetVertices()[4].normal = glm::vec3(1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[5].position = glm::vec3(1.0f, -1.0f, 1.0f);
	mesh->GetVertices()[5].normal = glm::vec3(1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[6].position = glm::vec3(1.0f, -1.0f, -1.0f);
	mesh->GetVertices()[6].normal = glm::vec3(1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[7].position = glm::vec3(1.0f, 1.0f, -1.0f);
	mesh->GetVertices()[7].normal = glm::vec3(1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[8].position = glm::vec3(1.0f, 1.0f, 1.0f);
	mesh->GetVertices()[8].normal = glm::vec3(0.0f, 1.0f, 0.0f);

	mesh->GetVertices()[9].position = glm::vec3(1.0f, 1.0f, -1.0f);
	mesh->GetVertices()[9].normal = glm::vec3(0.0f, 1.0f, 0.0f);

	mesh->GetVertices()[10].position = glm::vec3(-1.0f, 1.0f, -1.0f);
	mesh->GetVertices()[10].normal = glm::vec3(0.0f, 1.0f, 0.0f);

	mesh->GetVertices()[11].position = glm::vec3(-1.0f, 1.0f, 1.0f);
	mesh->GetVertices()[11].normal = glm::vec3(0.0f, 1.0f, 0.0f);

	mesh->GetVertices()[12].position = glm::vec3(-1.0f, 1.0f, 1.0f);
	mesh->GetVertices()[12].normal = glm::vec3(-1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[13].position = glm::vec3(-1.0f, 1.0f, -1.0f);
	mesh->GetVertices()[13].normal = glm::vec3(-1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[14].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	mesh->GetVertices()[14].normal = glm::vec3(-1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[15].position = glm::vec3(-1.0f, -1.0f, 1.0f);
	mesh->GetVertices()[15].normal = glm::vec3(-1.0f, 0.0f, 0.0f);

	mesh->GetVertices()[16].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	mesh->GetVertices()[16].normal = glm::vec3(0.0f, -1.0f, 0.0f);

	mesh->GetVertices()[17].position = glm::vec3(1.0f, -1.0f, -1.0f);
	mesh->GetVertices()[17].normal = glm::vec3(0.0f, -1.0f, 0.0f);

	mesh->GetVertices()[18].position = glm::vec3(1.0f, -1.0f, 1.0f);
	mesh->GetVertices()[18].normal = glm::vec3(0.0f, -1.0f, 0.0f);

	mesh->GetVertices()[19].position = glm::vec3(-1.0f, -1.0f, 1.0f);
	mesh->GetVertices()[19].normal = glm::vec3(0.0f, -1.0f, 0.0f);

	mesh->GetVertices()[20].position = glm::vec3(1.0f, -1.0f, -1.0f);
	mesh->GetVertices()[20].normal = glm::vec3(0.0f, 0.0f, -1.0f);

	mesh->GetVertices()[21].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	mesh->GetVertices()[21].normal = glm::vec3(0.0f, 0.0f, -1.0f);

	mesh->GetVertices()[22].position = glm::vec3(-1.0f, 1.0f, -1.0f);
	mesh->GetVertices()[22].normal = glm::vec3(0.0f, 0.0f, -1.0f);

	mesh->GetVertices()[23].position = glm::vec3(1.0f, 1.0f, -1.0f);
	mesh->GetVertices()[23].normal = glm::vec3(0.0f, 0.0f, -1.0f);

	for (int i = 0; i < 6; i++)
	{
		mesh->GetVertices()[i * 4 + 0].texCoord = glm::vec2(0.0f, 0.0f);
		mesh->GetVertices()[i * 4 + 1].texCoord = glm::vec2(1.0f, 0.0f);
		mesh->GetVertices()[i * 4 + 2].texCoord = glm::vec2(1.0f, 1.0f);
		mesh->GetVertices()[i * 4 + 3].texCoord = glm::vec2(0.0f, 1.0f);
	}

	mesh->GetIndices() = {
		0,1,2,
		0,2,3,
		4,5,6,
		4,6,7,
		8,9,10,
		8,10,11,
		12,13,14,
		12,14,15,
		16,17,18,
		16,18,19,
		20,21,22,
		20,22,23
	};


	mesh->mBounds = Box(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	return mesh;
}
