//core files
#include "PSIMPCH.h"
#include "Application.h"

#include "PSIM/Graphics/Renderer.h"
#include "PSIM/Graphics/Shader.h"
#include "PSIM/TimeStep.h"

#include "GLFW/glfw3.h"

RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

//vulkan entry point
//--------------------------------------------------------------------------------------------------------------------------------

Application* Application::s_AppInstance = nullptr;

Application::Application()
{
	PSIM_PROFILE_FUNCTION();

	PSIM_ASSERT(!s_AppInstance, "Application already exists!");
	s_AppInstance = this;
	m_Window = WindowPrototype::Create();
	m_Window->SetEventCallback(PSIM_BIND_EVENT_FN(Application::OnEvent));

	//init the renderer
	Renderer::Init();

	//create the shader Library to hold all of our shaders
	Ref<ShaderLibrary>shaderLib = ShaderLibrary::Create();

	//load a shader
	{
		PSIM_PROFILE_SCOPE("Load Triangle Shader");
		auto triangleShader = shaderLib->load("assets/Shaders/TriangleShaderVert.spv", "assets/Shaders/TriangleShaderFrag.spv");
	}


	//m_ImGuiLayer = new ImGuiLayer();
	//PushOverlay(m_ImGuiLayer);
}

Application::~Application()
{
	PSIM_PROFILE_FUNCTION();

	Renderer::Shutdown();
}

void Application::PushLayer(Layer* layer)
{
	PSIM_PROFILE_FUNCTION();

	m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void Application::PushOverlay(Layer* layer)
{
	PSIM_PROFILE_FUNCTION();

	m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
} 

void Application::OnEvent(Event& e)
{
	PSIM_PROFILE_FUNCTION();

	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(PSIM_BIND_EVENT_FN(Application::OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(PSIM_BIND_EVENT_FN(Application::OnWindowResize));

	for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
	{
		(*it)->OnEvent(e);
		if (e.Handled)
			break;
	}
}

void Application::Run()
{
	PSIM_PROFILE_FUNCTION();

	while (m_Running)
	{
		PSIM_PROFILE_SCOPE("RunLoop");

		float time = (float)glfwGetTime();
		Timestep timestep = time - m_LastFrameTime;
		m_LastFrameTime = time;
		std::cout << m_LastFrameTime << std::endl;

		if (!m_Minimized)
		{
			
			{
				PSIM_PROFILE_SCOPE("LayerStack OnUpdate");

				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);
			}

			/*
			m_ImGuiLayer->Begin();
			{
				PSIM_PROFILE_SCOPE("LayerStack OnImGuiRender");

				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();
			*/
		}

		m_Window->OnUpdate();
	}
}

bool Application::OnWindowClose(WindowCloseEvent& e)
{
	m_Running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e)
{
	PSIM_PROFILE_FUNCTION();

	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		m_Minimized = true;
		return false;
	}

	m_Minimized = false;
	Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

	return false;
}