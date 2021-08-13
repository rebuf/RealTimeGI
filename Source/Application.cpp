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




#include "Application.h"
#include "Core/GISystem.h"
#include "AppWindow.h"
#include "AppUser.h"
#include "Render/Renderer.h"
#include "Importers/GLTFImporter.h"
#include "Importers/RTGIImporter.h"

#include "Scene/Scene.h"
#include "Scene/LightProbeNode.h"
#include "Scene/IrradianceVolumeNode.h"


#include "GLFW/glfw3.h"






UniquePtr<Application> Application::mInstance = UniquePtr<Application>(new Application());






Application::Application()
	: mAppTime(0.0f)
	, mDeltaTime(0.0f)
{
	// The Application Name.
	mAppName = "RealTimeGI";

}


Application::~Application()
{

}


void Application::ProcessArg(int argc, char** argv)
{

}


void Application::Initialize()
{
	LOGI("Initialize App...");

	glfwInit();

	// The Window.
	SetupWindow();
	 
	// The Renderer.
	mRenderer = UniquePtr<Renderer>( new Renderer() );
	mRenderer->Initialize();

	// User.
	mAppUser = UniquePtr<AppUser>(new AppUser());
	mAppUser->Initialize();

	// ...
	mMainScene = Ptr<Scene>( new Scene() );
	mMainScene->GetCamera().SetAspect(mAppWnd->GetFrameBufferAspect());
	mMainScene->ComputeBounds();

	GLTFImporter::Import(mMainScene.get(), RESOURCES_DIRECTORY "Models/Sponza/Sponza.gltf");
	mMainScene->ResetView(); // Reset view.
	mMainScene->GetGlobal().SetSunColor( glm::vec3(1.0f, 0.9f, 0.85f) );
	mMainScene->GetGlobal().SetSunPower(4.0f);
	mMainScene->GetGlobal().SetSunDir( glm::normalize(glm::vec3(-1.0f, -1.0f, -3.0f)) );

	mMainScene->GetGlobal().isLightProbeEnabled = true;
	mMainScene->GetGlobal().isLightProbeHelpers = true;
	mMainScene->GetGlobal().isLightProbeVisualize = true;


	// ..............................................................
	// ..............................................................
#if 0
	float h0 = 100.0f;
	float h1 = 280.0f;
	float r0 = 240.0f;
	float r2 = 340.0f;


	std::vector<glm::vec4> probesPos = {
		// Bottom - Middle
		glm::vec4( 600.0f, 0.0f, h0, r0),
		glm::vec4( 500.0f, 0.0f, h0, r0),
		glm::vec4( 400.0f, 0.0f, h0, r0),
		glm::vec4( 300.0f, 0.0f, h0, r0),
		glm::vec4( 200.0f, 0.0f, h0, r0),
		glm::vec4( 100.0f, 0.0f, h0, r0),
		glm::vec4( 000.0f, 0.0f, h0, r0),
		glm::vec4(-200.0f, 0.0f, h0, r0),
		glm::vec4(-300.0f, 0.0f, h0, r0),
		glm::vec4(-400.0f, 0.0f, h0, r0),
		glm::vec4(-500.0f, 0.0f, h0, r0),
		glm::vec4(-600.0f, 0.0f, h0, r0),

#if 1
		// Top - Middle
		glm::vec4( 600.0f, 0.0f, h1, r0),
		glm::vec4( 500.0f, 0.0f, h1, r0),
		glm::vec4( 400.0f, 0.0f, h1, r0),
		glm::vec4( 300.0f, 0.0f, h1, r0),
		glm::vec4( 200.0f, 0.0f, h1, r0),
		glm::vec4( 100.0f, 0.0f, h1, r0),
		glm::vec4( 000.0f, 0.0f, h1, r0),
		glm::vec4(-200.0f, 0.0f, h1, r0),
		glm::vec4(-300.0f, 0.0f, h1, r0),
		glm::vec4(-400.0f, 0.0f, h1, r0),
		glm::vec4(-500.0f, 0.0f, h1, r0),
		glm::vec4(-600.0f, 0.0f, h1, r0),

		// Bottom...........................
		// Left
		glm::vec4( 600.0f,-220.0f, h0, r0),
		glm::vec4( 500.0f,-220.0f, h0, r0),
		glm::vec4( 400.0f,-220.0f, h0, r0),
		glm::vec4( 300.0f,-220.0f, h0, r0),
		glm::vec4( 200.0f,-220.0f, h0, r0),
		glm::vec4( 100.0f,-220.0f, h0, r0),
		glm::vec4( 000.0f,-220.0f, h0, r0),
		glm::vec4(-200.0f,-220.0f, h0, r0),
		glm::vec4(-300.0f,-220.0f, h0, r0),
		glm::vec4(-400.0f,-220.0f, h0, r0),
		glm::vec4(-500.0f,-220.0f, h0, r0),
		glm::vec4(-600.0f,-220.0f, h0, r0),

		// Left
		glm::vec4( 600.0f, 220.0f, h0, r0),
		glm::vec4( 500.0f, 220.0f, h0, r0),
		glm::vec4( 400.0f, 220.0f, h0, r0),
		glm::vec4( 300.0f, 220.0f, h0, r0),
		glm::vec4( 200.0f, 220.0f, h0, r0),
		glm::vec4( 100.0f, 220.0f, h0, r0),
		glm::vec4( 000.0f, 220.0f, h0, r0),
		glm::vec4(-200.0f, 220.0f, h0, r0),
		glm::vec4(-300.0f, 220.0f, h0, r0),
		glm::vec4(-400.0f, 220.0f, h0, r0),
		glm::vec4(-500.0f, 220.0f, h0, r0),
		glm::vec4(-600.0f, 220.0f, h0, r0),
		
		// Top..............................
		// Left
		glm::vec4( 600.0f, 220.0f, h1, r0),
		glm::vec4( 500.0f, 220.0f, h1, r0),
		glm::vec4( 400.0f, 220.0f, h1, r0),
		glm::vec4( 300.0f, 220.0f, h1, r0),
		glm::vec4( 200.0f, 220.0f, h1, r0),
		glm::vec4( 100.0f, 220.0f, h1, r0),
		glm::vec4( 000.0f, 220.0f, h1, r0),
		glm::vec4(-200.0f, 220.0f, h1, r0),
		glm::vec4(-300.0f, 220.0f, h1, r0),
		glm::vec4(-400.0f, 220.0f, h1, r0),
		glm::vec4(-500.0f, 220.0f, h1, r0),
		glm::vec4(-600.0f, 220.0f, h1, r0),

		// Left
		glm::vec4( 600.0f,-220.0f, h1, r0),
		glm::vec4( 500.0f,-220.0f, h1, r0),
		glm::vec4( 400.0f,-220.0f, h1, r0),
		glm::vec4( 300.0f,-220.0f, h1, r0),
		glm::vec4( 200.0f,-220.0f, h1, r0),
		glm::vec4( 100.0f,-220.0f, h1, r0),
		glm::vec4( 000.0f,-220.0f, h1, r0),
		glm::vec4(-200.0f,-220.0f, h1, r0),
		glm::vec4(-300.0f,-220.0f, h1, r0),
		glm::vec4(-400.0f,-220.0f, h1, r0),
		glm::vec4(-500.0f,-220.0f, h1, r0),
		glm::vec4(-600.0f,-220.0f, h1, r0)
#endif
	};


	for (uint32_t i = 0; i < probesPos.size(); ++i)
	{
		Ptr<LightProbeNode> tmp0(new LightProbeNode());
		tmp0->SetTranslate(probesPos[i]);
		tmp0->SetRadius(probesPos[i].w);
		tmp0->UpdateRenderLightProbe();
		mMainScene->AddNode(tmp0);
	}
#endif



	// ..............................................................
	// ..............................................................


#if 0

	Box sceneBounds = mMainScene->ComputeBounds();
	glm::vec3 vSize = sceneBounds.Extent() * 1.4f;
	glm::vec3 vStart = sceneBounds.Center() - vSize * 0.5f;

	int32_t cx = 15, cy = 8, cz = 5;
	float vRadius = glm::max(vSize.x, glm::max(vSize.y, vSize.z)) / (float)glm::min(cx, glm::min(cy, cz));
	vRadius *= 0.6;

	for (int32_t z = 0; z < cz; ++z)
	{
		for (int32_t y = 0; y < cy; ++y)
		{
			for (int32_t x = 0; x < cx; ++x)
			{
				glm::vec3 f = vStart + vSize * glm::vec3((float)x / (float)cx,
					(float)y / (float)cy, (float)z / (float)cz);

				Ptr<LightProbeNode> probe(new LightProbeNode());
				probe->SetTranslate(f);
				probe->SetRadius(vRadius);
				probe->UpdateRenderLightProbe();
				mMainScene->AddNode(probe);
			}
		}
	}
#endif

	// ..............................................................
	// ..............................................................


#if 1
	Box sceneBounds = mMainScene->ComputeBounds();
	glm::vec3 vSize = glm::vec3(1400.0, 250.0, 220.0f);
	glm::vec3 vStart = sceneBounds.Center() - glm::vec3(vSize.x * 0.5, vSize.y * 0.5, 340.0f);
	glm::ivec3 vCount(18, 4, 4);



#define MULTI_VOLUME 1

	// Bottom
	{

		// 0 ---------
		{
			Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
			irVolume->SetVolume(vStart, vSize, vCount);
			irVolume->SetAtten(glm::vec3(0.0f, 0.2f, 0.2f));
			irVolume->UpdateIrradianceVolumeNode();
			mMainScene->AddNode(irVolume);
		}

#if MULTI_VOLUME
		// 1 ---------
		{
			glm::vec3 gSize = glm::vec3(1400.0, 190.0, 210.0f);
			glm::vec3 gStart = sceneBounds.Center() - glm::vec3(gSize.x * 0.5, gSize.y * 0.5, 340.0f);
			gStart += glm::vec3(0.0f, vSize.y - 35, 0.0f);

			Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
			irVolume->SetVolume(gStart, gSize, vCount);
			irVolume->SetAtten(glm::vec3(0.0f, 0.2f, 0.2f));
			irVolume->UpdateIrradianceVolumeNode();
			mMainScene->AddNode(irVolume);
		}


		// 2 ---------
		{
			glm::vec3 gSize = glm::vec3(1400.0, 190.0, 210.0f);
			glm::vec3 gStart = sceneBounds.Center() - glm::vec3(gSize.x * 0.5, gSize.y * 0.5, 340.0f);
			gStart += glm::vec3(0.0f, -vSize.y + 35, 0.0f);

			Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
			irVolume->SetVolume(gStart, gSize, vCount);
			irVolume->SetAtten(glm::vec3(0.0f, 0.2f, 0.2f));
			irVolume->UpdateIrradianceVolumeNode();
			mMainScene->AddNode(irVolume);
		}

#endif
	}
	

#if MULTI_VOLUME
	// Top
	{

		// 0 ---------
		{
			glm::vec3 gSize = glm::vec3(1400.0, 250.0, 220.0f);
			glm::vec3 gStart = sceneBounds.Center() - glm::vec3(vSize.x * 0.5, vSize.y * 0.5, 340.0f);
			gStart += glm::vec3(0.0f, 0.0f, vSize.z);

			Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
			irVolume->SetVolume(gStart, gSize, vCount);
			irVolume->SetAtten(glm::vec3(0.0f, 0.2f, 0.4f));
			irVolume->UpdateIrradianceVolumeNode();
			mMainScene->AddNode(irVolume);
		}


		// 1 ---------
		{
			glm::vec3 gSize = glm::vec3(1400.0, 190.0, 240.0f);
			glm::vec3 gStart = sceneBounds.Center() - glm::vec3(vSize.x * 0.5, vSize.y * 0.5, 340.0f);
			gStart += glm::vec3(0.0f, vSize.y, vSize.z - 10.0);

			Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
			irVolume->SetVolume(gStart, gSize, vCount);
			irVolume->SetAtten(glm::vec3(0.0f, 0.2f, 0.0f));
			irVolume->UpdateIrradianceVolumeNode();
			mMainScene->AddNode(irVolume);
		}


		// 2 ---------
		{
			glm::vec3 gSize = glm::vec3(1400.0, 190.0, 240.0f);
			glm::vec3 gStart = sceneBounds.Center() - glm::vec3(vSize.x * 0.5, vSize.y * 0.5, 340.0f);
			gStart += glm::vec3(0.0f, -vSize.y + 62.0f, vSize.z - 10.0f);

			Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
			irVolume->SetVolume(gStart, gSize, vCount);
			irVolume->SetAtten(glm::vec3(0.0f, 0.2f, 0.0f));
			irVolume->UpdateIrradianceVolumeNode();
			mMainScene->AddNode(irVolume);
		}
	}



	// Top - Top
	{
		glm::vec3 gSize = glm::vec3(1400.0, 250.0, 220.0f);
		glm::vec3 gStart = sceneBounds.Center() - glm::vec3(vSize.x * 0.5, vSize.y * 0.5, 340.0f);
		gStart += glm::vec3(0.0f, 0.0f, vSize.z * 2.0f);
		glm::ivec3 gCount(8, 2, 2);

		Ptr<IrradianceVolumeNode> irVolume(new IrradianceVolumeNode());
		irVolume->SetVolume(gStart, gSize, gCount);
		irVolume->SetAtten(glm::vec3(0.0f, 0.2f, 0.4f));
		irVolume->UpdateIrradianceVolumeNode();
		mMainScene->AddNode(irVolume);
	}
#endif


#endif


}


void Application::ReplaceScene(Ptr<Scene> scene)
{
	if (mMainScene)
	{
		mRenderer->WaitForIdle();
		mMainScene->Destroy();
		mMainScene.reset();
	}

	if (!scene)
		return;

	mMainScene = scene;
	mMainScene->ComputeBounds();
	mMainScene->Start();
}


void Application::SetupWindow()
{
	mAppWnd = UniquePtr<AppWindow>(
		AppWindow::Create("RealTimeGI", glm::ivec2(1366, 768))
	);

}


int32_t Application::Run()
{
	// Start...
	mMainScene->Start();


	// Main Loop...
	while (!mAppWnd->ShouldClose())
	{
		// Time...
		mDeltaTime = (float)glfwGetTime() - mAppTime;
		mAppTime = (float)glfwGetTime();

		// Window System Events...
		glfwPollEvents();

		// ...
		Update();

		// Render...
		Render();

	}


	return 0;
}


void Application::Destroy()
{
	mAppUser->Destroy();

	// Wait for the renderer to be idle.
	mRenderer->WaitForIdle();

	// Destroy the main scene.
	if (mMainScene)
	{
		mMainScene->Destroy();
		mMainScene.reset();
	}

	// Destroy Renderer.
	mRenderer->Destroy();
}


void Application::Update()
{
	mAppUser->Update(mDeltaTime, mMainScene.get());


	// Update Scene...
	if (mMainScene)
	{
		mMainScene->Update(mDeltaTime);
	}
}


void Application::Render()
{
	// Don't Render while minimized.
	if (mAppWnd->IsMinimized())
		return;


	// Begin Rendering...
	mRenderer->BeginRender(mMainScene.get());

	// Render...
	mRenderer->Render();

	// End Rendering...
	mRenderer->EndRender();
}
