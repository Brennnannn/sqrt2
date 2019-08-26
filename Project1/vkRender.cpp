#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vkRender.h"



vkRender::vkRender()
{
	_InitInstance();														//Runs instance															
	_InitDevice();															//Runs device
	_InitWindow();															//Starts window
}


vkRender::~vkRender()
{
	_deInitDevice();														//Kills device
	_deInitInstance();														//Kills instance
	_deInitWindow();
}	

void vkRender::_InitInstance()												//Function to initialize instance
{
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;				//			???
	applicationInfo.apiVersion = VK_MAKE_VERSION(1,0,3);					//Version of Vulkan API (1.0.3 is a work around?)
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);			//This programs version			???
	applicationInfo.pApplicationName = "vkRender";							//Program name
	
	VkInstanceCreateInfo instanceCreateInfo{};								//Info setup for instance
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;		//Modifies sType paramater of info structure
	instanceCreateInfo.pApplicationInfo = &applicationInfo;					//Takes info from app info (above) and uses it in info structure
	instanceCreateInfo.enabledLayerCount = _instanceLayer.size();			//Feeds amount of layers to instance
	instanceCreateInfo.ppEnabledLayerNames = _instanceLayer.data();			//Loads these layers to instance

	auto err = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);	//Runs instance creation function and sets "err" to error code returned
	if (VK_SUCCESS != err)													//Quits if there is an error creating instance
	{
		assert(0 && "VK error: Instance creation failed");					//Assert 0 means false, causing a stop, then prints the error as defined
	}
}

void vkRender::_deInitInstance()											//Destroys VK instance
{
	vkDestroyInstance(_instance, nullptr);									//Destroys _instance (the instance) using default mem manage
	_instance = nullptr;													//Ensures instance is nothing
}

void vkRender::_InitDevice()												//Creates a device
{
	{
		uint32_t gpuCount = 0;												//Amount of graphic devices
		vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);			//Registers devices (start)
		std::vector<VkPhysicalDevice> gpuList(gpuCount);					//Stores the graphic devices
		vkEnumeratePhysicalDevices(_instance, &gpuCount, gpuList.data());	//Dumps devices in (finish)
		_gpu = gpuList[0];													//Sets graphic driver as first on list		REVISIT 
		vkGetPhysicalDeviceProperties(_gpu, &_gpuProperties);
	}
	
	{
		uint32_t familyCount = 0;											//How many families the GPU has
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, nullptr);//Asks which families the GPU has
		std::vector<VkQueueFamilyProperties> familyPropertyList(familyCount);//Initializes array of families and their data
		vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &familyCount, familyPropertyList.data());//Tells us which families the GPU has
		
		bool found = false;
		for (uint32_t i = 0; i < familyCount; i++)							//Searches families of graphics device for graphics support family
		{
			if (familyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)	//Tests for graphics support
			{
				found = true;												//Saves which family supports this
				_graphicsFamilyIndex = i;
			}
		}
		if (!found)															//Quits program if no graphic support is found
		{ 
			assert(0 && "VK error: Graphics not found to have queue familysupport"); 
		}
	}
	
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);			//tells system to que up layers
		std::vector<VkLayerProperties> layerPropertyList(layerCount);		// Create space to store these layers
		vkEnumerateInstanceLayerProperties(&layerCount, layerPropertyList.data());//Loads layers to vector
		std::cout << "Instance layers:\n";									//Neatly prints out a list of all layers 
		for (auto &i : layerPropertyList)
		{
			std::cout << " " << i.layerName << ":\t\t " << i.description << "\n";
		}
		std::cout << "\n-------------------------------------------\n";
	}

	{
		uint32_t layerCount = 0;
		vkEnumerateDeviceLayerProperties(_gpu, &layerCount, nullptr);		//tells system to que up devices
		std::vector<VkLayerProperties> layerPropertyList(layerCount);		// Create space to store these devices
		vkEnumerateDeviceLayerProperties(_gpu, &layerCount, layerPropertyList.data());//Loads layers to vector
		std::cout << "Instance devices:\n";									//Neatly prints out a list of all devices 
		for (auto &i : layerPropertyList)
		{
			std::cout << " " << i.layerName << ":\t\t " << i.description << "\n";
		}
		std::cout << "\n-------------------------------------------\n";
	}

	float queuePriorities[]{ 1.0f };										//Sets which que has priority - only one is used			???
	VkDeviceQueueCreateInfo deviceQueCreateInfo{};							//Info strucuture for the GPU device
	deviceQueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;	//Sets type of info strucuture
	deviceQueCreateInfo.queueFamilyIndex = _graphicsFamilyIndex;			//The que family being used is set
	deviceQueCreateInfo.queueCount = 1;										//Using 1 que of this family - hard coded
	deviceQueCreateInfo.pQueuePriorities = queuePriorities;					//Tells which que takes priority if multiple are running
	
	VkDeviceCreateInfo deviceCreateInfo{};									//New info structure for device
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;			//Sets type of info structure
	deviceCreateInfo.queueCreateInfoCount = 1;								//1 GPU used - hard coded
	deviceCreateInfo.pQueueCreateInfos = &deviceQueCreateInfo;				//Feeds information about device que to info structure 
	
	auto err = vkCreateDevice(_gpu, &deviceCreateInfo, nullptr, &_device);	//Creates device
	if (VK_SUCCESS != err)													//Ensures device created successfully
	{
		assert(0 && "VK error: Device creation falied");					//If device didn't create, stop program
	}

	vkGetDeviceQueue(_device, _graphicsFamilyIndex, 0, &_queue);
}

void vkRender::_deInitDevice()
{
	vkDestroyDevice(_device, nullptr);										//Destroys the device
	_device = VK_NULL_HANDLE;												//Clears the memory for the device
}

void vkRender::_InitWindow()
{
	if (!glfwInit())														//Loads GLFW library for usage
	{
		assert(0 && "GLFW error: library init failed");						//Kills program if GLFW cannot initialize
	}
	GLFWwindow* window = glfwCreateWindow(640, 480, "ClosedWorld", NULL, NULL);//Parameters of the window to be created
	if(!window)
	{
		assert(0 && "GLFW error: window not created");						//Kills program if window isn't set
	}
	VkSurfaceKHR glfwSurface;
	VkResult surface = glfwCreateWindowSurface(_instance, window, NULL, &glfwSurface);
	if (!surface)
	{
		assert(0 && "GLFW error: surface not created");
	}

}

void vkRender::_deInitWindow()
{
	//glfwDestroyWindow(window);
}


