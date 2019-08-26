#include "vkLoop.h"
#include "vkRender.h"


/*vkLoop::vkLoop()
{
	vkRender r;

	auto device = r._device;
	auto queue = r._queue;

	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);

	VkSemaphore semaphore;
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);

	VkCommandPool commandPool;
	VkCommandPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.queueFamilyIndex = r._graphicsFamilyIndex;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool);

	VkCommandBuffer commandBuffer[2];
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffer);
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffer[0], &beginInfo);


		VkViewport viewPort{};
		viewPort.maxDepth = 1.0f;
		viewPort.minDepth = 0.0f;
		viewPort.height = 512;
		viewPort.width = 512;
		viewPort.x = 0;
		viewPort.y = 0;
		vkCmdSetViewport(commandBuffer[0], 0, 1, &viewPort);

		vkEndCommandBuffer(commandBuffer[0]);
	}

	/*{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffer[1], &beginInfo);


		VkViewport viewPort{};
		viewPort.maxDepth = 1.0f;
		viewPort.minDepth = 0.0f;
		viewPort.height = 512;
		viewPort.width = 512;
		viewPort.x = 0;
		viewPort.y = 0;
		vkCmdSetViewport(commandBuffer[1], 0, 1, &viewPort);

		vkEndCommandBuffer(commandBuffer[1]);
	}
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[0];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphore;
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}*/
	/*{
		VkPipelineStageFlags flags[]{ VK_PIPELINE_STAGE_ALL_COMMANDS_BIT };
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[1];
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &semaphore;
		submitInfo.pWaitDstStageMask = flags;
		vkQueueSubmit(queue, 1, &submitInfo, fence);
	}
	//auto ret = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkQueueWaitIdle(queue);


}


vkLoop::~vkLoop()
{
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyFence(device, fence, nullptr);
	vkDestroySemaphore(device, semaphore, nullptr);
}*/
