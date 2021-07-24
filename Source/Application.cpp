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
#include "AppWindow.h"
#include "Render/Renderer.h"
#include "Scene/Scene.h"
#include "Importers/GLTFImporter.h"
#include "Core/GISystem.h"


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
	mMainScene = UniquePtr<Scene>( new Scene() );
	mMainScene->GetCamera().SetAspect(mAppWnd->GetFrameBufferAspect());

	GLTFImporter::Import(mMainScene.get(), RESOURCES_DIRECTORY "Models/Sponza/Sponza.gltf");
	mMainScene->ResetView(); // Reset view.
	mMainScene->GetGlobalSettings().sunColor = glm::vec4(1.0f);
	mMainScene->GetGlobalSettings().sunPower = 1.0f;
	mMainScene->GetGlobalSettings().sunDir = glm::normalize(glm::vec3(-1.0f));

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
