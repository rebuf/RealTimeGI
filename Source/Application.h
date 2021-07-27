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

#include <string>





class AppWindow;
class AppUser;
class Scene;
class Renderer;






// Application:
//    - The singleton class that manage the entire application.
//
class Application
{
private:
	// Construct
	Application();

public:
  // Destruct.
	~Application();

	// Return the applicaiton singleton.
	inline static Application& Get() { return *mInstance; }

	// Process application argument.
	void ProcessArg(int argc, char** argv);

	// Initialize Application.
	void Initialize();

	// Initialize and Run the application.
	int32_t Run();

	// Cleanup applicaiton.
	void Destroy();

	// Return the application's main scene.
	inline Scene* GetMainScene() const { return mMainScene.get(); }

	// Return the application name.
	inline const std::string& GetAppName() const { return mAppName; }

	// Get Application's main window.
	inline AppWindow* GetMainWindow() { return mAppWnd.get(); }

	// Return the application renderer.
	inline Renderer* GetRenderer() { return mRenderer.get(); }

	// Return the application time.
	inline float GetAppTime() { return mAppTime; }

	// Return the application time.
	inline float GetDeltaTime() { return mDeltaTime; }

private:
	// Initialize the window.
	void SetupWindow();

	// Update application.
	void Update();

	// Render a single frame to the application's window.
	void Render();

private:
	// Application Singleton instance.
	static UniquePtr<Application> mInstance;

	// The application main window.
	UniquePtr<AppWindow> mAppWnd;

	// Time from the start of the app.
	float mAppTime;

	// The delta time of the last frame.
	float mDeltaTime;

	// The application main scene.
	UniquePtr<Scene> mMainScene;

	// The application's name.
	std::string mAppName;

	// The application's renderer.
	UniquePtr<Renderer> mRenderer;

	//
	UniquePtr<AppUser> mAppUser;
};



