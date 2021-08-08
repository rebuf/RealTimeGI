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




#include "AppWindow.h"


#include "GLFW/glfw3.h"




AppWindow::AppWindow()
	: glfw_window(nullptr)
{
	
}


AppWindow::~AppWindow()
{

}


AppWindow* AppWindow::Create(const std::string& title, const glm::ivec2& size)
{
	AppWindow* newWnd = new AppWindow();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL Context.
	newWnd->glfw_window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

	// Set user pointer to be mapped with our window.
	glfwSetWindowUserPointer(newWnd->glfw_window, newWnd);

	// Callbacks...
	glfwSetFramebufferSizeCallback(newWnd->glfw_window, &AppWindow::glfw_FramebufferResizeCallback);
	glfwSetDropCallback(newWnd->glfw_window, &AppWindow::glfw_SetDropCallback);

	return newWnd;
}


void AppWindow::glfw_FramebufferResizeCallback(GLFWwindow* wnd, int x, int y)
{
	AppWindow* appWnd = (AppWindow*)glfwGetWindowUserPointer(wnd);
	appWnd->FramebufferResize(x, y);
}

void AppWindow::glfw_SetDropCallback(GLFWwindow* wnd, int num, const char** paths)
{
	AppWindow* appWnd = (AppWindow*)glfwGetWindowUserPointer(wnd);
	appWnd->FileDrop(num, paths);
}



void AppWindow::FramebufferResize(int32_t x, int32_t y)
{

}


void AppWindow::FileDrop(int32_t num, const char** paths)
{
	if (num < 1)
		return;

	std::string path = paths[0];

	if (FileDropEvent.IsValid())
		FileDropEvent.Execute(path);
}


bool AppWindow::ShouldClose()
{
	return glfwWindowShouldClose(glfw_window);
}


glm::vec2 AppWindow::GetFrameBufferSize()
{
	int width, height;
	glfwGetFramebufferSize(glfw_window, &width, &height);
	return glm::vec2((float)width, (float)height);
}


float AppWindow::GetFrameBufferAspect()
{
	glm::vec2 size = GetFrameBufferSize();
	return size.x / size.y;
}


bool AppWindow::IsMinimized()
{
	return glfwGetWindowAttrib(glfw_window, GLFW_ICONIFIED) == GLFW_TRUE;
}
