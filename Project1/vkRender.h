#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class vkRender
{
public:
	vkRender();
	~vkRender();


private:
	void _InitInstance();
	void _deInitInstance();

	void _InitDevice();
	void _deInitDevice();

	void _setupDebug();
	void _initDebug();
	void _deInitDebug();

	VkInstance _instance = nullptr;
	VkDevice _device = nullptr;
	VkPhysicalDevice _gpu = nullptr;
	VkPhysicalDeviceProperties _gpuProperties = {};

	uint32_t _graphicsFamilyIndex = 0;

	std::vector<const char*> _instanceLayer;
	std::vector<const char*> _instanceExtension;

	VkDebugReportCallbackEXT _debugReport = nullptr;
};