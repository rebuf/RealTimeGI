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


#include "Scene/Scene.h"
#include "Scene/MeshNode.h"



#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#undef LoadImage



#include <array>



// List of supported gltf extensions.
static const std::array<std::string, 1> GLTF_EXT_LIST = { "gltf" };


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







std::shared_ptr<Image2D> GLTFLoadImage(const tinygltf::TextureInfo& imgInfo, const std::string& dir)
{
	if (imgInfo.index < gImagesUri.size() && imgInfo.index >= 0)
	{
		std::string uri = gImagesUri[imgInfo.index];
		std::shared_ptr<Image2D> img;

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


	if (!gDefaultWhite)
	{
		gDefaultWhite = std::shared_ptr<Image2D>(new Image2D());
		gDefaultWhite->LoadImage(RESOURCES_DIRECTORY "Textures/Default_White.png");
	}

	return gDefaultWhite;
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

	// Mesh <-> Materail
	for (size_t im = 0; im < model.materials.size(); ++im)
	{
		const tinygltf::Material& mat = model.materials[im];

		Mesh* mesh = new Mesh();
		meshes.emplace_back(mesh);

		// Load Textures...
		const tinygltf::TextureInfo tex1 = mat.pbrMetallicRoughness.baseColorTexture;
		const tinygltf::TextureInfo tex2 = mat.pbrMetallicRoughness.metallicRoughnessTexture;


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
				for (size_t i = 0; i < accIndex.count; ++i)
				{
					uint32_t index = indices_i == nullptr ? indices_s[i] : indices_i[i];
					index = indicesMap[index];
					mesh->GetIndices().push_back(index);
				}


			}
		}
	}


	// Create a new MeshNode and add it to the scene.
	Ptr<MeshNode> node = Ptr<MeshNode>( new MeshNode() );

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		meshes[i]->UpdateRenderMesh(); // Create & Update RenderMesh.
		node->SetMesh((uint32_t)i, meshes[i]);
	}


	scene->AddNode(node);


	gDefaultWhite.reset();
	gImagesUri.clear();
	gImagesMap.clear();
	return true;
}


void LoadMeshes(std::vector<Mesh*>& outMeshes)
{

}
