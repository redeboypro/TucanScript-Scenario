#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../ThirdParty/lodepng.h"
#include "../SystemModules/Graphics/Vulkan.h"

inline DWord GetMipmapCnt (Width_t uW, Height_t uH) {
	return static_cast<DWord>(std::floor (std::log2 (Max (uW, uH)))) + 1u;
}

static Undef ReadShaderAssembly (const String& fName, ShaderCode* pShaderCode, Size* pShaderSize) {
    IFileStream file (fName, std::ios::ate | std::ios::binary);

    if (!file.is_open ()) {
        throw std::runtime_error ("Failed to open file!");
    }

    *pShaderSize = (Size) file.tellg ();
	*pShaderCode = (ShaderCode) std::malloc (*pShaderSize);

    file.seekg (Zero);
    file.read (reinterpret_cast<Sym*>(*pShaderCode), *pShaderSize);

    file.close ();
}

struct UniformBufferObject {
	alignas(16) glm::mat4 m_ModelMat, m_ProjMat;
};

ProgramExitCode_t main () {
	VkApplication* hApp;
	VkMakeApp (&hApp, "Vulkan Test", 800, 600);
	VkCreateSwapchain (hApp);
	VkGetSwapchainImages (hApp);
	VkCreateRenderPass (hApp);

	VkDescriptorSetLayoutBinding uboLayoutBinding {
		.binding = 0u,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = 1u,
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	};

	VkDescriptorSetLayoutBinding texLayoutBinding {
		.binding = 1u,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1u,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	};

	VkDescriptorSetLayoutBinding aLayoutBindings[2] {
		uboLayoutBinding,
		texLayoutBinding
	};

	VkCreateDescriptorSetLayout (hApp, 0u, aLayoutBindings, 2u);

	ShaderCode pVertShaderAsmCode;
	Size uVertShaderSize;
	ReadShaderAssembly ("vert.spv", &pVertShaderAsmCode, &uVertShaderSize);
	ShaderCode pFragShaderAsmCode;
	Size uFragShaderSize;
	ReadShaderAssembly ("frag.spv", &pFragShaderAsmCode, &uFragShaderSize);
	VkCreateShaderModules (hApp, 
						   pVertShaderAsmCode, uVertShaderSize,
						   pFragShaderAsmCode, uFragShaderSize,
						   "main", 0u);

	VkPipelineDepthStencilStateCreateInfo pipelineDepthInfo;
	VkEnableDepthTest (&pipelineDepthInfo);

	VkCreatePipeline(hApp, 0u, pipelineDepthInfo);

	VkGenCmdPool (hApp);
	VkGenColorImageBundle (hApp);
	VkGenDepthImageBundle (hApp);
	VkGenFrameBuffers (hApp);

	VkTexture texture {};
	Width_t uTexWidth = 256u;
	Height_t uTexHeight = 256u;
	Vector<UInt8> textureBuffer;
	lodepng::decode(textureBuffer, uTexWidth, uTexHeight, "test.png");
	DWord nMipLevels = GetMipmapCnt (uTexWidth, uTexHeight);
	VkTexImage(hApp, &texture, textureBuffer.data (), uTexWidth, uTexHeight, 4u, nMipLevels, VK_FORMAT_R8G8B8A8_SRGB);
	VkTexImageView(hApp, &texture, VK_FORMAT_R8G8B8A8_SRGB, nMipLevels);
	VkTexSampler(hApp, &texture, 
				VK_FILTER_LINEAR, VK_FILTER_LINEAR,
				VK_SAMPLER_ADDRESS_MODE_REPEAT,
				VK_SAMPLER_ADDRESS_MODE_REPEAT,
				VK_SAMPLER_ADDRESS_MODE_REPEAT, 
				nMipLevels, 
				VK_SAMPLER_MIPMAP_MODE_LINEAR);

	VkDescriptorPoolSize uboPoolSize {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = MAX_FRAMES_IN_FLIGHT
	};

	VkDescriptorPoolSize texPoolSize {
		.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = MAX_FRAMES_IN_FLIGHT
	};

	VkDescriptorPoolSize aPoolSizes[2u] {
		uboPoolSize,
		texPoolSize
	};

	VkGenUniformBuffers (hApp, 0u, sizeof (UniformBufferObject));
	VkCreateDescriptorPool (hApp, 0u, aPoolSizes, 2u);

	VkDescriptorBufferInfo uboDescriptorBufferInfo {
		.offset = 0u,
		.range = sizeof (UniformBufferObject)
	};

	VkWriteDescriptorSet uboDescriptorSet;
	VkCreateUniformDescriptorSet (&uboDescriptorSet, &uboDescriptorBufferInfo, 0u, 0u);

	VkDescriptorImageInfo texDescriptorImageInfo {
		.sampler = texture.m_Sampler,
		.imageView = texture.m_ImageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	VkWriteDescriptorSet imgDescriptorSet;
	VkCreateImageDescriptorSet (&imgDescriptorSet, &texDescriptorImageInfo, 1u, 0u);
	
	VkWriteDescriptorSet writeDescriptorSets[2] {
		uboDescriptorSet,
		imgDescriptorSet
	};

	VkCreateDescriptorSets (hApp, 0u, writeDescriptorSets, 2u);

	VkGenCmdBuffers (hApp);
	VkGenSyncPrimitives (hApp);

	VkBuffer hVertexBuffer;
	VkDeviceMemory hVertexBufferMemory;

	Vector<VkVertex> vertices = {
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},

		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},

		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},

		{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},

		{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},

		{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
	};

	VkGenVertexBuffer (hApp,
					   &hVertexBuffer, &hVertexBufferMemory,
					   vertices.data (), vertices.size ());

	VkBuffer hIndexBuffer;
	VkDeviceMemory hIndexBufferMemory;

	Vector<VkIndex> indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};

	VkGenIndexBuffer (hApp,
					  &hIndexBuffer, &hIndexBufferMemory,
					  indices.data (), indices.size ());
	
	Dec32 fRotation = 0.0f;
	Dec64 fLastFrameTime = glfwGetTime ();
	while (!glfwWindowShouldClose(hApp->m_hWindow)) {
		glfwPollEvents ();

		Dec64 fCurrentFrameTime = glfwGetTime ();
		Dec64 fDeltaTime = fCurrentFrameTime - fLastFrameTime;
		fLastFrameTime = fCurrentFrameTime;

		fRotation += fDeltaTime;

		glm::mat4 model = glm::mat4 (1.0f);

		model = glm::translate (model, { 0.0f, 0.0f, -10.0f });
		model = glm::rotate (model, fRotation, glm::vec3 (1.0f, 0.0f, 0.0f));
		model = glm::rotate (model, fRotation, glm::vec3 (0.0f, 1.0f, 0.0f));
		model = glm::rotate (model, fRotation, glm::vec3 (0.0f, 0.0f, 1.0f));
		model = glm::scale (model, { 3.0f, 3.0f, 3.0f });

		UniformBufferObject ubo {
			.m_ModelMat = model,
			.m_ProjMat = glm::perspective (glm::radians (45.0f), 800.0f / 600.0f, 0.01f, 100.0f)
		};

		VkBegin (hApp, {0.0f, 0.5f, 0.8f, 1.0f}, {1.0f, 0});

		VkFillUniformBuffer (hApp, 0u, &ubo, sizeof (UniformBufferObject));

		VkBindPipeline (hApp, 0u);
		VkMakeViewport (hApp,
						0.0f, 0.0f,
						hApp->m_Extent.width, hApp->m_Extent.height,
						0.0f, 1.0f);
		VkScissor (hApp,
				   0u, 0u,
				   hApp->m_Extent.width, hApp->m_Extent.height);

		VkDeviceSize aOffsets[] = { 0u };
		vkCmdBindVertexBuffers (hApp->m_RtInfo.m_CmdBuffer, 0u, 1u, &hVertexBuffer, aOffsets);
		vkCmdBindIndexBuffer (hApp->m_RtInfo.m_CmdBuffer, hIndexBuffer, 0u, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets (hApp->m_RtInfo.m_CmdBuffer, 
								 VK_PIPELINE_BIND_POINT_GRAPHICS, 
								 hApp->m_PipelineViews[0u].m_hPipelineLayout,
								 0u, 
								 1u,
								 &hApp->m_PipelineViews[0u].m_DescriptorSets[hApp->m_RtInfo.m_Frame],
								 0u, 
								 nullptr);

		vkCmdDrawIndexed (hApp->m_RtInfo.m_CmdBuffer, static_cast<DWord>(indices.size ()), 1u, 0u, 0u, 0u);

		VkEnd (hApp);
	}

	return 1;
}