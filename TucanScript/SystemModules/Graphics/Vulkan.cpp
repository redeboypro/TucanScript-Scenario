#include "Vulkan.h"

using namespace TucanScript;

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

Undef VkMakeApp (
	VkApplication** hAppPtr, 
	RCStr sAppTitle, 
	const Width_t iW, const Height_t iH) {
	VkApplication* hApp = new VkApplication ();
	if (!hApp) {
		LogErr ("Failed to create application!");
		return;
	}
	*hAppPtr = hApp;

	glfwInit ();

	glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);

	hApp->m_hWindow = glfwCreateWindow (iW, iH, sAppTitle, nullptr, nullptr);
	if (!hApp->m_hWindow) {
		glfwTerminate ();
		LogErr ("Failed to create window!");
		return;
	}

	DWord nExtensions = 0u;
	RCStr* aExtensions = glfwGetRequiredInstanceExtensions (&nExtensions);

	Vector<RCStr> extensionWrapper (aExtensions, aExtensions + nExtensions);

	VkApplicationInfo appInfo {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = sAppTitle,
		.applicationVersion = VK_MAKE_VERSION (1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION (1, 0, 0),
		.apiVersion = VK_API_VERSION_1_4
	};

	const Vector<RCStr> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkInstanceCreateInfo createInfo {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<DWord>(validationLayers.size ()),
		.ppEnabledLayerNames = validationLayers.data (),
		.enabledExtensionCount = static_cast<DWord>(extensionWrapper.size ()),
		.ppEnabledExtensionNames = extensionWrapper.data ()
	};

	vkCreateInstance (&createInfo, nullptr, &hApp->m_hInstance);
	glfwCreateWindowSurface (hApp->m_hInstance, hApp->m_hWindow, nullptr, &hApp->m_SurfaceView.m_hSurface);

	DWord nDevices = 0u;
	vkEnumeratePhysicalDevices (hApp->m_hInstance, &nDevices, nullptr);
	Vector<VkPhysicalDevice> enumPhysicalDevices (nDevices);
	vkEnumeratePhysicalDevices (hApp->m_hInstance, &nDevices, enumPhysicalDevices.data ());

	hApp->m_DeviceView.hPhysicalDevice = enumPhysicalDevices.front ();

	VkPhysicalDeviceProperties deviceProps {};
	vkGetPhysicalDeviceProperties (hApp->m_DeviceView.hPhysicalDevice, &deviceProps);

	VkSampleCountFlags counts = deviceProps.limits.framebufferColorSampleCounts &
		deviceProps.limits.framebufferDepthSampleCounts;

	hApp->m_uSampleCount = VK_SAMPLE_COUNT_1_BIT;

	if (counts & VK_SAMPLE_COUNT_64_BIT) hApp->m_uSampleCount = VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT) hApp->m_uSampleCount = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT) hApp->m_uSampleCount = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT)  hApp->m_uSampleCount = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT)  hApp->m_uSampleCount = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT)  hApp->m_uSampleCount = VK_SAMPLE_COUNT_2_BIT;

	DWord nQueueFamilies = 0u;
	vkGetPhysicalDeviceQueueFamilyProperties (hApp->m_DeviceView.hPhysicalDevice, &nQueueFamilies, nullptr);

	Vector<VkQueueFamilyProperties> enumQueueFamilies (nQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties (hApp->m_DeviceView.hPhysicalDevice, &nQueueFamilies, enumQueueFamilies.data ());

	hApp->m_GraphicsQueueView.m_uQueueFamilyIndex = 0u;
	hApp->m_PresentQueueView.m_uQueueFamilyIndex = 0u;

	for (DWord iQueueFamily = 0u; iQueueFamily < nQueueFamilies; ++iQueueFamily) {
		VkBool32 uPresentSupported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR (hApp->m_DeviceView.hPhysicalDevice, iQueueFamily, hApp->m_SurfaceView.m_hSurface, &uPresentSupported);

		if (uPresentSupported) {
			hApp->m_PresentQueueView.m_uQueueFamilyIndex = iQueueFamily;
		}

		if (enumQueueFamilies[iQueueFamily].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			hApp->m_GraphicsQueueView.m_uQueueFamilyIndex = iQueueFamily;
			break;
		}
	}

	VkPhysicalDeviceFeatures deviceFeatures { .samplerAnisotropy = VK_TRUE };
	Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	Set<DWord> enumUniqueQueueFamilies = {
		hApp->m_GraphicsQueueView.m_uQueueFamilyIndex,
		hApp->m_PresentQueueView.m_uQueueFamilyIndex,
	};
	
	Dec32 fPriority = 1.0f;
	for (const DWord queueFamily : enumUniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queueFamily,
			.queueCount = 1,
			.pQueuePriorities = &fPriority
		};
		queueCreateInfos.emplace_back (queueCreateInfo);
	}

	const Vector<RCStr> enumDeviceExtensions {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size ()),
		.pQueueCreateInfos = queueCreateInfos.data (),
		.enabledExtensionCount = (DWord) enumDeviceExtensions.size (),
		.ppEnabledExtensionNames = enumDeviceExtensions.data (),
		.pEnabledFeatures = &deviceFeatures
	};

	vkCreateDevice (hApp->m_DeviceView.hPhysicalDevice, &deviceCreateInfo, nullptr, &hApp->m_DeviceView.hLogicalDevice);

	vkGetDeviceQueue (hApp->m_DeviceView.hLogicalDevice, 
					  hApp->m_GraphicsQueueView.m_uQueueFamilyIndex, 
					  0u, 
					  &hApp->m_GraphicsQueueView.m_hQueue);

	vkGetDeviceQueue (hApp->m_DeviceView.hLogicalDevice, 
					  hApp->m_PresentQueueView.m_uQueueFamilyIndex, 
					  0u, 
					  &hApp->m_PresentQueueView.m_hQueue);

	DWord presentModeCount = 0u;
	vkGetPhysicalDeviceSurfacePresentModesKHR (hApp->m_DeviceView.hPhysicalDevice, hApp->m_SurfaceView.m_hSurface, &presentModeCount, nullptr);
	Vector<VkPresentModeKHR> presentModes (presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR (hApp->m_DeviceView.hPhysicalDevice, hApp->m_SurfaceView.m_hSurface, &presentModeCount, presentModes.data ());

	hApp->m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& availablePresentMode : presentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			hApp->m_PresentMode = availablePresentMode;
			break;
		}
	}

	DWord nSurfaceFormats = 0u;
	vkGetPhysicalDeviceSurfaceFormatsKHR (hApp->m_DeviceView.hPhysicalDevice,
										  hApp->m_SurfaceView.m_hSurface, &nSurfaceFormats,
										  nullptr);

	Vector<VkSurfaceFormatKHR> enumSurfaceFormats (nSurfaceFormats);
	vkGetPhysicalDeviceSurfaceFormatsKHR (hApp->m_DeviceView.hPhysicalDevice,
										  hApp->m_SurfaceView.m_hSurface, &nSurfaceFormats, 
										  enumSurfaceFormats.data ());

	hApp->m_SurfaceView.m_SurfaceFormat = enumSurfaceFormats.front ();
	for (VkSurfaceFormatKHR& availableFormat : enumSurfaceFormats) {
		if (availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB) {
			hApp->m_SurfaceView.m_SurfaceFormat = availableFormat;
			break;
		}
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR (hApp->m_DeviceView.hPhysicalDevice, hApp->m_SurfaceView.m_hSurface, &hApp->m_SurfaceView.m_SurfaceCapabilities);
	hApp->m_Extent = hApp->m_SurfaceView.m_SurfaceCapabilities.currentExtent;
	hApp->m_nImages = NextWord (hApp->m_SurfaceView.m_SurfaceCapabilities.minImageCount);
}

Undef VkCreateSwapchain (VkApplication* hApp) {
	VkSwapchainCreateInfoKHR swapchainCreateInfo {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0u,
		.surface = hApp->m_SurfaceView.m_hSurface,
		.minImageCount = hApp->m_nImages,
		.imageFormat = hApp->m_SurfaceView.m_SurfaceFormat.format,
		.imageColorSpace = hApp->m_SurfaceView.m_SurfaceFormat.colorSpace,
		.imageExtent = hApp->m_Extent,
		.imageArrayLayers = 1u,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0u,
		.pQueueFamilyIndices = nullptr,
		.preTransform = hApp->m_SurfaceView.m_SurfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = hApp->m_PresentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};

	DWord aQueueFamilyIndices[] {
		hApp->m_GraphicsQueueView.m_uQueueFamilyIndex,
		hApp->m_PresentQueueView.m_uQueueFamilyIndex
	};

	if (aQueueFamilyIndices[0u] != aQueueFamilyIndices[1u]) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2u;
		swapchainCreateInfo.pQueueFamilyIndices = aQueueFamilyIndices;
	}
	else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	vkCreateSwapchainKHR (hApp->m_DeviceView.hLogicalDevice, &swapchainCreateInfo, nullptr, &hApp->m_SwapchainView.m_hSwapchain);
}

Undef VkGetSwapchainImages (VkApplication* hApp) {
	DWord nSwapchainImages = 0u;
	vkGetSwapchainImagesKHR (hApp->m_DeviceView.hLogicalDevice, 
							 hApp->m_SwapchainView.m_hSwapchain, 
							 &nSwapchainImages, nullptr);

	hApp->m_SwapchainView.m_EnumSwapchainImages = Vector<VkImage>(nSwapchainImages);
	
	vkGetSwapchainImagesKHR (hApp->m_DeviceView.hLogicalDevice, 
							 hApp->m_SwapchainView.m_hSwapchain, 
							 &nSwapchainImages, 
							 hApp->m_SwapchainView.m_EnumSwapchainImages.data ());

	hApp->m_SwapchainView.m_EnumSwapchainImageViews = Vector<VkImageView> (nSwapchainImages);

	for (Size iImage = 0u; iImage < nSwapchainImages; iImage++) {
		VkImageViewCreateInfo imageViewCreateInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = hApp->m_SwapchainView.m_EnumSwapchainImages[iImage],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = hApp->m_SurfaceView.m_SurfaceFormat.format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0u,
				.levelCount = 1u,
				.baseArrayLayer = 0u,
				.layerCount = 1u
			}
		};
		vkCreateImageView (hApp->m_DeviceView.hLogicalDevice, 
						   &imageViewCreateInfo, nullptr, 
						   &hApp->m_SwapchainView.m_EnumSwapchainImageViews[iImage]);
	}
}

DWord VkFindMemoryType (VkApplication* hApp, DWord uTypeFilter, VkMemoryPropertyFlags uMemPropFlags) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties (hApp->m_DeviceView.hPhysicalDevice, &memProperties);
	for (DWord uType = 0u; uType < memProperties.memoryTypeCount; uType++) {
		if ((uTypeFilter & (1 << uType)) && (memProperties.memoryTypes[uType].propertyFlags & uMemPropFlags) == uMemPropFlags) {
			return uType;
		}
	}
	LogErr ("Failed to find suitable memory type!");
}

Undef VkCreateRenderPass (VkApplication* hApp) {
	VkAttachmentDescription colorAttachmentDescription {
		.format = hApp->m_SurfaceView.m_SurfaceFormat.format,
		.samples = hApp->m_uSampleCount,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkAttachmentDescription depthAttachmentDescription {
		.format = VkGetDepthFormat (hApp),
		.samples = hApp->m_uSampleCount,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkAttachmentDescription colorAttachmentResolveDescription {
		.format = hApp->m_SurfaceView.m_SurfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference colorAttachmentRef {
		.attachment = 0u,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference depthAttachmentRef {
		.attachment = 1u,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference colorAttachmentResolveRef {
		.attachment = 2u,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpassDescription {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1u,
		.pColorAttachments = &colorAttachmentRef,
		.pResolveAttachments = &colorAttachmentResolveRef,
		.pDepthStencilAttachment = &depthAttachmentRef
	};

	VkSubpassDependency dependency {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0u,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0u,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};

	constexpr DWord NUM_ATTACHMENTS = 3u;

	VkAttachmentDescription aAttachments[NUM_ATTACHMENTS] = {
		colorAttachmentDescription,
		depthAttachmentDescription,
		colorAttachmentResolveDescription
	};

	VkRenderPassCreateInfo renderPassCreateInfo {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = NUM_ATTACHMENTS,
		.pAttachments = aAttachments,
		.subpassCount = 1u,
		.pSubpasses = &subpassDescription,
		.dependencyCount = 1u,
		.pDependencies = &dependency
	};

	vkCreateRenderPass (hApp->m_DeviceView.hLogicalDevice, 
						&renderPassCreateInfo, 
						nullptr, 
						&hApp->m_hRenderPass);
}

Undef VkCreateShaderModules (VkApplication* hApp, ShaderCode pVertexShaderSrc, Size uVertexShaderSize, ShaderCode pFragmentShaderSrc, Size uFragmentShaderSize,
							 RCStr sEntryPt, DWord uPipelineIndex) {
	VkShaderModuleCreateInfo vertexShaderCreateInfo {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = uVertexShaderSize,
		.pCode = pVertexShaderSrc
	};
	vkCreateShaderModule (hApp->m_DeviceView.hLogicalDevice,
						  &vertexShaderCreateInfo, nullptr, 
						  &hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_hVertexShaderModule);

	VkShaderModuleCreateInfo fragmentShaderCreateInfo {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = uFragmentShaderSize,
		.pCode = pFragmentShaderSrc
	};
	vkCreateShaderModule (hApp->m_DeviceView.hLogicalDevice,
						  &fragmentShaderCreateInfo, nullptr, 
						  &hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_hFragmentShaderModule);

	hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_aShaderStages[0u] = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_hVertexShaderModule,
		.pName = sEntryPt
	};

	hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_aShaderStages[1u] = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_hFragmentShaderModule,
		.pName = sEntryPt
	};
}

Undef VkCreatePipeline (VkApplication* hApp, DWord uPipelineIndex,
						const VkPipelineDepthStencilStateCreateInfo& depthStencilCreateInfo) {
	VkVertexInputBindingDescription bindingDescription {
		.binding = 0u,
		.stride = sizeof (VkVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

	VkVertexInputAttributeDescription aAttributeDescriptions[2u] = {
		{
			.location = 0u,
			.binding = 0u,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof (VkVertex, m_Position)
		},
		{
			.location = 1u,
			.binding = 0u,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof (VkVertex, m_UV)
		}
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1u,
		.pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = 2u,
		.pVertexAttributeDescriptions = aAttributeDescriptions
	};

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyCreateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1u,
		.scissorCount = 1u
	};

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
						  VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = 1u,
		.pAttachments = &colorBlendAttachment,
	};

	VkPipelineMultisampleStateCreateInfo multisampling {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = hApp->m_uSampleCount,
		.sampleShadingEnable = VK_FALSE,
	};

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2u,
		.pDynamicStates = dynamicStates,
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1u,
		.pSetLayouts = &hApp->m_PipelineViews[uPipelineIndex].m_DescriptorSetLayout
	};

	auto hPipelineLayout = &hApp->m_PipelineViews[uPipelineIndex].m_hPipelineLayout;
	vkCreatePipelineLayout (hApp->m_DeviceView.hLogicalDevice, &pipelineLayoutInfo, nullptr, hPipelineLayout);

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2u,
		.pStages = hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_aShaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &pipelineInputAssemblyCreateInfo,
		.pViewportState = &pipelineViewportStateCreateInfo,
		.pRasterizationState = &pipelineRasterizationStateCreateInfo,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencilCreateInfo,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &pipelineDynamicStateCreateInfo,
		.layout = *hPipelineLayout,
		.renderPass = hApp->m_hRenderPass,
		.subpass = 0u,
		.basePipelineHandle = VK_NULL_HANDLE,
	};

	if (vkCreateGraphicsPipelines (
		hApp->m_DeviceView.hLogicalDevice, 
		VK_NULL_HANDLE,
		1u, 
		&graphicsPipelineCreateInfo, 
		nullptr, 
		&hApp->m_PipelineViews[uPipelineIndex].m_hPipeline) != VK_SUCCESS) {
		LogErr ("Failed to create pipeline!");
	}

	vkDestroyShaderModule (hApp->m_DeviceView.hLogicalDevice, 
						   hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_hVertexShaderModule,
						   nullptr);
	vkDestroyShaderModule (hApp->m_DeviceView.hLogicalDevice, 
						   hApp->m_PipelineViews[uPipelineIndex].m_ShaderView.m_hFragmentShaderModule,
						   nullptr);
}

Undef VkCreateDescriptorPool (VkApplication* hApp, DWord uPipelineIndex, VkDescriptorPoolSize* aPoolSizes, DWord nPoolSizes) {
	VkDescriptorPoolCreateInfo poolInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
		.poolSizeCount = nPoolSizes,
		.pPoolSizes = aPoolSizes
	};

	if (vkCreateDescriptorPool (hApp->m_DeviceView.hLogicalDevice, &poolInfo, nullptr, 
		&hApp->m_PipelineViews[uPipelineIndex].m_DescriptorPool) != VK_SUCCESS) {
		LogErr ("Failed to create descriptor pool!");
	}
}

Undef VkCreateDescriptorSetLayout (VkApplication* hApp, DWord uPipelineIndex, VkDescriptorSetLayoutBinding* aBindings, DWord nBindings) {
	VkDescriptorSetLayoutCreateInfo layoutInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = nBindings,
		.pBindings = aBindings
	};

	if (vkCreateDescriptorSetLayout (hApp->m_DeviceView.hLogicalDevice, &layoutInfo, nullptr, 
		&hApp->m_PipelineViews[uPipelineIndex].m_DescriptorSetLayout) != VK_SUCCESS) {
		LogErr ("Failed to create descriptor set layout!");
	}
}

Undef VkCreateDescriptorSets (VkApplication* hApp, DWord uPipelineIndex, VkWriteDescriptorSet* aDescriptorSets, DWord nDescriptorSets) {
	auto& pipeline = hApp->m_PipelineViews[uPipelineIndex];
	Vector<VkDescriptorSetLayout> layouts (MAX_FRAMES_IN_FLIGHT, pipeline.m_DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = pipeline.m_DescriptorPool,
		.descriptorSetCount = static_cast<DWord>(MAX_FRAMES_IN_FLIGHT),
		.pSetLayouts = layouts.data ()
	};
	pipeline.m_DescriptorSets.resize (MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets (hApp->m_DeviceView.hLogicalDevice, &allocInfo, pipeline.m_DescriptorSets.data ()) != VK_SUCCESS) {
		LogErr ("Failed to allocate descriptor sets!");
		return;
	}
	Vector<VkDescriptorBufferInfo*> enumBuffInfos;
	for (DWord uImage = 0u; uImage < MAX_FRAMES_IN_FLIGHT; uImage++) {
		Vector<VkWriteDescriptorSet> localDescriptorSets;
		Vector<VkDescriptorBufferInfo> localBufferInfos;

		for (DWord uSet = 0u; uSet < nDescriptorSets; uSet++) {
			auto& srcSet = aDescriptorSets[uSet];

			VkWriteDescriptorSet localSet = srcSet;
			localSet.dstSet = pipeline.m_DescriptorSets[uImage];

			if (localSet.pBufferInfo) {
				VkDescriptorBufferInfo bufferInfo {
					.buffer = pipeline.m_UniformBindingSet.m_Buffers[uImage],
					.offset = srcSet.pBufferInfo->offset,
					.range = srcSet.pBufferInfo->range
				};
				localBufferInfos.push_back (bufferInfo);
				localSet.pBufferInfo = &localBufferInfos.back ();
			}

			localDescriptorSets.push_back (localSet);
		}

		vkUpdateDescriptorSets (
			hApp->m_DeviceView.hLogicalDevice,
			static_cast<DWord>(localDescriptorSets.size ()),
			localDescriptorSets.data (),
			0u, nullptr);
	}
}

Undef VkCreateUniformDescriptorSet (
	VkWriteDescriptorSet* pDescriptorSet,
	VkDescriptorBufferInfo* pBuffInfo,
	DWord uDstBinding,
	DWord uDstArrayElement) {
	*pDescriptorSet = VkWriteDescriptorSet {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstBinding = uDstBinding,
		.dstArrayElement = uDstArrayElement,
		.descriptorCount = 1u,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pImageInfo = nullptr,
		.pBufferInfo = pBuffInfo
	};
}

Undef VkCreateImageDescriptorSet (
	VkWriteDescriptorSet* pDescriptorSet,
	VkDescriptorImageInfo* pImgInfo,
	DWord uDstBinding,
	DWord uDstArrayElement) {
	*pDescriptorSet = VkWriteDescriptorSet {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstBinding = uDstBinding,
		.dstArrayElement = uDstArrayElement,
		.descriptorCount = 1u,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = pImgInfo,
		.pBufferInfo = nullptr
	};
}

Undef VkGenFrameBuffers (VkApplication* hApp) {
	auto& enumImageViews = hApp->m_SwapchainView.m_EnumSwapchainImageViews;
	hApp->m_FrameBuffers = Vector<VkFramebuffer> (enumImageViews.size ());
	for (Size iImage = 0u; iImage < enumImageViews.size (); ++iImage) {
		VkImageView attachments[3u] = {
			hApp->m_ColorImageBundle.m_hImageView,
			hApp->m_DepthImageBundle.m_hImageView,
			enumImageViews[iImage]
		};
		VkFramebufferCreateInfo framebufferCreateInfo {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = hApp->m_hRenderPass,
			.attachmentCount = 3u,
			.pAttachments = attachments,
			.width = hApp->m_Extent.width,
			.height = hApp->m_Extent.height,
			.layers = 1u
		};
		vkCreateFramebuffer (
			hApp->m_DeviceView.hLogicalDevice, 
			&framebufferCreateInfo, 
			nullptr, 
			&hApp->m_FrameBuffers[iImage]);
	}
}

Undef VkGenCmdPool (VkApplication* hApp) {
	VkCommandPoolCreateInfo commandPoolCreateInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = hApp->m_GraphicsQueueView.m_uQueueFamilyIndex
	};
	vkCreateCommandPool (hApp->m_DeviceView.hLogicalDevice, 
						 &commandPoolCreateInfo, 
						 nullptr, 
						 &hApp->m_hCmdPool);
}

Undef VkGenCmdBuffers (VkApplication* hApp) {
	hApp->m_CmdBuffers = Vector<VkCommandBuffer> (MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo cmdBufferAllocateInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = hApp->m_hCmdPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<DWord>(MAX_FRAMES_IN_FLIGHT)
	};
	vkAllocateCommandBuffers (
		hApp->m_DeviceView.hLogicalDevice, 
		&cmdBufferAllocateInfo, 
		hApp->m_CmdBuffers.data ());
}

Undef VkGenSyncPrimitives (VkApplication* hApp) {
	VkSemaphoreCreateInfo semaphoreInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkFenceCreateInfo fenceInfo {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (Size iImage = 0u; iImage < MAX_FRAMES_IN_FLIGHT; ++iImage) {
		VkSemaphore hSemaphore;
		VkFence hFence;
		vkCreateSemaphore (hApp->m_DeviceView.hLogicalDevice, &semaphoreInfo, nullptr, &hSemaphore);
		vkCreateFence (hApp->m_DeviceView.hLogicalDevice, &fenceInfo, nullptr, &hFence);

		hApp->m_SyncPrimitives.m_ImageAvailableSemaphores.emplace_back (hSemaphore);
		hApp->m_SyncPrimitives.m_InFlightFences.emplace_back (hFence);
	}

	for (Size iImage = 0u; iImage < hApp->m_nImages; ++iImage) {
		VkSemaphore hSemaphore;
		vkCreateSemaphore (hApp->m_DeviceView.hLogicalDevice, &semaphoreInfo, nullptr, &hSemaphore);

		hApp->m_SyncPrimitives.m_RenderFinishedSemaphores.emplace_back (hSemaphore);
	}
}

Undef VkBegin (VkApplication* hApp, VkClearColorValue const& clearColor, VkClearDepthStencilValue const& clearDepth) {
	auto& rtInfo = hApp->m_RtInfo;
	rtInfo.m_Fence = hApp->m_SyncPrimitives.m_InFlightFences[rtInfo.m_Frame];
	rtInfo.m_CmdBuffer = hApp->m_CmdBuffers[rtInfo.m_Frame];

	vkWaitForFences (hApp->m_DeviceView.hLogicalDevice, 1u, &rtInfo.m_Fence, VK_TRUE, UINT64_MAX);

	vkAcquireNextImageKHR (hApp->m_DeviceView.hLogicalDevice,
						   hApp->m_SwapchainView.m_hSwapchain,
						   UINT64_MAX, 
						   hApp->m_SyncPrimitives
								.m_ImageAvailableSemaphores[rtInfo.m_Frame], 
						   VK_NULL_HANDLE, &rtInfo.m_uImage);

	vkResetFences (hApp->m_DeviceView.hLogicalDevice, 1u, &rtInfo.m_Fence);
	vkResetCommandBuffer (rtInfo.m_CmdBuffer, 0u);

	VkCommandBufferBeginInfo beginInfo {
	.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer (rtInfo.m_CmdBuffer, &beginInfo);

	VkClearValue clearValues[2u] {
		{ .color = clearColor }, { .depthStencil = clearDepth }
	};

	VkRenderPassBeginInfo renderPassInfo {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = hApp->m_hRenderPass,
		.framebuffer = hApp->m_FrameBuffers[rtInfo.m_uImage],
		.renderArea = { { 0u, 0u }, hApp->m_Extent },
		.clearValueCount = 2u,
		.pClearValues = clearValues
	};

	vkCmdBeginRenderPass (rtInfo.m_CmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

Undef VkEnd (VkApplication* hApp) {
	auto& rtInfo = hApp->m_RtInfo;
	vkCmdEndRenderPass (rtInfo.m_CmdBuffer);
	vkEndCommandBuffer (rtInfo.m_CmdBuffer);

	VkPipelineStageFlags aWaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore aWaitSemaphores[] = { hApp->m_SyncPrimitives.m_ImageAvailableSemaphores[rtInfo.m_Frame] };
	VkSemaphore aSignalSemaphores[] = { hApp->m_SyncPrimitives.m_RenderFinishedSemaphores[rtInfo.m_uImage] };

	VkSubmitInfo submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1u,
		.pWaitSemaphores = aWaitSemaphores,
		.pWaitDstStageMask = aWaitStages,
		.commandBufferCount = 1u,
		.pCommandBuffers = &rtInfo.m_CmdBuffer,
		.signalSemaphoreCount = 1u,
		.pSignalSemaphores = aSignalSemaphores
	};
	vkQueueSubmit (hApp->m_GraphicsQueueView.m_hQueue, 1u, &submitInfo, rtInfo.m_Fence);

	VkPresentInfoKHR presentInfo {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1u,
		.pWaitSemaphores = aSignalSemaphores,
		.swapchainCount = 1u,
		.pSwapchains = &hApp->m_SwapchainView.m_hSwapchain,
		.pImageIndices = &rtInfo.m_uImage
	};
	vkQueuePresentKHR (hApp->m_PresentQueueView.m_hQueue, &presentInfo);

	rtInfo.m_Frame = (rtInfo.m_Frame + 1u) % MAX_FRAMES_IN_FLIGHT;
}

Undef VkMakeViewport (VkApplication* hApp, 
					  Dec32 fX, Dec32 fY,
					  Dec32 fW, Dec32 fH,
					  Dec32 fMinDepth, Dec32 fMaxDepth) {
	VkViewport viewport {
		.x = fX, .y = fY,
		.width = fW, .height = fH,
		.minDepth = fMinDepth, .maxDepth = fMaxDepth
	};
	vkCmdSetViewport (hApp->m_RtInfo.m_CmdBuffer, 0u, 1u, &viewport);
}

Undef VkScissor (VkApplication* hApp, 
				 DWord uX, DWord uY,
				 Width_t uW, Height_t uH) {
	VkRect2D scissor { 
		{uX, uY},
		{uW, uH}
	};
	vkCmdSetScissor (hApp->m_RtInfo.m_CmdBuffer, 0u, 1u, &scissor);
}

Undef VkBindPipeline (VkApplication* hApp, DWord uPipelineIndex) {
	auto* hPipeline = hApp->m_PipelineViews[uPipelineIndex].m_hPipeline;
	vkCmdBindPipeline (hApp->m_RtInfo.m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, hPipeline);
}

VkFormat VkGetSupportedFormat (VkApplication* hApp, const Vector<VkFormat>& enumCandidates, VkImageTiling uImgTiling, VkFormatFeatureFlags uFormatFeatureFlags) {
	for (VkFormat format : enumCandidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties (hApp->m_DeviceView.hPhysicalDevice, format, &props);

		if (uImgTiling == VK_IMAGE_TILING_LINEAR && 
			(props.linearTilingFeatures & uFormatFeatureFlags) == uFormatFeatureFlags) {
			return format;
		}
		else if (uImgTiling == VK_IMAGE_TILING_OPTIMAL && 
				 (props.optimalTilingFeatures & uFormatFeatureFlags) == uFormatFeatureFlags) {
			return format;
		}
	}
	LogErr ("Failed to find supported format!");
}

VkFormat VkGetDepthFormat (VkApplication* hApp) {
	return VkGetSupportedFormat (
		hApp,
		{ 
			VK_FORMAT_D32_SFLOAT, 
			VK_FORMAT_D32_SFLOAT_S8_UINT, 
			VK_FORMAT_D24_UNORM_S8_UINT 
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

Undef VkGenDepthImageBundle (VkApplication* hApp) {
	VkFormat depthFormat = VkGetDepthFormat (hApp);
	VkGenImage (
		hApp,
		hApp->m_Extent.width,
		hApp->m_Extent.height, 
		1u, 
		hApp->m_uSampleCount,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&hApp->m_DepthImageBundle.m_hImage,
		&hApp->m_DepthImageBundle.m_hImageMemory);
	hApp->m_DepthImageBundle.m_hImageView = VkGenImageView (
		hApp,
		hApp->m_DepthImageBundle.m_hImage,
		depthFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT, 1u);
}

Undef VkGenColorImageBundle (VkApplication* hApp) {
	VkFormat colorFormat = hApp->m_SurfaceView.m_SurfaceFormat.format;
	VkGenImage (hApp,
				hApp->m_Extent.width,
				hApp->m_Extent.height,
				1u, 
				hApp->m_uSampleCount, 
				colorFormat, 
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&hApp->m_ColorImageBundle.m_hImage, 
				&hApp->m_ColorImageBundle.m_hImageMemory);
	hApp->m_ColorImageBundle.m_hImageView = VkGenImageView (
		hApp,
		hApp->m_ColorImageBundle.m_hImage,
		colorFormat,
		VK_IMAGE_ASPECT_COLOR_BIT, 1u);
}

Undef VkGenImage(VkApplication* hApp,
	Width_t width, Height_t height, DWord nMipLevels, VkSampleCountFlagBits nSamples, 
				 VkFormat uImgFormat,
				 VkImageTiling uImgTiling,
				 VkImageUsageFlags uImgUsageFlags,
				 VkMemoryPropertyFlags uMemPropFlags, VkImage* hImage, VkDeviceMemory* hImageMemory) {
	VkImageCreateInfo imageInfo {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0u,
		.imageType = VK_IMAGE_TYPE_2D,

		.format = uImgFormat,

		.extent = {
			.width = width,
			.height = height,
			.depth = 1u
		},

		.mipLevels = nMipLevels,
		.arrayLayers = 1u,

		.samples = nSamples,
		.tiling = uImgTiling,
		.usage = uImgUsageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0u,
		.pQueueFamilyIndices = nullptr,

		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	if (vkCreateImage (hApp->m_DeviceView.hLogicalDevice, &imageInfo, nullptr, hImage) != VK_SUCCESS) {
		LogErr ("Failed to create image!");
		return;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements (hApp->m_DeviceView.hLogicalDevice, *hImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = VkFindMemoryType (hApp, memRequirements.memoryTypeBits, uMemPropFlags)
	};

	if (vkAllocateMemory (hApp->m_DeviceView.hLogicalDevice, &allocInfo, nullptr, hImageMemory) != VK_SUCCESS) {
		LogErr ("Failed to allocate image memory!");
		return;
	}

	vkBindImageMemory (hApp->m_DeviceView.hLogicalDevice, *hImage, *hImageMemory, 0u);
}

VkImageView VkGenImageView (VkApplication* hApp, VkImage hImage, VkFormat uFormat, VkImageAspectFlags uImgAspectFlags, DWord nMipLevels) {
	VkImageViewCreateInfo viewInfo {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0u,
		.image = hImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = uFormat,
		.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange = {
			.aspectMask = uImgAspectFlags,
			.baseMipLevel = 0u,
			.levelCount = nMipLevels,
			.baseArrayLayer = 0u,
			.layerCount = 1u
		}
	};

	VkImageView imageView;
	if (vkCreateImageView (hApp->m_DeviceView.hLogicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		LogErr ("Failed to create texture image view!");
		return nullptr;
	}

	return imageView;
}

Undef VkTransitionImageLayout (VkApplication* hApp, VkImage hImage, VkFormat uFormat, VkImageLayout uPrevLayout, VkImageLayout uNewLayout, DWord nMipLevels) {
	VkCommandBuffer hCmdBuffer;
	VkCmdBegin (hApp, &hCmdBuffer);

	VkImageSubresourceRange subresourceRange {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0u,
		.levelCount = nMipLevels,
		.baseArrayLayer = 0u,
		.layerCount = 1u
	};

	VkImageMemoryBarrier barrier {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = uPrevLayout,
		.newLayout = uNewLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = hImage,
		.subresourceRange = subresourceRange
	};

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (uPrevLayout == VK_IMAGE_LAYOUT_UNDEFINED && uNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0u;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (uPrevLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && uNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument ("unsupported layout transition!");
	}

	vkCmdPipelineBarrier (
		hCmdBuffer,
		sourceStage, destinationStage,
		0u,
		0u, nullptr,
		0u, nullptr,
		1u, &barrier
	);

	VkCmdEnd (hApp, hCmdBuffer);
}

Undef VkGenMipmaps (VkApplication* hApp, VkImage hImage, VkFormat uFormat, Width_t uW, Height_t uH, DWord nMipLevels) {
	VkFormatProperties formatProperties {};
	vkGetPhysicalDeviceFormatProperties (hApp->m_DeviceView.hPhysicalDevice, uFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		LogErr ("Image format doesn't support linear blitting!");
		return;
	}

	VkCommandBuffer hCmdBuffer;
	VkCmdBegin (hApp, &hCmdBuffer);

	VkImageSubresourceRange subresourceRange {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.levelCount = 1u,
		.baseArrayLayer = 0u,
		.layerCount = 1u
	};

	VkImageMemoryBarrier barrier {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = hImage,
		.subresourceRange = subresourceRange
	};

	SInt32 iMipWidth = uW;
	SInt32 iMipHeight = uH;

	for (DWord uLevel = 1u; uLevel < nMipLevels; uLevel++) {
		barrier.subresourceRange.baseMipLevel = uLevel - 1u;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier (hCmdBuffer,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
							  0u, nullptr,
							  0u, nullptr,
							  1u, &barrier);

		VkImageBlit blit {
			.srcSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = uLevel - 1u,
				.baseArrayLayer = 0u,
				.layerCount = 1u
			},
			.srcOffsets = {
				{0, 0, 0},
				{iMipWidth, iMipHeight, 1}
			},
			.dstSubresource = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = uLevel,
				.baseArrayLayer = 0u,
				.layerCount = 1u
			},
			.dstOffsets = {
				{0, 0, 0},
				{
					iMipWidth > 1 ? iMipWidth / 2 : 1,
					iMipHeight > 1 ? iMipHeight / 2 : 1,
					1
				}
			}
		};

		vkCmdBlitImage (hCmdBuffer,
						hImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						hImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1u, &blit,
						VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier (hCmdBuffer,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							  0, nullptr,
							  0, nullptr,
							  1, &barrier);

		if (iMipWidth > 1) {
			iMipWidth /= 2;
		}

		if (iMipHeight > 1) {
			iMipHeight /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = nMipLevels - 1u;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier (hCmdBuffer,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						  0u, nullptr,
						  0u, nullptr,
						  1u, &barrier);

	VkCmdEnd (hApp, hCmdBuffer);
}

Undef VkImageBufferCpy (VkApplication* hApp, VkBuffer hBuffer, VkImage hImage, Width_t uW, Height_t uH) {
	VkCommandBuffer hCmdBuffer;
	VkCmdBegin (hApp, &hCmdBuffer);

	VkImageSubresourceLayers subresource {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0u,
		.baseArrayLayer = 0u,
		.layerCount = 1u
	};

	VkBufferImageCopy region {
		.bufferOffset = 0u,
		.bufferRowLength = 0u,
		.bufferImageHeight = 0u,
		.imageSubresource = subresource,
		.imageOffset = { Zero, Zero, Zero },
		.imageExtent = {
			uW,
			uH,
			1u
		}
	};

	vkCmdCopyBufferToImage (hCmdBuffer, hBuffer, hImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);

	VkCmdEnd (hApp, hCmdBuffer);
}

Undef VkTexSampler (VkApplication* hApp,
					VkTexture* hTexture,
					VkFilter uMinFilter, VkFilter uMagFilter,
					VkSamplerAddressMode uAddrModeU,
					VkSamplerAddressMode uAddrModeV,
					VkSamplerAddressMode uAddrModeW, DWord nMipLevels, VkSamplerMipmapMode uMipmapMode) {
	VkPhysicalDeviceProperties properties {};
	vkGetPhysicalDeviceProperties (hApp->m_DeviceView.hPhysicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0u,
		.magFilter = uMagFilter,
		.minFilter = uMinFilter,
		.mipmapMode = uMipmapMode,
		.addressModeU = uAddrModeU,
		.addressModeV = uAddrModeV,
		.addressModeW = uAddrModeW,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_TRUE,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0.0f,
		.maxLod = static_cast<Dec32>(nMipLevels),
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK
	};

	if (vkCreateSampler (hApp->m_DeviceView.hLogicalDevice, &samplerInfo, nullptr, &hTexture->m_Sampler) != VK_SUCCESS) {
		LogErr ("Failed to create texture sampler!");
	}
}

Undef VkTexImageView (VkApplication* hApp, VkTexture* hTexture, VkFormat uFormat, DWord nMipLevels) {
	hTexture->m_ImageView = VkGenImageView (hApp, hTexture->m_Image, uFormat, VK_IMAGE_ASPECT_COLOR_BIT, nMipLevels);
}

Undef VkTexImage (VkApplication* hApp, VkTexture* hTexture, UInt8* pData, Width_t uW, Height_t uH, DWord nChannels, DWord nMipLevels, VkFormat uFormat) {
	VkDeviceSize uImageSize = static_cast<VkDeviceSize>(uW * uH * nChannels);

	VkBuffer pStagingBuffer;
	VkDeviceMemory pStagingBufferMemory;
	VkGenBuffer (hApp,
				 uImageSize, 
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				 &pStagingBuffer, &pStagingBufferMemory);

	Undef* pDestBuff;
	vkMapMemory (hApp->m_DeviceView.hLogicalDevice, pStagingBufferMemory, 0u, uImageSize, 0u, &pDestBuff);
	memcpy (pDestBuff, pData, uImageSize);
	vkUnmapMemory (hApp->m_DeviceView.hLogicalDevice, pStagingBufferMemory);

	VkGenImage (hApp,
				uW, uH,
				nMipLevels, VK_SAMPLE_COUNT_1_BIT,
				uFormat,
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				&hTexture->m_Image, &hTexture->m_ImageMemory);

	VkTransitionImageLayout (hApp, hTexture->m_Image, uFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, nMipLevels);
	VkImageBufferCpy (hApp, pStagingBuffer, hTexture->m_Image, uW, uH);
	//VkTransitionImageLayout (hApp, hTexture->m_Image, uFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, nMipLevels);

	vkDestroyBuffer (hApp->m_DeviceView.hLogicalDevice, pStagingBuffer, nullptr);
	vkFreeMemory (hApp->m_DeviceView.hLogicalDevice, pStagingBufferMemory, nullptr);

	VkGenMipmaps (hApp, hTexture->m_Image, uFormat, uW, uH, nMipLevels);
}

Undef VkCmdBegin (VkApplication* hApp, VkCommandBuffer* hCmdBuffer) {
	VkCommandBufferAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = hApp->m_hCmdPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1u
	};

	vkAllocateCommandBuffers (hApp->m_DeviceView.hLogicalDevice, &allocInfo, hCmdBuffer);

	VkCommandBufferBeginInfo beginInfo {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer (*hCmdBuffer, &beginInfo);
}

Undef VkCmdEnd (VkApplication* hApp, VkCommandBuffer hCmdBuffer) {
	vkEndCommandBuffer (hCmdBuffer);

	VkSubmitInfo submitInfo {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1u,
		.pCommandBuffers = &hCmdBuffer
	};

	vkQueueSubmit (hApp->m_GraphicsQueueView.m_hQueue, 1u, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle (hApp->m_PresentQueueView.m_hQueue);

	vkFreeCommandBuffers (hApp->m_DeviceView.hLogicalDevice, hApp->m_hCmdPool, 1u, &hCmdBuffer);
}

Undef VkGenBuffer (VkApplication* hApp, VkDeviceSize uBuffSize, VkBufferUsageFlags uBuffUsageFlags, VkMemoryPropertyFlags uMemPropFlags, VkBuffer* hBuffRef, VkDeviceMemory* hBuffMemoryRef) {
	VkBufferCreateInfo bufferInfo {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = uBuffSize,
		.usage = uBuffUsageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	if (vkCreateBuffer (hApp->m_DeviceView.hLogicalDevice, &bufferInfo, nullptr, hBuffRef) != VK_SUCCESS) {
		LogErr ("Failed to create vertex buffer!");
		return;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements (hApp->m_DeviceView.hLogicalDevice, *hBuffRef, &memRequirements);

	VkMemoryAllocateInfo allocInfo {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = VkFindMemoryType (hApp, memRequirements.memoryTypeBits, uMemPropFlags)
	};

	if (vkAllocateMemory (hApp->m_DeviceView.hLogicalDevice, &allocInfo, nullptr, hBuffMemoryRef) != VK_SUCCESS) {
		LogErr ("Failed to allocate vertex buffer memory!");
		return;
	}

	vkBindBufferMemory (hApp->m_DeviceView.hLogicalDevice, *hBuffRef, *hBuffMemoryRef, 0);
}

Undef VkCpyBuffer (VkApplication* hApp, VkBuffer hSrcBuff, VkBuffer hDstBuff, VkDeviceSize uBuffSize) {
	VkCommandBuffer hCmdBuff;
	VkCmdBegin (hApp, &hCmdBuff);

	VkBufferCopy cpyRegion {
		.size = uBuffSize
	};

	vkCmdCopyBuffer (hCmdBuff, hSrcBuff, hDstBuff, 1u, &cpyRegion);
	
	VkCmdEnd (hApp, hCmdBuff);
}

Undef VkGenVertexBuffer (VkApplication* hApp, VkBuffer* hBuffRef, VkDeviceMemory* hBuffMemoryRef, VkVertex* aVertices, Size nVertices) {
	VkDeviceSize uBuffSize = sizeof (VkVertex) * nVertices;

	VkBuffer hStagingBuff;
	VkDeviceMemory hStagingBuffMemory;
	VkGenBuffer (hApp, uBuffSize,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 &hStagingBuff, &hStagingBuffMemory);

	Undef* hData;
	vkMapMemory (hApp->m_DeviceView.hLogicalDevice, hStagingBuffMemory, 0u, uBuffSize, 0u, &hData);
	std::memcpy (hData, aVertices, uBuffSize);
	vkUnmapMemory (hApp->m_DeviceView.hLogicalDevice, hStagingBuffMemory);

	VkGenBuffer (hApp,
				 uBuffSize,
				 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 hBuffRef, hBuffMemoryRef);

	VkCpyBuffer (hApp, hStagingBuff, *hBuffRef, uBuffSize);

	vkDestroyBuffer (hApp->m_DeviceView.hLogicalDevice, hStagingBuff, nullptr);
	vkFreeMemory (hApp->m_DeviceView.hLogicalDevice, hStagingBuffMemory, nullptr);
}

Undef VkGenIndexBuffer (VkApplication* hApp, VkBuffer* hBuffRef, VkDeviceMemory* hBuffMemoryRef, VkIndex* aIndices, Size nIndices) {
	VkDeviceSize uBuffSize = sizeof (VkIndex) * nIndices;

	VkBuffer hStagingBuff;
	VkDeviceMemory hStagingBuffMemory;
	VkGenBuffer (hApp,
				 uBuffSize,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 &hStagingBuff, &hStagingBuffMemory);

	Undef* hData;
	vkMapMemory (hApp->m_DeviceView.hLogicalDevice, hStagingBuffMemory, 0u, uBuffSize, 0u, &hData);
	std::memcpy (hData, aIndices, uBuffSize);
	vkUnmapMemory (hApp->m_DeviceView.hLogicalDevice, hStagingBuffMemory);

	VkGenBuffer (hApp, 
				 uBuffSize,
				 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 hBuffRef, hBuffMemoryRef);

	VkCpyBuffer (hApp, hStagingBuff, *hBuffRef, uBuffSize);

	vkDestroyBuffer (hApp->m_DeviceView.hLogicalDevice, hStagingBuff, nullptr);
	vkFreeMemory (hApp->m_DeviceView.hLogicalDevice, hStagingBuffMemory, nullptr);
}

Undef VkGenUniformBuffers (VkApplication* hApp, DWord uPipelineIndex, VkDeviceSize uUBOSize) {
	auto& pipeline = hApp->m_PipelineViews[uPipelineIndex];

	pipeline.m_UniformBindingSet.m_Buffers.resize(MAX_FRAMES_IN_FLIGHT);
	pipeline.m_UniformBindingSet.m_Memories.resize (MAX_FRAMES_IN_FLIGHT);
	pipeline.m_UniformBindingSet.m_Mapped.resize (MAX_FRAMES_IN_FLIGHT);

	for (Size uBuffer = 0u; uBuffer < MAX_FRAMES_IN_FLIGHT; uBuffer++) {
		VkGenBuffer (
			hApp,
			uUBOSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&pipeline.m_UniformBindingSet.m_Buffers[uBuffer],
			&pipeline.m_UniformBindingSet.m_Memories[uBuffer]);

		vkMapMemory (
			hApp->m_DeviceView.hLogicalDevice,
			pipeline.m_UniformBindingSet.m_Memories[uBuffer],
			0u,
			uUBOSize,
			0u,
			&pipeline.m_UniformBindingSet.m_Mapped[uBuffer]);
	}
}

Undef VkFillUniformBuffer (VkApplication* hApp, DWord uPipelineIndex, Undef* hData, VkDeviceSize uUBOSize) {
	auto& pipeline = hApp->m_PipelineViews[uPipelineIndex];
	std::memcpy (pipeline.m_UniformBindingSet.m_Mapped[hApp->m_RtInfo.m_Frame], hData, uUBOSize);
}

Undef VkEnableDepthTest (VkPipelineDepthStencilStateCreateInfo* pDepthStencilStateCreateInfo) {
	*pDepthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0u,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f
	};
}

Undef VkDisableDepthTest (VkPipelineDepthStencilStateCreateInfo* pDepthStencilStateCreateInfo) {
	*pDepthStencilStateCreateInfo = VkPipelineDepthStencilStateCreateInfo {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0u,
		.depthTestEnable = VK_FALSE,
		.depthWriteEnable = VK_FALSE,
		.depthCompareOp = VK_COMPARE_OP_ALWAYS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f
	};
}