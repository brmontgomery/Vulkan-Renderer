#pragma once

#include "PSIM/Graphics/RenderAPI/RendererCommands.h"
#include "PSIM/Graphics/RenderAPI/RendererAPI.h"

#include "PSIM/Graphics/Camera/Camera.h"
#include "PSIM/Graphics/Components/VertexArray.h"

class Renderer
{
public:
	static void Init();
	static void Shutdown();

	static void OnWindowResize(uint32_t width, uint32_t height);

	static void BeginScene(Camera& camera);
	static void EndScene();

	static void Submit(const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

	inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
private:
	struct SceneData
	{
		glm::mat4 ViewProjectionMatrix;
	};

	static Scope<SceneData> s_SceneData;
};

