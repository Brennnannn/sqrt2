#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class vkRender
{
public:
	const uint32_t screenX = 1920;
	const uint32_t screenY = 1080;
	GLFWwindow* window;
	VkSurfaceKHR surface;
	bool debugTest;

	VkCommandPool commandPool = VK_NULL_HANDLE;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;

	vkRender();
	~vkRender();

	void startRender();
	void endRender();
	 
private:

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice gpu = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties gpuProperties = {};

	uint32_t graphicsFamilyIndex = 0;

	std::vector<const char*> instanceLayer = { "VK_LAYER_KHRONOS_validation" };
	std::vector<const char*> instanceExtension;
	std::vector<const char*> deviceExtension;

	VkSurfaceFormatKHR surfaceFormat;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	uint32_t swapchainImageCount = 2;		//USE FOR DIFFERENT BUFFER SYSTEMS
	uint32_t activeSwapchainImageID = UINT32_MAX;

	VkFence swapchainImageAvailable = VK_NULL_HANDLE;

	VkImage depthStencilImage = VK_NULL_HANDLE;
	VkDeviceMemory depthStencilImageMemory = VK_NULL_HANDLE;
	VkImageView depthStencilImageView = VK_NULL_HANDLE;
	VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
	bool stencilAvailable = false;

	VkRenderPass renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> framebuffers;

	VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;

	void InitInstance();
	void deInitInstance();

	void InitDevice();
	void deInitDevice();

	void InitWindow();
	void deInitWindow();

	void InitSwapchain();
	void deInitSwapchain();

	void InitDepthStencilImage();
	void deInitDepthStencilImage();

	void InitDebug();
	void deInitDebug();

	void InitRenderPass();
	void deInitRenderPass();

	void InitFramebuffers();
	void deInitFramebuffers();

	void InitSynchronizations();
	void deInitSynchronizations();

	void InitResources();
	void deInitResources();

};