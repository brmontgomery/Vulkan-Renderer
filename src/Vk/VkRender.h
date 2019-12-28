#pragma once
//in the premake but included here for redundancy

//vendor files
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

//non-core std-lib
#include <array>
#include <optional>

//GLOBAL vars
const int WIDTH = 800;
const int HEIGHT = 600;

class VkRender
{
public:
	//public var for vulkan
	//--------------------------------------------------------------------------------------------------------------------------------
	bool framebufferResized = false;

	//--------------------------------------------------------------------------------------------------------------------------------


	//public structs for vulkan
	//--------------------------------------------------------------------------------------------------------------------------------
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription getBindingDescription() {
			vk::VertexInputBindingDescription bindingDescription = { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };

			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};

			attributeDescriptions[0] = { 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos) };
			attributeDescriptions[1] = { 1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color) };
			attributeDescriptions[2] = { 2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord) };
			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};

	//--------------------------------------------------------------------------------------------------------------------------------


	//public funcs for vulkan
	//--------------------------------------------------------------------------------------------------------------------------------
	VkRender();
	~VkRender();

	bool mainLoop();

	//--------------------------------------------------------------------------------------------------------------------------------

private:
	//private vars for vulkan
	//--------------------------------------------------------------------------------------------------------------------------------
	GLFWwindow* window;
	vk::Instance instance;
	vk::DebugUtilsMessengerEXT debugUtilsMessenger;

	std::vector<const char*> instanceLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	vk::SurfaceKHR surface;

	vk::PhysicalDevice physicalDevice = nullptr;

	vk::Device device;

	vk::Queue graphicsQueue;
	vk::Queue presentQueue;

	vk::SwapchainKHR swapChain;
	std::vector<vk::Image> swapChainImages;
	vk::Format swapChainImageFormat;
	vk::Extent2D swapChainExtent;

	//--------------------------------------------------------------------------------------------------------------------------------


	//private structs for vulkan
	//--------------------------------------------------------------------------------------------------------------------------------
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return (graphicsFamily.has_value() && presentFamily.has_value());
		}
	};

	struct SwapChainSupportDetails {
		SwapChainSupportDetails(vk::SurfaceCapabilitiesKHR surfaceCapabilites, std::vector<vk::SurfaceFormatKHR> surfaceFormats, std::vector<vk::PresentModeKHR> surfacePresentModes)
			: capabilities(surfaceCapabilites), formats(surfaceFormats), presentModes(surfacePresentModes) {}
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	//--------------------------------------------------------------------------------------------------------------------------------

	//private funcs for vulkan
	//--------------------------------------------------------------------------------------------------------------------------------
	void initVulkan();
	void initWindow();
	void cleanUp();

	void createInstance();
	std::vector<const char*> getInstanceExtensions();
	std::vector<const char*> getInstanceLayers();

	void setupDebugMessenger();
	vk::DebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo();

	void createSurface();

	void pickPhysicalDevice();
	bool isDeviceSuitable(vk::PhysicalDevice device);
	int rateDevice(vk::PhysicalDevice device);
	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

	void createLogicalDevice();

	VkRender::SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void recreateSwapChain();
	void cleanupSwapChain();
	//--------------------------------------------------------------------------------------------------------------------------------
};
