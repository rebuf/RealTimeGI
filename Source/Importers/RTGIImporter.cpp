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





#include "RTGIImporter.h"
#include "Core/GISystem.h"
#include "Scene/Scene.h"
#include "Scene/IrradianceVolumeNode.h"
#include "Scene/LightProbeNode.h"
#include "json/json.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>


#include <vector>
#include <fstream>




using json = nlohmann::json;





namespace glm
{
	// serialize vectors...
	template<class Archive> void serialize(Archive& archive, glm::vec2& v)  { std::vector<float>   tmp = { v.x, v.y };           archive(tmp); }
	template<class Archive> void serialize(Archive& archive, glm::vec3& v)  { std::vector<float>   tmp = { v.x, v.y, v.z };      archive(tmp); }
	template<class Archive> void serialize(Archive& archive, glm::vec4& v)  { std::vector<float>   tmp = { v.x, v.y, v.z, v.w }; archive(tmp); }
	template<class Archive> void serialize(Archive& archive, glm::ivec2& v) { std::vector<int32_t> tmp = { v.x, v.y };           archive(tmp); }
	template<class Archive> void serialize(Archive& archive, glm::ivec3& v) { std::vector<int32_t> tmp = { v.x, v.y, v.z };      archive(tmp); }
	template<class Archive> void serialize(Archive& archive, glm::ivec4& v) { std::vector<int32_t> tmp = { v.x, v.y, v.z, v.w }; archive(tmp); }
}



bool RTGIImporter::IsSupported(const std::string& file)
{
	std::string ext = GISystem::GetFileExtension(file);
	return ext == ".rtgi";
}


bool RTGIImporter::SaveLightComponents(Scene* scene, const std::string& file)
{
	// Open Ouput File stream..
	std::ofstream fs;
	fs.open(file, std::ios::out);

	if (!fs.is_open())
	{
		LOGE("Failed to save .rtgi file.");
		return false;
	}


	json j = json::array();

	//
	for (auto& node : scene->GetLights())
	{
		if (node->GetType() == ENodeType::LightProbe)
		{
			LightProbeNode* probe = static_cast<LightProbeNode*>(node);
			glm::vec3 pos = probe->GetPosition();
			float r = probe->GetRadius();

			json obj = {
				{ "TYPE", "LIGHT_PROBE" },
				{ "POSITION", {pos.x, pos.y, pos.z} },
				{ "RADIUS", r }
			};

			j.push_back(obj);
		}
		else if (node->GetType() == ENodeType::IrradianceVolume)
		{
			IrradianceVolumeNode* volume = static_cast<IrradianceVolumeNode*>(node);

			glm::vec3 start, extent, atten;
			glm::ivec3 count;
			volume->GetVolume(start, extent, count);
			atten = volume->GetAtten();

			json obj = {
				{ "TYPE", "IRRADIANCE_VOLUME" },
				{ "START",  { start.x,  start.y,  start.z  } },
				{ "SIZE",   { extent.x, extent.y, extent.z } },
				{ "COUNT",  { count.x,  count.y,  count.z  } },
				{ "ATTEN",  { atten.x,  atten.y,  atten.z  } }
			};

			j.push_back(obj);
		}
	}



	// Write...
	{
		std::string x = j.dump(4);
		fs.write(x.c_str(), x.size());
		fs.close();
	}


	return true;
}


bool RTGIImporter::ImportLightComponents(Scene* scene, const std::string& file)
{
	std::ifstream fs;
	fs.open(file, std::ios::in);

	if (!fs.is_open())
	{
		return false;
	}


	// Json Object..
	json j;

	// Read File...
	{
		char buffer[1024];
		std::string input;

		while (fs.good())
		{
			fs.read(buffer, 1000);
			input.append(buffer, buffer + fs.gcount());
		}

		fs.close();

		j = json::parse(input.c_str());
	}



	for (auto& jo : j)
	{
		// Is Probe?
		if (jo["TYPE"] == "LIGHT_PROBE")
		{
			Ptr<LightProbeNode> probe(new LightProbeNode());

			if (jo["POSITION"].is_array() && jo["POSITION"].size() > 2)
			{
				glm::vec3 val(0.0f);
				val.x = jo["POSITION"][0].get<float>();
				val.y = jo["POSITION"][1].get<float>();
				val.z = jo["POSITION"][2].get<float>();
				probe->SetTranslate(val);
			}

			if (jo["RADIUS"].is_number_float())
			{
				float radius = 0.0f;
				radius = jo["RADIUS"].get<float>();
				probe->SetRadius(radius);
			}

			probe->UpdateRenderLightProbe();
			scene->AddNode(probe);
		}
		else if (jo["TYPE"] == "IRRADIANCE_VOLUME") // Is Volume?
		{
			Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
			glm::vec3 start(0.0f), size(0.0f);
			glm::ivec3 count;

			if (jo["START"].is_array() && jo["START"].size() > 2)
			{
				start.x = jo["START"][0].get<float>();
				start.y = jo["START"][1].get<float>();
				start.z = jo["START"][2].get<float>();
			}

			if (jo["SIZE"].is_array() && jo["SIZE"].size() > 2)
			{
				size.x = jo["SIZE"][0].get<float>();
				size.y = jo["SIZE"][1].get<float>();
				size.z = jo["SIZE"][2].get<float>();
			}


			if (jo["COUNT"].is_array() && jo["COUNT"].size() > 2)
			{
				count.x = jo["COUNT"][0].get<int32_t>();
				count.y = jo["COUNT"][1].get<int32_t>();
				count.z = jo["COUNT"][2].get<int32_t>();
			}

			if (jo["ATTEN"].is_array() && jo["ATTEN"].size() > 2)
			{
				glm::vec3 val(0.0f);
				val.x = jo["ATTEN"][0].get<float>();
				val.y = jo["ATTEN"][1].get<float>();
				val.z = jo["ATTEN"][2].get<float>();
				irVolume->SetAtten(val);
			}

			irVolume->SetVolume(start, size, count);
			irVolume->UpdateIrradianceVolumeNode();
			scene->AddNode(irVolume);
		}

	}



	return true;
}
