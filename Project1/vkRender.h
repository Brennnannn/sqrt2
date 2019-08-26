#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class vkRender
{
public:
	vkRender();
	~vkRender();


//private:
	void _InitInstance();
	void _deInitInstance();

	void _InitDevice();
	void _deInitDevice();

	void _InitWindow();
	void _deInitWindow();

	VkInstance _instance = VK_NULL_HANDLE;
	VkPhysicalDevice _gpu = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	VkQueue _queue = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties _gpuProperties = {};

	uint32_t _graphicsFamilyIndex = 0;

	std::vector<const char*> _instanceLayer;
	std::vector<const char*> _instanceExtension;

	VkDebugReportCallbackEXT _debugReport = VK_NULL_HANDLE;
};