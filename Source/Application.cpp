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

#include "Scene/Scene.h"
#include "Scene/LightProbeNode.h"


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

	// ...
	mAppUser = UniquePtr<AppUser>(new AppUser());

	mMainScene = UniquePtr<Scene>( new Scene() );
	mMainScene->GetCamera().SetAspect(mAppWnd->GetFrameBufferAspect());

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

	// Irrdiance Volume.
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



	mAppUser->MatchCamera(mMainScene.get());


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


		GISystem::Sleep(24);
		LOGW("FPS: %f", 1.0F / (float)mDeltaTime);
	}


	return 0;
}


void Application::Destroy()
{
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
