#ifndef VULKAN_H
#define VULKAN_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../../Utility.h"

using namespace TucanScript;

#ifndef NUM_PIPELINES
#define NUM_PIPELINES 2u
#endif

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2u
#endif

typedef uint32_t Width_t;
typedef uint32_t Height_t;
typedef const Sym* RCStr;

struct VkAppDeviceView {
	VkPhysicalDevice hPhysicalDevice;
	VkDevice hLogicalDevice;
};

struct VkSurfaceView {
	VkSurfaceKHR m_hSurface;
	VkSurfaceFormatKHR m_SurfaceFormat;
	VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
};

struct VkSwapchainView {
	VkSwapchainKHR m_hSwapchain;
	Vector<VkImage> m_EnumSwapchainImages;
	Vector<VkImageView> m_EnumSwapchainImageViews;
};

struct VkQueueView {
	VkQueue m_hQueue;
	DWord m_uQueueFamilyIndex;
};

struct VkShaderView {
	VkShaderModule m_hFragmentShaderModule;
	VkShaderModule m_hVertexShaderModule;
	VkPipelineShaderStageCreateInfo m_aShaderStages[2];
};

struct VkPipelineView {
	VkPipeline m_hPipeline;
	VkPipelineLayout m_hPipelineLayout;
	VkShaderView m_ShaderView;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	Vector<VkDescriptorSet> m_DescriptorSets;
	VkDescriptorPool m_DescriptorPool;

	typedef struct {
		Vector<VkBuffer> m_Buffers;
		Vector<VkDeviceMemory> m_Memories;
		Vector<Undef*> m_Mapped;
	} VkUniformBindingSet;

	VkUniformBindingSet m_UniformBindingSet;
};

struct VkSyncPrimitives {
	Vector<VkSemaphore> m_ImageAvailableSemaphores;
	Vector<VkSemaphore> m_RenderFinishedSemaphores;
	Vector<VkFence> m_InFlightFences;
};

struct VkImageBundle {
	VkImage m_hImage;
	VkDeviceMemory m_hImageMemory;
	VkImageView m_hImageView;
};

struct VkVertex {
	Dec32 m_Position[3];
	Dec32 m_UV[2];
};

struct VkRuntimeInfo {
	DWord m_Frame { 0u };
	DWord m_uImage { 0u };
	VkCommandBuffer m_CmdBuffer;
	VkFence m_Fence;
};

struct VkTexture {
	VkImage m_Image;
	VkDeviceMemory m_ImageMemory;
	VkImageView m_ImageView;
	VkSampler m_Sampler;
	DWord m_uMipLevels;
};

typedef uint16_t VkIndex;
typedef uint32_t* ShaderCode;

struct VkApplication {
	GLFWwindow* m_hWindow;
	VkInstance m_hInstance;
	VkAppDeviceView m_DeviceView;
	VkSurfaceView m_SurfaceView;
	VkQueueView m_GraphicsQueueView;
	VkQueueView m_PresentQueueView;
	VkPresentModeKHR m_PresentMode;
	VkSwapchainView m_SwapchainView;
	VkImageBundle m_DepthImageBundle;
	VkImageBundle m_ColorImageBundle;
	VkCommandPool m_hCmdPool;
	Vector<VkCommandBuffer> m_CmdBuffers;
	VkPipelineView m_PipelineViews[NUM_PIPELINES];
	VkExtent2D m_Extent;
	Vector<VkFramebuffer> m_FrameBuffers;
	VkSyncPrimitives m_SyncPrimitives;
	DWord m_nImages;
	VkRenderPass m_hRenderPass;
	VkSampleCountFlagBits m_uSampleCount;
	VkRuntimeInfo m_RtInfo;
};

Undef VkMakeApp (VkApplication** hAppPtr, RCStr sAppTitle, const Width_t iW, const Height_t iH);
Undef VkCreateSwapchain (VkApplication* hApp);
Undef VkGetSwapchainImages (VkApplication* hApp);
DWord VkFindMemoryType (VkApplication* hApp, DWord uTypeFilter, VkMemoryPropertyFlags uMemPropFlags);
Undef VkCreateRenderPass (VkApplication* hApp);
Undef VkCreateShaderModules(VkApplication* hApp, 
									ShaderCode pVertexShaderSrc, Size uVertexShaderSize, 
									ShaderCode pFragmentShaderSrc, Size uFragmentShaderSize,
									RCStr sEntryPt,
									DWord uPipelineIndex);
Undef VkCreatePipeline(VkApplication* hApp, DWord uPipelineIndex, const VkPipelineDepthStencilStateCreateInfo& depthStencilCreateInfo);
Undef VkCreateDescriptorPool (VkApplication* hApp, DWord uPipelineIndex, VkDescriptorPoolSize* aPoolSizes, DWord nPoolSizes);
Undef VkCreateDescriptorSetLayout (VkApplication* hApp, 
										  DWord uPipelineIndex,
										  VkDescriptorSetLayoutBinding* aBindings,
										  DWord nBindings);
Undef VkCreateDescriptorSets (VkApplication* hApp, DWord uPipelineIndex, VkWriteDescriptorSet* aDescriptorSets, DWord nDescriptorSets);
Undef VkCreateUniformDescriptorSet(VkWriteDescriptorSet* pDescriptorSet, 
										  VkDescriptorBufferInfo* pBuffInfo,
										  DWord uDstBinding,
										  DWord uDstArrayElement);
Undef VkCreateImageDescriptorSet (VkWriteDescriptorSet* pDescriptorSet,
										 VkDescriptorImageInfo* pImgInfo,
										 DWord uDstBinding,
										 DWord uDstArrayElement);
Undef VkGenFrameBuffers(VkApplication* hApp);
Undef VkGenCmdPool (VkApplication* hApp);
Undef VkGenCmdBuffers (VkApplication* hApp);
Undef VkGenSyncPrimitives (VkApplication* hApp);
Undef VkBegin(VkApplication* hApp, VkClearColorValue const& clearColor, VkClearDepthStencilValue const& clearDepth);
Undef VkEnd (VkApplication* hApp);
Undef VkMakeViewport (VkApplication* hApp, 
							 Dec32 fX, Dec32 fY,
							 Dec32 fW, Dec32 fH,
							 Dec32 fMinDepth, Dec32 fMaxDepth);
Undef VkScissor (VkApplication* hApp,
						DWord uX, DWord uY,
						Width_t uW, Height_t uH);
Undef VkBindPipeline (VkApplication* hApp, DWord uPipelineIndex);
VkFormat VkGetSupportedFormat (VkApplication* hApp, const Vector<VkFormat>& enumCandidates, VkImageTiling uImgTiling, VkFormatFeatureFlags uFormatFeatureFlags);
VkFormat VkGetDepthFormat (VkApplication* hApp);
Undef VkGenDepthImageBundle (VkApplication* hApp);
Undef VkGenColorImageBundle (VkApplication* hApp);
Undef VkGenImage(VkApplication* hApp,
				 Width_t width, Height_t height, 
				 DWord nMipLevels, VkSampleCountFlagBits nSamples, 
				 VkFormat uImgFormat, 
				 VkImageTiling uImgTiling,
				 VkImageUsageFlags uImgUsageFlags, 
				 VkMemoryPropertyFlags uMemPropFlags, 
				 VkImage* hImage, VkDeviceMemory* hImageMemory);
VkImageView VkGenImageView(VkApplication* hApp, VkImage hImage, VkFormat uFormat, VkImageAspectFlags uImgAspectFlags, DWord nMipLevels);
Undef VkTransitionImageLayout(VkApplication* hApp, VkImage hImage, VkFormat uFormat, VkImageLayout uPrevLayout, VkImageLayout uNewLayout, DWord nMipLevels);
Undef VkGenMipmaps (VkApplication* hApp, 
					VkImage hImage, 
					VkFormat uFormat, 
					Width_t uW, Height_t uH,
					DWord nMipLevels);
Undef VkImageBufferCpy (VkApplication* hApp, VkBuffer hBuffer, VkImage hImage, Width_t uW, Height_t uH);
Undef VkTexSampler(VkApplication* hApp, VkTexture* hTexture, 
					VkFilter uMinFilter, VkFilter uMagFilter,
					VkSamplerAddressMode uAddrModeU,
					VkSamplerAddressMode uAddrModeV, 
					VkSamplerAddressMode uAddrModeW, DWord nMipLevels, VkSamplerMipmapMode uMipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR);
Undef VkTexImageView(VkApplication* hApp, VkTexture* hTexture, VkFormat uFormat, DWord nMipLevels);
Undef VkTexImage(VkApplication* hApp, 
				  VkTexture* hTexture, 
				  UInt8* pData, 
				  Width_t uW, Height_t uH,
				  DWord nChannels, DWord nMipLevels, VkFormat uFormat);
Undef VkCmdBegin (VkApplication* hApp, VkCommandBuffer* hCmdBuffer);
Undef VkCmdEnd (VkApplication* hApp, VkCommandBuffer hCmdBuffer);
Undef VkGenBuffer (
	VkApplication* hApp,
	VkDeviceSize uBuffSize,
	VkBufferUsageFlags uBuffUsageFlags,
	VkMemoryPropertyFlags uMemPropFlags,
	VkBuffer* hBuffRef,
	VkDeviceMemory* hBuffMemoryRef);
Undef VkCpyBuffer (
	VkApplication* hApp,
	VkBuffer hSrcBuff,
	VkBuffer hDstBuff,
	VkDeviceSize uBuffSize);
Undef VkGenVertexBuffer (VkApplication* hApp, 
								VkBuffer* hBuffRef,
								VkDeviceMemory* hBuffMemoryRef,
								VkVertex* aVertices, Size nVertices);
Undef VkGenIndexBuffer (VkApplication* hApp,
							   VkBuffer* hBuffRef,
							   VkDeviceMemory* hBuffMemoryRef,
							   VkIndex* aIndices, Size nIndices);
Undef VkGenUniformBuffers (VkApplication* hApp,
								  DWord uPipelineIndex,
								  VkDeviceSize uUBOSize);
Undef VkFillUniformBuffer (VkApplication* hApp, 
								  DWord uPipelineIndex,
								  Undef* hData,
								  VkDeviceSize uUBOSize);
Undef VkEnableDepthTest (VkPipelineDepthStencilStateCreateInfo* pDepthStencilStateCreateInfo);
Undef VkDisableDepthTest (VkPipelineDepthStencilStateCreateInfo* pDepthStencilStateCreateInfo);
#endif