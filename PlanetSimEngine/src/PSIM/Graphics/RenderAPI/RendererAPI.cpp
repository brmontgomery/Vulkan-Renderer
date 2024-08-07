#include "PSIMPCH.h"
#include "RendererAPI.h"

#include "Platform/Vk/RenderAPI/VulkanRendererAPI.h"

RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

Scope<RendererAPI> RendererAPI::Create()
{
	switch (s_API)
	{
	case RendererAPI::API::None:    PSIM_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case RendererAPI::API::OpenGL:  PSIM_ASSERT(false, "RendererAPI::Vulkan is currently not supported!"); return nullptr;
	case RendererAPI::API::Vulkan:  return CreateScope<VulkanRendererAPI>();
	}

	PSIM_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}