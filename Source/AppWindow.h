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
#include "Core/Delegate.h"
#include "glm/vec2.hpp"


#include <string>




struct GLFWwindow;




// AppWindow:
//   - Handle a single window in the application.
//
class AppWindow
{
private:
	// Construct.
	AppWindow();

public:
	// Destruct.
	~AppWindow();

	// Create a new window.
	static AppWindow* Create(const std::string& title, const glm::ivec2& size);

	// Return true if the window should be closed.
	bool ShouldClose();

	// Return GLFW window handle.
	inline GLFWwindow* GetHandle() { return glfw_window; }

	// Return window framebuffer size.
	glm::vec2 GetFrameBufferSize();

	// Return window framebuffer aspect ratio.
	float GetFrameBufferAspect();

	// Return true if the window is minimized.
	bool IsMinimized();

private:
	// GLFW Window Callbacks...
	static void glfw_FramebufferResizeCallback(GLFWwindow* wnd, int x, int y);
	static void glfw_SetDropCallback(GLFWwindow* wnd, int num, const char** paths);

	// Called when the window fraebuffer get resized.
	void FramebufferResize(int32_t x, int32_t y);

	// Called whe file path is darg & droped on window.
	void FileDrop(int32_t num, const char** paths);

private:
	// GLFW Window Handle.
	GLFWwindow* glfw_window;

public:
	// Event exectued when file drag drop happen.
	Delegate<const std::string&> FileDropEvent;
};


