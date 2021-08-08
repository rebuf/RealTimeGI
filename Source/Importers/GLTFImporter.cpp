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





#include "GLTFImporter.h"
#include "Core/GISystem.h"
#include "Core/Image2D.h"
#include "Core/Mesh.h"
#include "Core/Material.h"


#include "Scene/Scene.h"
#include "Scene/MeshNode.h"



#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#undef LoadImage



#include <array>



// List of supported gltf extensions.
static const std::array<std::string, 1> GLTF_EXT_LIST = { ".gltf" };


std::vector<std::string> gImagesUri;
std::map< std::string, Ptr<Image2D> > gImagesMap;
Ptr<Image2D> gDefaultWhite;




// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 




bool GLTFLoadImageDataFunction(tinygltf::Image* img, const int image_idx, std::string* err,
	std::string* warn, int, int,
	const unsigned char*, int,
	void* user_pointer)
{
	if (image_idx + 1 > gImagesUri.size())
		gImagesUri.resize(image_idx + 1);

	gImagesUri[image_idx] = img->uri;
	return true;
}




// --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- --- 





bool GLTFImporter::IsSupported(const std::string& file)
{
	std::string ext = GISystem::GetFileExtension(file);

	if (ext.empty())
		return false;

	for (uint32_t i = 0; i < GLTF_EXT_LIST.size(); ++i)
	{
		if (GLTF_EXT_LIST[i] == ext)
			return true;
	}

	return false;
}







Ptr<Image2D> GLTFLoadImage(const tinygltf::TextureInfo& imgInfo, const std::string& dir)
{
	if (imgInfo.index < gImagesUri.size() && imgInfo.index >= 0)
	{
		std::string uri = gImagesUri[imgInfo.index];
		Ptr<Image2D> img;

		if (!uri.empty())
		{
			if (gImagesMap.count(uri))
			{
				img = gImagesMap[uri];
			}
			else
			{
				img = std::make_shared<Image2D>();
				std::string path = dir + uri;

				bool isLoaded = img->LoadImage(path);

				if (!isLoaded || img->GetSize().x == 0 || img->GetSize().y == 0)
					img = nullptr;

				gImagesMap[uri] = img;
			}

			if (img)
				return img;
		}
	}

	return nullptr;
}


bool GLTFImporter::Import(Scene* scene, const std::string& file)
{
	std::string nfile = GISystem::NormalizePath(file);
	std::string dir = GISystem::GetDirectory(nfile);

	// Model Data...
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;


	loader.SetImageLoader(&GLTFLoadImageDataFunction, nullptr);

	// Load Model From File...
	if (!loader.LoadASCIIFromFile(&model, &err, &warn, nfile))
	{
		LOGE("Failed to import GLTF file(%s).", file.c_str());
		return false;
	}


	//
	std::vector< Ptr<Mesh> > meshes;
	std::vector< Ptr<Material> > materails;

	// Mesh <-> Materail
	for (size_t im = 0; im < model.materials.size(); ++im)
	{
		const tinygltf::Material& mat = model.materials[im];

		Mesh* mesh = new Mesh();
		meshes.emplace_back(mesh);

		Material* material = new Material();
		materails.emplace_back(material);

		// Load Textures...
		const tinygltf::TextureInfo tex1 = mat.pbrMetallicRoughness.baseColorTexture;
		const tinygltf::TextureInfo tex2 = mat.pbrMetallicRoughness.metallicRoughnessTexture;
		material->SetColorTexture(GLTFLoadImage(tex1, dir));
		material->SetRoughnessMetallic(GLTFLoadImage(tex2, dir));

		material->SetColor(glm::vec4(
			mat.pbrMetallicRoughness.baseColorFactor[0],
			mat.pbrMetallicRoughness.baseColorFactor[1],
			mat.pbrMetallicRoughness.baseColorFactor[2],
			mat.pbrMetallicRoughness.baseColorFactor[3]));

		material->SetEmission(glm::vec4(
			mat.emissiveFactor[0],
			mat.emissiveFactor[1],
			mat.emissiveFactor[2], 0.0f));


		// Load Mesh Data...
		for (size_t is = 0; is < model.meshes.size(); ++is)
		{
			const tinygltf::Mesh& ms = model.meshes[is];

			// Load Primitive Data
			for (size_t ip = 0; ip < ms.primitives.size(); ++ip)
			{
				const tinygltf::Primitive& prim = ms.primitives[ip];

				// Not this material?
				if (prim.material != im)
					continue;



				const float* positions = nullptr;
				const float* normals = nullptr;
				const float* texCoords = nullptr;
				const uint16_t* indices_s = nullptr;
				const uint32_t* indices_i = nullptr;


				// Positions...
				const tinygltf::Accessor& accPosition = model.accessors[(prim.attributes.find("POSITION"))->second];
				{
					const tinygltf::BufferView& bufferViewPos = model.bufferViews[accPosition.bufferView];
					const tinygltf::Buffer& bufferPos = model.buffers[bufferViewPos.buffer];
					positions = reinterpret_cast<const float*>(&bufferPos.data[bufferViewPos.byteOffset + accPosition.byteOffset]);
				}

				// Normals...
				{
					const tinygltf::Accessor& accNormal = model.accessors[(prim.attributes.find("NORMAL"))->second];
					const tinygltf::BufferView& bufferViewNormal = model.bufferViews[accNormal.bufferView];
					const tinygltf::Buffer& bufferNormal = model.buffers[bufferViewNormal.buffer];
					normals = reinterpret_cast<const float*>(&bufferNormal.data[bufferViewNormal.byteOffset + accNormal.byteOffset]);
				}

				// TexCoords...
				{
					const tinygltf::Accessor& accTexCoord = model.accessors[(prim.attributes.find("TEXCOORD_0"))->second];
					const tinygltf::BufferView& bufferViewTexCoord = model.bufferViews[accTexCoord.bufferView];
					const tinygltf::Buffer& bufferTexCoord = model.buffers[bufferViewTexCoord.buffer];
					texCoords = reinterpret_cast<const float*>(&bufferTexCoord.data[bufferViewTexCoord.byteOffset + accTexCoord.byteOffset]);
				}

				// Indices...
				const tinygltf::Accessor& accIndex = model.accessors[prim.indices];
				{
					const tinygltf::BufferView& bufferViewIndex = model.bufferViews[accIndex.bufferView];
					const tinygltf::Buffer& bufferIndex = model.buffers[bufferViewIndex.buffer];

					if (accIndex.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
						indices_s = reinterpret_cast<const uint16_t*>(&bufferIndex.data[bufferViewIndex.byteOffset]);

					if (accIndex.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
						indices_i = reinterpret_cast<const uint32_t*>(&bufferIndex.data[bufferViewIndex.byteOffset]);
				}

				std::vector<uint32_t> indicesMap;
				indicesMap.resize(accPosition.count);


				// Vertex Data...
				for (size_t i = 0; i < accPosition.count; ++i)
				{
					MeshVert vx;
					vx.position.x = positions[i * 3 + 0];
					vx.position.z = positions[i * 3 + 1];
					vx.position.y = positions[i * 3 + 2];
					vx.position *= 0.5f;

					vx.normal.x = normals[i * 3 + 0];
					vx.normal.z = normals[i * 3 + 1];
					vx.normal.y = normals[i * 3 + 2];

					vx.texCoord.x = texCoords[i * 2 + 0];
					vx.texCoord.y = texCoords[i * 2 + 1];

					mesh->GetBounds().Add(vx.position);
					mesh->GetVertices().push_back(vx);

					indicesMap[i] = (uint32_t)mesh->GetVertices().size() - 1;
				}


				// Indices...
				for (size_t i = 0; i < accIndex.count; i+=3)
				{
					uint32_t index0 = indices_i == nullptr ? indices_s[i+0] : indices_i[i+0];
					uint32_t index1 = indices_i == nullptr ? indices_s[i+1] : indices_i[i+1];
					uint32_t index2 = indices_i == nullptr ? indices_s[i+2] : indices_i[i+2];

					index0 = indicesMap[index0];
					index1 = indicesMap[index1];
					index2 = indicesMap[index2];

					mesh->GetIndices().push_back(index0);
					mesh->GetIndices().push_back(index2);
					mesh->GetIndices().push_back(index1);
				}


			}
		}
	}


	// Create a new MeshNode and add it to the scene.
	Ptr<MeshNode> node = Ptr<MeshNode>( new MeshNode() );

	int32_t gcount = 0;

	for (uint32_t i = 0; i < (uint32_t)meshes.size(); ++i)
	{
		if (materails[i]->GetColorTexture() == gDefaultWhite)
			continue;

		meshes[i]->UpdateRenderMesh(); // Create & Update RenderMesh.
		materails[i]->UpdateRenderMaterial();
		node->SetMesh(gcount, meshes[i]);
		node->SetMaterial(gcount, materails[i]);

		++gcount;
	}


	scene->AddNode(node);


	gDefaultWhite.reset();
	gImagesUri.clear();
	gImagesMap.clear();
	return true;
}
