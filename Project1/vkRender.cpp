#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <vector>
#include <array>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vkRender.h"

VkSurfaceKHR surface;
VkSwapchainKHR swapchain = VK_NULL_HANDLE;
bool debugTest;
std::vector<const char*> requiredExtensions;
PFN_vkCreateDebugReportCallbackEXT fetchedvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fetchedvkDestroyDebugReportCallbackEXT = nullptr;

vkRender::vkRender()
{

	#ifdef DEBUG															//Runs debug if in debug mode
	debugTest = true;
	#endif

	InitInstance();															//Runs instance
	if (debugTest) { InitDebug(); }											//Runs debug if I feel like it
	InitDevice();															//Runs device
	InitWindow();															//Starts window
	InitSwapchain();														//Creates swapchain
	InitDepthStencilImage();												//Creates image
	InitRenderPass();														//Creates a render pass
	InitFramebuffers();														//Creates a framebuffer
	InitSynchronizations();
	InitResources();
}	

vkRender::~vkRender()
{
	vkQueueWaitIdle(queue);
	deInitResources();
	deInitSynchronizations();
	deInitFramebuffers();													//Kills framebuffers
	deInitRenderPass();														//Kills render pass
	deInitDepthStencilImage();												//Kills image setup
	deInitSwapchain();														//Kills the swapchain
	deInitDevice();															//Kills device
	if (debugTest) { deInitDebug(); }										//Kills debug 
	deInitInstance();														//Kills instance
	deInitWindow();															//Kills the window
}	

	void vkRender::InitInstance()
	{
		if (debugTest) {
			instanceLayer.push_back("VK_LAYER_LUNARG_standard_validation");		//Gives the program knowledge of debug layers
			instanceExtension.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);	//Enables debug callback
		}
		instanceExtension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);				//Allows swapchain to work???

		VkApplicationInfo applicationInfo{};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;				//Tells struct what is being stored here
		applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 3);					//Version of Vulkan API (1.0.3 is a work around?)
		applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);			//This programs version			???
		applicationInfo.pApplicationName = "vkRender";							//Program name

		if (!glfwInit())														//Loads GLFW library for usage - needs to be run before any other GLFW methods
		{
			assert(0 && "GLFW error: library init failed");						//Kills program if GLFW cannot initialize
		}

		uint32_t count;
		const char** extensions = glfwGetRequiredInstanceExtensions(&count);	//Populates instance extension needs 

		VkInstanceCreateInfo instanceCreateInfo{};								//Info setup for instance
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;		//Modifies sType paramater of info structure
		instanceCreateInfo.pApplicationInfo = &applicationInfo;					//Takes info from app info (above) and uses it in info structure
		instanceCreateInfo.enabledExtensionCount = count;						//Tells how many extensions there are
		instanceCreateInfo.ppEnabledExtensionNames = extensions;				//Feeds the extensions into the info structure
		instanceCreateInfo.enabledLayerCount = instanceLayer.size();			//Tells how many layers there are
		instanceCreateInfo.ppEnabledLayerNames = instanceLayer.data();			//Feeds layers into the info strucutre

		auto err = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);	//Runs instance creation function and sets "err" to error code returned
		if (VK_SUCCESS != err)													//Quits if there is an error creating instance
		{
			assert(0 && "VK error: Instance creation failed");					//Assert 0 means false, causing a stop, then prints the error as defined
		}

	}

	void vkRender::deInitInstance()
	{
		vkDestroyInstance(instance, nullptr);									//Destroys _instance (the instance) using default memory manage
		instance = nullptr;														//Ensures instance is nothing
	}

	void vkRender::InitDevice()
	{
		{
			deviceExtension.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);			//Enables swapchain support - possibly to start gathering info from device

			uint32_t gpuCount = 0;												//Amount of graphic devices
			vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);			//Registers devices (start)
			std::vector<VkPhysicalDevice> gpuList(gpuCount);					//Stores the graphic devices
			vkEnumeratePhysicalDevices(instance, &gpuCount, gpuList.data());	//Dumps devices in (finish)
			gpu = gpuList[0];													//Sets graphic driver as first on list <-- REWRITE TO PICK BEST DEVICE 
			vkGetPhysicalDeviceProperties(gpu, &gpuProperties);					//Populates gpuProperties for use in the info structure
		}

		{
			uint32_t familyCount = 0;											//How many families the GPU has
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, nullptr);//Asks which families the GPU has
			std::vector<VkQueueFamilyProperties> familyPropertyList(familyCount);//Initializes array of families and their data
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &familyCount, familyPropertyList.data());//Tells us which families the GPU has

			bool found = false;
			for (uint32_t i = 0; i < familyCount; i++)							//Searches families of graphics device for graphics support family
			{
				if (familyPropertyList[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)	//Tests for graphics support
				{
					found = true;												//Saves which family supports this
					graphicsFamilyIndex = i;
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
			if (debugTest)
			{
				std::cout << "Instance layers:\n";									//Neatly prints out a list of all layers 
				for (auto &i : layerPropertyList)
				{
					std::cout << " " << i.layerName << ":\t\t " << i.description << "\n";
				}
				std::cout << "\n-------------------------------------------\n";
			}
		}

		{
			float queuePriorities[]{ 1.0f };									//Sets which que has priority - only one is used			???
			VkDeviceQueueCreateInfo deviceQueCreateInfo{};						//Info strucuture for the GPU device
			deviceQueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;//Sets type of info strucuture
			deviceQueCreateInfo.queueFamilyIndex = graphicsFamilyIndex;			//The que family being used is set
			deviceQueCreateInfo.queueCount = 1;									//Using 1 que of this family - hard coded
			deviceQueCreateInfo.pQueuePriorities = queuePriorities;				//Tells which que takes priority if multiple are running

			VkDeviceCreateInfo deviceCreateInfo{};								//New info structure for device
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;		//Sets type of info structure
			deviceCreateInfo.queueCreateInfoCount = 1;							//1 GPU used - hard coded
			deviceCreateInfo.pQueueCreateInfos = &deviceQueCreateInfo;			//Feeds information about device que to info structure 
			deviceCreateInfo.enabledExtensionCount = deviceExtension.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtension.data();


			auto err = vkCreateDevice(gpu, &deviceCreateInfo, nullptr, &device);//Creates device
			if (VK_SUCCESS != err)												//Ensures device created successfully
			{
				assert(0 && "VK error: Device creation falied");				//If device didn't create, stop program
			}

			vkGetDeviceQueue(device, graphicsFamilyIndex, 0, &queue);			//Feeds device queue into a variable - what is devicequeue???
		}
	}

	void vkRender::deInitDevice()
	{
		vkDestroyDevice(device, nullptr);										//Destroys the device
		device = VK_NULL_HANDLE;												//Clears the memory for the device
	}

	void vkRender::InitWindow()
	{
		uint32_t count;
		std::vector<std::string> extensions;

		auto res = glfwGetRequiredInstanceExtensions(&count);
		extensions.reserve(count);
		for (uint32_t i = 0; i < count; i++) {
			extensions.emplace_back(res[i]);
		}
		glfwGetPhysicalDevicePresentationSupport(instance, gpu, graphicsFamilyIndex);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(screenX, screenY, "ClosedWorld", NULL, NULL);	//Parameters of the window to be created
		if (!window)
		{
			assert(0 && "GLFW error: window not created");						//Kills program if window isn't set
		}
		VkResult surfaceTest = glfwCreateWindowSurface(instance, window, NULL, &surface);//Creates a surface that code can be written to
		if (surfaceTest != VK_SUCCESS)
		{
			assert(0 && "GLFW error: surface not created");
		}
	}

	void vkRender::deInitWindow()
	{
		glfwDestroyWindow(window);
	}

	void vkRender::InitSwapchain()
	{


		VkSurfaceFormatKHR surfaceFormat = {};									//Sets empty  parameters as there is not data on the surface
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;						//format / data type to pass color in - RESEARCH LATER
		surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;			//How to send color - RESEARCH LATER
		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};						//Also no info on surface capabilities

		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		uint32_t swapchainImageCount = 2;										//Determines how many swap images a PC can run
		if (swapchainImageCount < surfaceCapabilities.minImageCount + 1)
		{
			swapchainImageCount = surfaceCapabilities.minImageCount + 1;
		}
		if (surfaceCapabilities.maxImageCount > 0)
		{
			if (swapchainImageCount > surfaceCapabilities.maxImageCount)
			{
				swapchainImageCount = surfaceCapabilities.maxImageCount;
			}
		}

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;				//Sets default mode in case higher modes aren't supported
		uint32_t supportedPresentModes = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &supportedPresentModes, nullptr);//Determines amount of supported modes
		std::vector<VkPresentModeKHR> presentModeList(supportedPresentModes);	//Creates storage for different present modes
		vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &supportedPresentModes, presentModeList.data()); //Populates list with supported modes
		for (int i = 0; i < presentModeList.size(); i++)
		{
			if (presentModeList[i] == VK_PRESENT_MODE_MAILBOX_KHR)				//Sets type to Mailbox if it's supported
			{
				presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;//Structure type
		swapchainCreateInfo.surface = surface;									//Surface to render on
		swapchainCreateInfo.minImageCount = 2;									//Swap image count <- REWRITE THIS WHEN ADDING VARIABLE BUFFER SUPPORT
		swapchainCreateInfo.imageFormat = surfaceFormat.format;					//Not sure ???
		swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;			//Not sure ???
		swapchainCreateInfo.imageExtent.width = screenX;						//Resolution to send X at
		swapchainCreateInfo.imageExtent.height = screenY;						//Resolution to send Y at
		swapchainCreateInfo.imageArrayLayers = 1;								//Not sure ???
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	//Sets mode to rendering new content (Not reused)
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;		//Indicates whether this chain is the exclusive renderer
		swapchainCreateInfo.queueFamilyIndexCount = 0;							//Used for interaction between multiple chains, unused here ^
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;						//Used for same process as data above
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;//Used for something... ???
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	//Sets data type for transfering color
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;									//Allows partial rerenders as opposed to forcing each push to rewrite entirely
		swapchainCreateInfo.oldSwapchain = nullptr;								//Used when restarting swapchain (EX: resizing window) <-- REWRITE FOR DYNAMIC WINDOW

		VkResult issue = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
		if (issue != VK_SUCCESS) {
			std::cout << issue << std::endl;
			throw std::runtime_error("Failed to create logical device");
		}

		VkBool32 surfaceSupported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, graphicsFamilyIndex, surface, &surfaceSupported);
		if (surfaceSupported)
		{
			vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);//Creates swap chain
			vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
		}

		{
			swapchainImages.resize(swapchainImageCount);
			swapchainImageViews.resize(swapchainImageCount);
			vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
			for (uint32_t i = 0; i < swapchainImageCount; i++)
			{
				VkImageViewCreateInfo imageViewCreateInfo{};
				imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				imageViewCreateInfo.image = swapchainImages[i];
				imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				imageViewCreateInfo.format = surfaceFormat.format;
				imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
				imageViewCreateInfo.subresourceRange.levelCount = 1;
				imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
				imageViewCreateInfo.subresourceRange.layerCount = 2;

				vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
			}
		}
	}

	void vkRender::deInitSwapchain()
	{
		for (auto view : swapchainImageViews)
		{
			vkDestroyImageView(device, view, nullptr);
		}

		vkDestroySwapchainKHR(device, VK_NULL_HANDLE, nullptr);
	}

	void vkRender::InitDepthStencilImage()
	{
		std::vector<VkFormat> tryFormats
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM
		};
		for (auto format : tryFormats)
		{
			VkFormatProperties formatProperties{};
			vkGetPhysicalDeviceFormatProperties(gpu, format, &formatProperties);
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				depthStencilFormat = format;
				break;
			}
		}
		if (depthStencilFormat == VK_FORMAT_UNDEFINED)
		{
			assert(0 && "Depth stencils unavailable");
		}

		{
			stencilAvailable = (depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
				depthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
				depthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
				depthStencilFormat == VK_FORMAT_D32_SFLOAT ||
				depthStencilFormat == VK_FORMAT_D16_UNORM);

			VkImageCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.flags = 0;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = depthStencilFormat;
			imageCreateInfo.extent.width = screenX;
			imageCreateInfo.extent.height = screenY;
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.pQueueFamilyIndices = nullptr;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			vkCreateImage(device, &imageCreateInfo, nullptr, &depthStencilImage);
		}

		{
			VkMemoryRequirements imageMemoryRequirements{};
			vkGetImageMemoryRequirements(device, depthStencilImage, &imageMemoryRequirements);

			uint32_t memoryIndex = UINT32_MAX;
			VkPhysicalDeviceMemoryProperties gpuMemoryProperties;
			vkGetPhysicalDeviceMemoryProperties(gpu, &gpuMemoryProperties);				//Used often - implement to shared method eventually - use last 5 min of vid 9
			auto requiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			for (uint32_t i = 0; i < gpuMemoryProperties.memoryTypeCount; ++i)
			{
				if ((imageMemoryRequirements.memoryTypeBits & (1 << i)) &&
					(gpuMemoryProperties.memoryTypes[i].propertyFlags & requiredProperties))
				{
					memoryIndex = i;
					break;
				}
			}
			if (memoryIndex == UINT32_MAX)
			{
				assert(0 && "Memory type unrecognized");
			}

			VkMemoryAllocateInfo memoryCreateInfo{};
			memoryCreateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryCreateInfo.allocationSize = imageMemoryRequirements.size;
			memoryCreateInfo.memoryTypeIndex = memoryIndex;

			vkAllocateMemory(device, &memoryCreateInfo, nullptr, &depthStencilImageMemory);
			vkBindImageMemory(device, depthStencilImage, depthStencilImageMemory, 0);
		}
		{
			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image = depthStencilImage;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = depthStencilFormat;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT
				| (stencilAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 0;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(device, &imageViewCreateInfo, nullptr, &depthStencilImageView);
		}
	}

	void vkRender::deInitDepthStencilImage()
	{
		vkDestroyImageView(device, depthStencilImageView, nullptr);
		vkFreeMemory(device, depthStencilImageMemory, nullptr);
		vkDestroyImage(device, depthStencilImage, nullptr);
	}

	void vkRender::InitRenderPass()
	{
		std::array<VkAttachmentDescription, 2> attachments{};	//Important render data
		attachments[0].flags = 0;							//Revisit renderpass tutorial when getting fancy
		attachments[0].format = depthStencilFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//Determines if image is kept or cleared
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;					//Crucial stuff to render here
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		attachments[1].flags = 0;
		attachments[1].format = surfaceFormat.format;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		std::array<VkAttachmentReference, 1> subpass1ColorAttachment{};
		subpass1ColorAttachment[0].attachment = 1;
		subpass1ColorAttachment[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference subpass1DepthStencilAttachment{};
		subpass1DepthStencilAttachment.attachment = 0;
		subpass1DepthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::array<VkSubpassDescription, 1> subpasses{};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = subpass1ColorAttachment.size();
		subpasses[0].pColorAttachments = subpass1ColorAttachment.data();
		//subpasses[0].pResolveAttachments = ; //Multisampling
		subpasses[0].pDepthStencilAttachment = &subpass1DepthStencilAttachment;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = attachments.size();
		renderPassCreateInfo.pAttachments = attachments.data();
		renderPassCreateInfo.subpassCount = subpasses.size();
		renderPassCreateInfo.pSubpasses = subpasses.data();

		vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
	}

	void vkRender::deInitRenderPass()
	{
		vkDestroyRenderPass(device, renderPass, nullptr);
	}

	void vkRender::InitFramebuffers()
	{
		framebuffers.resize(swapchainImageCount);
		for (uint32_t i = 0; i < swapchainImageCount; ++i)
		{
			std::array<VkImageView, 2> attachments{};
			attachments[0] = depthStencilImageView;
			attachments[1] = swapchainImageViews[i];

			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass = renderPass;
			framebufferCreateInfo.attachmentCount = attachments.size();
			framebufferCreateInfo.pAttachments = attachments.data();
			framebufferCreateInfo.width = screenX;
			framebufferCreateInfo.height = screenY;
			framebufferCreateInfo.layers = 1;

			vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]);
		}
	}

	void vkRender::deInitFramebuffers()
	{
		for (auto i : framebuffers)
		{
			vkDestroyFramebuffer(device, i, nullptr);
		}
	}

	void vkRender::InitSynchronizations()
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		vkCreateFence(device, &fenceCreateInfo, nullptr, &swapchainImageAvailable);
	}

	void vkRender::deInitSynchronizations()
	{
		vkDestroyFence(device, swapchainImageAvailable, nullptr);
	}

	void vkRender::InitResources()
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT  
									| VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = graphicsFamilyIndex;

		vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
	
		VkCommandBufferAllocateInfo commandBufferAllocationInfo{};
		commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocationInfo.commandPool = commandPool;
		commandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocationInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(device, &commandBufferAllocationInfo, &commandBuffer);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
	}

	void vkRender::deInitResources()
	{
		vkDestroySemaphore(device, renderCompleteSemaphore, nullptr);

		vkDestroyCommandPool(device, commandPool, nullptr);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugReportFlagsEXT messageFlags,										//Type of callback such as warning or error
		VkDebugReportObjectTypeEXT objectType,									//Type of object that caused the error
		uint64_t sourceObject,													//Location of object that caused error
		size_t location,														// ???
		int32_t messageCode,													//How important the callback is ???
		const char* layerPrefix,												//Which layer fired the callback
		const char* message,													//Human readable string of error
		void * userData															// ???
	) {
		std::cout << message << std::endl;
		return false;															//Allows more similar operation to release - maybe continues on callback?
	}

	void vkRender::InitDebug()
	{
		fetchedvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		fetchedvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (nullptr == fetchedvkCreateDebugReportCallbackEXT || nullptr == fetchedvkDestroyDebugReportCallbackEXT)
		{
			assert(0 && "VK error: Debug creation failed!");
		}

		VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo{};
		debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
		debugCallbackCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT
			| VK_DEBUG_REPORT_WARNING_BIT_EXT	
			| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
			| VK_DEBUG_REPORT_ERROR_BIT_EXT
			| VK_DEBUG_REPORT_DEBUG_BIT_EXT | 0;

		fetchedvkCreateDebugReportCallbackEXT(instance, &debugCallbackCreateInfo, nullptr, &debugReport);
		//vkCreateDebugReportCallbackEXT(instance, nullptr, nullptr, nullptr);
	}

	void vkRender::deInitDebug()
	{
		fetchedvkDestroyDebugReportCallbackEXT(instance, debugReport, nullptr);	//Destroys callback report
		debugReport = nullptr;													//Cleans up potential garbage
	}

//------------------------------------------------

	void vkRender::startRender()
	{
		vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, VK_NULL_HANDLE, swapchainImageAvailable, &activeSwapchainImageID);
		vkWaitForFences(device, 1, &swapchainImageAvailable, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &swapchainImageAvailable);
		vkQueueWaitIdle(queue);

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		VkRect2D renderAreaCreateInfo{};
		renderAreaCreateInfo.offset.x = 0;
		renderAreaCreateInfo.offset.y = 0;
		renderAreaCreateInfo.extent = { screenX, screenY };

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].depthStencil.depth = 0.0f;
		clearValues[0].depthStencil.stencil = 0;
		clearValues[1].color.float32[0] = 1.0f;	//Chain to surface based on type 29:30 v12
		clearValues[1].color.float32[1] = 0.45f;
		clearValues[1].color.float32[2] = 0.65f;
		clearValues[1].color.float32[3] = 0.3f;

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffers[activeSwapchainImageID];
		renderPassBeginInfo.renderArea = renderAreaCreateInfo;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphore;
			
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}

	void vkRender::endRender()
	{
		vkCmdEndRenderPass(commandBuffer);

		vkEndCommandBuffer(commandBuffer);

		VkResult results = VkResult::VK_RESULT_MAX_ENUM;

		VkPresentInfoKHR presentCreateInfo{};
		presentCreateInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentCreateInfo.waitSemaphoreCount = 1;
		presentCreateInfo.pWaitSemaphores = &renderCompleteSemaphore;
		presentCreateInfo.swapchainCount = 1;
		presentCreateInfo.pSwapchains = &swapchain;
		presentCreateInfo.pImageIndices = &activeSwapchainImageID;
		presentCreateInfo.pResults = &results;


		vkQueuePresentKHR(queue, &presentCreateInfo);
	}