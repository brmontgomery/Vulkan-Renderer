#include "PSIMPCH.h"
#include "PSIM/Graphics/Renderer2D.h"

#include "PSIM/Graphics/VertexArray.h"
#include "PSIM/Graphics/Shader.h"
#include "PSIM/Graphics/RenderAPI/RendererCommands.h"

#include <glm/gtc/matrix_transform.hpp>

struct Renderer2DStorage
{
	Ref<VertexArray> QuadVertexArray;
	Ref<LinkedShader> TextureShader;
	Ref<Texture2D> WhiteTexture;
};

static Renderer2DStorage* s_Data;

void Renderer2D::Init()
{
	PSIM_PROFILE_FUNCTION();

	s_Data = new Renderer2DStorage();
	s_Data->QuadVertexArray = VertexArray::Create();

	float squareVertices[5 * 4] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};

	Ref<VertexBuffer> squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
	squareVB->SetLayout({
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	});
	s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

	uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
	Ref<IndexBuffer> squareIB = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
	s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

	s_Data->WhiteTexture = Texture2D::Create(1, 1);
	uint32_t whiteTextureData = 0xffffffff;
	s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

	Ref<ShaderLibrary>shaderLib = ShaderLibrary::Create();
	shaderLib->load("Triangle Shader", "assets/shaders/TriangleShaderVert.spv", "assets/shaders/TriangleShaderFrag.spv");
	//s_Data->TextureShader = shaderLib->get("Triangle Shader");
	s_Data->TextureShader->bind();
	s_Data->TextureShader->setInt("u_Texture", 0);
}

void Renderer2D::Shutdown()
{
	PSIM_PROFILE_FUNCTION();

	delete s_Data;
}

void Renderer2D::BeginScene(const OrthographicCamera& camera)
{
	PSIM_PROFILE_FUNCTION();

	s_Data->TextureShader->bind();
	s_Data->TextureShader->setMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
}

void Renderer2D::EndScene()
{
	PSIM_PROFILE_FUNCTION();

}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
{
	DrawQuad({ position.x, position.y, 0.0f }, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
{
	PSIM_PROFILE_FUNCTION();

	s_Data->TextureShader->setFloat4("u_Color", color);
	s_Data->WhiteTexture->Bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
	s_Data->TextureShader->setMat4("u_Transform", transform);
	s_Data->QuadVertexArray->Bind();
	RenderCommands::DrawIndexed(s_Data->QuadVertexArray);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
{
	DrawQuad({ position.x, position.y, 0.0f }, size, texture);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
{
	PSIM_PROFILE_FUNCTION();

	s_Data->TextureShader->setFloat4("u_Color", glm::vec4(1.0f));
	texture->Bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
	s_Data->TextureShader->setMat4("u_Transform", transform);

	s_Data->QuadVertexArray->Bind();
	RenderCommands::DrawIndexed(s_Data->QuadVertexArray);
}