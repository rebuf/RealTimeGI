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






#include "AppUser.h"
#include "Application.h"
#include "AppWindow.h"
#include "Core/GISystem.h"
#include "Scene/Scene.h"
#include "Scene/Node.h"
#include "Scene/LightProbeNode.h"
#include "Scene/IrradianceVolumeNode.h"

#include "Core/UI/imGUI/imgui.h"
#include "GLFW/glfw3.h"
#include "Importers/GLTFImporter.h"

#include "glm/gtc/type_ptr.hpp"




static float g_KeyW = 0.0f;
static float g_KeyA = 0.0f;
static float g_KeyS = 0.0f;
static float g_KeyD = 0.0f;
static float g_KeyQ = 0.0f;
static float g_KeyE = 0.0f;
static float g_KeyUp = 0.0f;
static float g_KeyDown = 0.0f;
static float g_KeyMouseRight = 0.0f;

static float g_KeyMouseLeft = 0.0f;
static bool  g_KeyMouseLeft_Released = false;

static float g_Key1 = 0.0f;
static float g_Key2 = 0.0f;
static float g_Key3 = 0.0f;
static float g_Key4 = 0.0f;

static bool g_Key1_Released = false;
static bool g_Key2_Released = false;
static bool g_Key3_Released = false;
static bool g_Key4_Released = false;



static int32_t g_NavMode = 0;
static glm::vec2 g_MousePos(0.0f);
static glm::vec2 g_MouseOffset(0.0f);

static float g_Velocity = 0.0f;





#define KEY_STATE_UPDATE(Key, KeyReleased, KeyName) \
	{ \
		bool wasDown = (Key == 1.0f); \
		Key = glfwGetKey(window, KeyName) == GLFW_PRESS ? 1.0f : 0.0f; \
		KeyReleased = (wasDown && Key == 0.0f) ? true : false; \
	}







AppUser::AppUser()
	: target(0.0f, 0.0f, 0.0f)
	, eye(1.0f)
	, up(0.0f, 0.0f, 1.0f)
{

}


AppUser::~AppUser()
{

}


void AppUser::Initialize()
{
	// Bind File Drop to load scene..
	Application::Get().GetMainWindow()->FileDropEvent.BindMemberRaw(this, &AppUser::LoadNewScene);

}


void AppUser::Destroy()
{
	// Clear Binding.
	Application::Get().GetMainWindow()->FileDropEvent.Reset();

}


void AppUser::MatchCamera(Scene* scene)
{
	Camera& camera = Application::Get().GetMainScene()->GetCamera();

	scene->ResetView();

	target = camera.GetViewTarget();
	eye = camera.GetViewPos();
	up = camera.GetUp();
}


void AppUser::LoadNewScene(const std::string& path)
{
	LOGI("Loading scene... (%s)", path.c_str());

	if (path.empty())
		return;


	std::string ext = GISystem::GetFileExtension(path);
	bool isSuccess = false;
	Ptr<Scene> scene;


	// Load based on extension...
	if (GLTFImporter::IsSupported(path))
	{
		scene = Ptr<Scene>(new Scene());
		isSuccess = GLTFImporter::Import(scene.get(), path);
	}


	// Failed to load?
	if (!isSuccess)
		return;

	scene->GetCamera().SetAspect(Application::Get().GetMainWindow()->GetFrameBufferAspect());
	scene->GetGlobal().SetSunColor(glm::vec3(1.0f, 0.9f, 0.85f));
	scene->GetGlobal().SetSunPower(4.0f);
	scene->GetGlobal().SetSunDir(glm::normalize(glm::vec3(-1.0f, -1.0f, -3.0f)));

	scene->ResetView(); // Reset view.
	MatchCamera(scene.get());

	Application::Get().ReplaceSceen(scene);
}


void AppUser::Update(float deltaTime, Scene* scene)
{
	UpdateInput(deltaTime);
	UpdateNav(deltaTime);

	Camera& camera = scene->GetCamera();
	camera.SetViewTarget(target);
	camera.SetViewPos(eye);
	camera.RecomputeUp();

	// TEMP------------------------------------------
	if (g_KeyMouseLeft_Released)
		SceneSelect(scene);

	if (g_Key1_Released)
		scene->GetGlobal().isLightProbeEnabled = !scene->GetGlobal().isLightProbeEnabled;

	if (g_Key2_Released)
		scene->GetGlobal().isLightProbeHelpers = !scene->GetGlobal().isLightProbeHelpers;

	if (g_Key3_Released)
		scene->GetGlobal().isLightProbeVisualize = !scene->GetGlobal().isLightProbeVisualize;

	// TEMP------------------------------------------


}
	

void AppUser::UpdateInput(float deltaTime)
{
	auto appWindow = Application::Get().GetMainWindow();
	auto window = appWindow->GetHandle();

	g_KeyW = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? 1.0f : 0.0f;
	g_KeyA = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? 1.0f : 0.0f;
	g_KeyS = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? 1.0f : 0.0f;
	g_KeyD = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? 1.0f : 0.0f;
	g_KeyQ = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? 1.0f : 0.0f;
	g_KeyE = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? 1.0f : 0.0f;
	g_KeyUp = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ? 1.0f : 0.0f;
	g_KeyDown = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ? 1.0f : 0.0f;

	KEY_STATE_UPDATE(g_Key1, g_Key1_Released, GLFW_KEY_1);
	KEY_STATE_UPDATE(g_Key2, g_Key2_Released, GLFW_KEY_2);
	KEY_STATE_UPDATE(g_Key3, g_Key3_Released, GLFW_KEY_3);
	KEY_STATE_UPDATE(g_Key4, g_Key4_Released, GLFW_KEY_4);

	g_KeyMouseRight = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ? 1.0f : 0.0f;

	g_KeyMouseLeft_Released = false;
	if (g_KeyMouseLeft > 0.0f)
	{
		g_KeyMouseLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? 1.0f : 0.0f;

		if (g_KeyMouseLeft < 1.0f)
		{
			g_KeyMouseLeft_Released = true;
		}
	}
	else
	{
		g_KeyMouseLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ? 1.0f : 0.0f;
	}

	
	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	g_MouseOffset = glm::vec2((float)mx - g_MousePos.x, (float)my - g_MousePos.y);
	g_MousePos = glm::vec2((float)mx, (float)my);
}


void AppUser::UpdateNav(float deltaTime)
{
	static const float FLY_SPEED = 5.0f;

	float flyspeed = deltaTime * FLY_SPEED * 100.0f;
	glm::vec3 dir = glm::normalize(target - eye);
	glm::vec3 right = glm::normalize(glm::cross(dir, up));

	if (g_NavMode == 0)
	{
		// Rotation.
		glm::mat4 rot =
			  glm::rotate(glm::mat4(1.0f), g_MouseOffset.x * g_KeyMouseRight * 0.002f, up)
			* glm::rotate(glm::mat4(1.0f), g_MouseOffset.y * g_KeyMouseRight * -0.002f, right);

		dir = rot * glm::vec4(dir, 0.0f);

		// WASD Movement...
		eye += dir * g_KeyW * flyspeed;
		eye -= dir * g_KeyS * flyspeed;
		eye += right * g_KeyA * flyspeed;
		eye -= right * g_KeyD * flyspeed;
		target = eye + dir;
	}
	else
	{

	}
}


void AppUser::SceneSelect(Scene* scene)
{
	auto appWindow = Application::Get().GetMainWindow();

	double mx, my;
	glfwGetCursorPos(appWindow->GetHandle(), &mx, &my);

	glm::vec2 fboSize = appWindow->GetFrameBufferSize();
	glm::vec2 ndc(mx / fboSize.x, my / fboSize.y);

	glm::mat4 view = scene->GetCamera().GetViewTransform();
	glm::mat4 proj = scene->GetCamera().GetProjection();
	glm::mat4 viewprojInv = glm::inverse(proj * view);


	glm::vec4 deproj0 = viewprojInv * glm::vec4(ndc * 2.0f - 1.0f, 0.0f, 1.0f);
	glm::vec4 deproj1 = viewprojInv * glm::vec4(ndc * 2.0f - 1.0f, 1.0f, 1.0f);
	deproj0 /= deproj0.w;
	deproj1 /= deproj1.w;

	glm::vec3 rayPos = deproj0;
	glm::vec3 rayDir = glm::normalize(deproj1 - deproj0);

	scene->SetSelectedLight(nullptr);

	// Lights & Light Probes...
	for (auto& node : scene->GetLights())
	{
		if (node->GetType() == ENodeType::LightProbe)
		{
			LightProbeNode* probe = static_cast<LightProbeNode*>(node);
			glm::vec3 ppos = probe->GetPosition();

			glm::vec3 vec = (ppos - rayPos);
			float dot = glm::dot(vec, rayDir);
			glm::vec3 pp = rayPos + rayDir * dot;

			if (dot > 0.0f && glm::length(pp - ppos) < 5.0)
			{
				scene->SetSelectedLight(node);
				LOGW("PROBE SELECTED.");
				break;
			}
		}
	}



}


void AppUser::UpdateImGui()
{
	Scene* scene = Application::Get().GetMainScene();


	ImGui::Begin("Vulkan Render");
	ImGui::Text("%.3f ms/Frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Separator();


	// -----
	// SUN
	{
		bool isUpdate = false;
		float yaw = 0.0f, pitch = 0.0f;
		Transform::DirectionToPolar(scene->GetGlobal().GetSunDir(), pitch, yaw);
		yaw = glm::degrees(yaw);
		pitch = glm::degrees(-pitch);

		glm::vec3 color = scene->GetGlobal().GetSunColor();
		float power = scene->GetGlobal().GetSunPower();

		

		ImGui::Text("SUN");
		isUpdate = ImGui::SliderFloat("YAW", &yaw, 0.0f, 359.0f, "%.3f") || isUpdate;
		isUpdate = ImGui::SliderFloat("PITCH", &pitch, 0.0f, 88.5f)  || isUpdate;
		isUpdate = ImGui::ColorEdit3("COLOR", glm::value_ptr(color)) || isUpdate;
		isUpdate = ImGui::SliderFloat("POWER", &power, 0.0f, 20.0f)  || isUpdate;



		if (isUpdate)
		{
			yaw = glm::radians(yaw);
			pitch = glm::radians(-pitch);

			scene->GetGlobal().SetSunColor(color);
			scene->GetGlobal().SetSunPower(power);
			scene->GetGlobal().SetSunDir(Transform::PolarToDirection(pitch, yaw));
			scene->GetGlobal().SetDirtyFlag(ESceneGlobalDirtyFlag::DirtySun);
		}
	}

	// -----

	ImGui::Separator();

	{
		//
		if (ImGui::Button("Update."))
		{
			UpdateProbes();
		}

		// 
		if (ImGui::Button("Save Light Data."))
		{

		}
	}

	ImGui::End();
}


void AppUser::UpdateProbes()
{
	Scene* scene = Application::Get().GetMainScene();


	// Lights & Light Probes...
	for (auto& node : scene->GetLights())
	{
		if (node->GetType() == ENodeType::LightProbe)
		{
			LightProbeNode* probe = static_cast<LightProbeNode*>(node);
			probe->SetDirty();
		}
		else if (node->GetType() == ENodeType::IrradianceVolume)
		{
			IrradianceVolumeNode* volume = static_cast<IrradianceVolumeNode*>(node);
			volume->SetDirty();
		}
	}
}
