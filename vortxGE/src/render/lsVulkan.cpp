/**
 * @file lsVulkan.cpp
 * @brief lsVulkan — V1: windowed rendering core.
 *
 * Implemented for real: Probe, Init (+helpers), SetWindow, BeginFrame,
 * EndFrame, Present, Resize, Clear, ClearColor, ClearDepth, CheckErrors,
 * GetError, StoreDefaultFramebuffer, RestoreDefaultFramebuffer, Viewport,
 * destructor teardown. Everything else is a V2+ stub (end of file).
 *
 * V1 has NO pipelines: the window clear happens via the render pass
 * color attachment loadOp (and vkCmdClearAttachments for Clear()).
 * Viewport/scissor become relevant with the first draws in V2.
 */

// lsVulkan.h FIRST: glfw3.h only declares glfwCreateWindowSurface etc.
// when VK_VERSION_1_0 is already defined.
#include "lsVulkan.h"
#include "lsVulkanTypes.h"

#define GLFW_INCLUDE_NONE		// keep glfw3.h from pulling in GL headers
#include <GLFW/glfw3.h>

#include <cstring>
#include <set>
#include <algorithm>
#include <limits>

using namespace std;

namespace
{
#ifndef NDEBUG
	const bool kValidation = true;		// Debug builds: request the Khronos layer if present
#else
	const bool kValidation = false;
#endif

	const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT* data,
		void* )
	{
		if( severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
			cout << "VULKAN [" << data->pMessageIdName << "]: " << data->pMessage << endl;
		return VK_FALSE;
	}

	bool validationLayerAvailable()
	{
		uint32_t count = 0;
		vkEnumerateInstanceLayerProperties( &count, nullptr );
		std::vector<VkLayerProperties> layers( count );
		vkEnumerateInstanceLayerProperties( &count, layers.data() );
		for( const auto& l : layers )
			if( strcmp( l.layerName, kValidationLayer ) == 0 )
				return true;
		return false;
	}
}

// ---------------------------------------------------------------------
// Probe — used before window creation (vortxGE::PreferredRendererType)
// ---------------------------------------------------------------------
bool lsVulkan::Probe()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "vortxGE-probe";
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &appInfo;

	VkInstance inst = VK_NULL_HANDLE;
	if( vkCreateInstance( &ci, nullptr, &inst ) != VK_SUCCESS )
		return false;

	uint32_t count = 0;
	vkEnumeratePhysicalDevices( inst, &count, nullptr );
	vkDestroyInstance( inst, nullptr );
	return count > 0;
}

// ---------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------
lsVulkan::lsVulkan()
{
	setType( RenderType::Vulkan );
}

lsVulkan::~lsVulkan()
{
	if(! m_initialized ) return;

	vkDeviceWaitIdle( m_device );

	// V2 resources
	for( auto& [key, pipeline] : m_pipelineCache )
		vkDestroyPipeline( m_device, pipeline, nullptr );
	m_pipelineCache.clear();
	for( auto& [id, prog] : m_programs )
	{
		if( prog.vsModule ) vkDestroyShaderModule( m_device, prog.vsModule, nullptr );
		if( prog.fsModule ) vkDestroyShaderModule( m_device, prog.fsModule, nullptr );
		if( prog.setLayout ) vkDestroyDescriptorSetLayout( m_device, prog.setLayout, nullptr );
		if( prog.pipelineLayout ) vkDestroyPipelineLayout( m_device, prog.pipelineLayout, nullptr );
	}
	m_programs.clear();
	for( auto* infos : m_descriptorImageInfos ) delete infos;
	m_descriptorImageInfos.clear();
	for( auto& [id, buf] : m_buffers )
	{
		if( buf.buffer ) vkDestroyBuffer( m_device, buf.buffer, nullptr );
		if( buf.memory ) vkFreeMemory( m_device, buf.memory, nullptr );
	}
	m_buffers.clear();
	if( m_uboBuffer ) vkDestroyBuffer( m_device, m_uboBuffer, nullptr );
	if( m_uboMemory ) vkFreeMemory( m_device, m_uboMemory, nullptr );
	if( m_descriptorPool ) vkDestroyDescriptorPool( m_device, m_descriptorPool, nullptr );
	if( m_dummySampler ) vkDestroySampler( m_device, m_dummySampler, nullptr );
	if( m_dummyView ) vkDestroyImageView( m_device, m_dummyView, nullptr );
	if( m_dummyImage ) vkDestroyImage( m_device, m_dummyImage, nullptr );
	if( m_dummyMemory ) vkFreeMemory( m_device, m_dummyMemory, nullptr );

	cleanupSwapchain();

	for( uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++ )
	{
		if( m_imageAvailable[i] ) vkDestroySemaphore( m_device, m_imageAvailable[i], nullptr );
		if( m_renderFinished[i] ) vkDestroySemaphore( m_device, m_renderFinished[i], nullptr );
		if( m_inFlight[i] ) vkDestroyFence( m_device, m_inFlight[i], nullptr );
	}
	if( m_commandPool ) vkDestroyCommandPool( m_device, m_commandPool, nullptr );
	if( m_renderPass ) vkDestroyRenderPass( m_device, m_renderPass, nullptr );
	if( m_device ) vkDestroyDevice( m_device, nullptr );
	if( m_surface ) vkDestroySurfaceKHR( m_instance, m_surface, nullptr );

	if( m_debugMessenger )
	{
		auto destroyFn = (PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr( m_instance, "vkDestroyDebugUtilsMessengerEXT" );
		if( destroyFn ) destroyFn( m_instance, m_debugMessenger, nullptr );
	}
	if( m_instance ) vkDestroyInstance( m_instance, nullptr );

	m_initialized = false;
}

void lsVulkan::SetWindow( void* nativeWindow )
{
	m_window = nativeWindow;
}

bool lsVulkan::Init()
{
	// vortxGE::Init() calls Init() once during selection and once in its
	// subsystem init — must be idempotent.
	if( m_initialized ) return true;

	if(! m_window )
	{
		cout << "ERROR: lsVulkan::Init() - no native window (SetWindow not called)" << endl;
		return false;
	}

	if(! createInstance() )			return false;
	if(! createSurface() )			return false;
	if(! pickPhysicalDevice() )		return false;
	if(! createDevice() )			return false;
	if(! createSwapchain( VK_NULL_HANDLE ) )	return false;
	if(! createImageViews() )		return false;
	if(! createRenderPass() )		return false;
	if(! createDepthResources() )		return false;
	if(! createFramebuffers() )		return false;
	if(! createCommandPoolAndBuffers() )	return false;
	if(! createSyncObjects() )		return false;

	// V2: UBO ring (persistently mapped), descriptor pool, dummy texture
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties( m_physDevice, &props );
		m_uboAlignment = props.limits.minUniformBufferOffsetAlignment;
		if( m_uboAlignment < 16 ) m_uboAlignment = 16;

		VkBufferCreateInfo bi{};
		bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bi.size = UBO_RING_SIZE;
		bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if( vkCreateBuffer( m_device, &bi, nullptr, &m_uboBuffer ) != VK_SUCCESS )
			return false;
		VkMemoryRequirements reqs;
		vkGetBufferMemoryRequirements( m_device, m_uboBuffer, &reqs );
		VkMemoryAllocateInfo ai{};
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = reqs.size;
		ai.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		if( vkAllocateMemory( m_device, &ai, nullptr, &m_uboMemory ) != VK_SUCCESS )
			return false;
		vkBindBufferMemory( m_device, m_uboBuffer, m_uboMemory, 0 );
		vkMapMemory( m_device, m_uboMemory, 0, UBO_RING_SIZE, 0, &m_uboMapped );

		VkDescriptorPoolSize sizes[2]{};
		sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		sizes[0].descriptorCount = 64;
		sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sizes[1].descriptorCount = 256;
		VkDescriptorPoolCreateInfo pi{};
		pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pi.maxSets = 128;
		pi.poolSizeCount = 2;
		pi.pPoolSizes = sizes;
		if( vkCreateDescriptorPool( m_device, &pi, nullptr, &m_descriptorPool ) != VK_SUCCESS )
			return false;

		if(! createDummyTexture() ) return false;
	}

	m_initialized = true;

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties( m_physDevice, &props );
	cout << "INFO: lsVulkan - device: " << props.deviceName
		 << " (swapchain " << m_swapchainExtent.width << "x" << m_swapchainExtent.height
		 << ", " << m_swapchainImages.size() << " images)" << endl;
	return true;
}

// ---------------------------------------------------------------------
// Init helpers
// ---------------------------------------------------------------------
bool lsVulkan::createInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "vortxGE";
	appInfo.applicationVersion = VK_MAKE_VERSION( 0, 2, 0 );
	appInfo.pEngineName = "vortxGE";
	appInfo.apiVersion = VK_API_VERSION_1_2;

	// GLFW tells us which surface extensions the platform needs
	uint32_t glfwExtCount = 0;
	const char** glfwExts = glfwGetRequiredInstanceExtensions( &glfwExtCount );
	if(! glfwExts )
	{
		cout << "ERROR: lsVulkan - glfwGetRequiredInstanceExtensions failed" << endl;
		return false;
	}
	std::vector<const char*> exts( glfwExts, glfwExts + glfwExtCount );

	std::vector<const char*> layers;
	if( kValidation && validationLayerAvailable() )
	{
		layers.push_back( kValidationLayer );
		exts.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
	}

	VkInstanceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &appInfo;
	ci.enabledExtensionCount = (uint32_t)exts.size();
	ci.ppEnabledExtensionNames = exts.data();
	ci.enabledLayerCount = (uint32_t)layers.size();
	ci.ppEnabledLayerNames = layers.data();

	m_lastError = vkCreateInstance( &ci, nullptr, &m_instance );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - vkCreateInstance failed (" << m_lastError << ")" << endl;
		return false;
	}

	// Debug messenger (only when the layer is actually enabled)
	if(! layers.empty() )
	{
		VkDebugUtilsMessengerCreateInfoEXT di{};
		di.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		di.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		di.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		di.pfnUserCallback = debugCallback;

		auto createFn = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr( m_instance, "vkCreateDebugUtilsMessengerEXT" );
		if( createFn )
			createFn( m_instance, &di, nullptr, &m_debugMessenger );
	}
	return true;
}

bool lsVulkan::createSurface()
{
	m_lastError = glfwCreateWindowSurface( m_instance, (GLFWwindow*)m_window,
										   nullptr, &m_surface );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - glfwCreateWindowSurface failed (" << m_lastError << ")" << endl;
		return false;
	}
	return true;
}

bool lsVulkan::pickPhysicalDevice()
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices( m_instance, &count, nullptr );
	if( count == 0 )
	{
		cout << "ERROR: lsVulkan - no physical devices" << endl;
		return false;
	}
	std::vector<VkPhysicalDevice> devices( count );
	vkEnumeratePhysicalDevices( m_instance, &count, devices.data() );

	for( auto dev : devices )
	{
		// queue families: need graphics + present (may be the same family)
		uint32_t qcount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( dev, &qcount, nullptr );
		std::vector<VkQueueFamilyProperties> qprops( qcount );
		vkGetPhysicalDeviceQueueFamilyProperties( dev, &qcount, qprops.data() );

		int graphics = -1, present = -1;
		for( uint32_t i = 0; i < qcount; i++ )
		{
			if( qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
				if( graphics < 0 ) graphics = (int)i;
			VkBool32 canPresent = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR( dev, i, m_surface, &canPresent );
			if( canPresent && present < 0 ) present = (int)i;
		}
		if( graphics < 0 || present < 0 ) continue;

		// swapchain extension required
		uint32_t ecount = 0;
		vkEnumerateDeviceExtensionProperties( dev, nullptr, &ecount, nullptr );
		std::vector<VkExtensionProperties> eprops( ecount );
		vkEnumerateDeviceExtensionProperties( dev, nullptr, &ecount, eprops.data() );
		bool hasSwapchain = false;
		for( const auto& e : eprops )
			if( strcmp( e.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) == 0 )
				hasSwapchain = true;
		if(! hasSwapchain ) continue;

		// swapchain must have at least one format + present mode
		uint32_t fcount = 0, pcount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR( dev, m_surface, &fcount, nullptr );
		vkGetPhysicalDeviceSurfacePresentModesKHR( dev, m_surface, &pcount, nullptr );
		if( fcount == 0 || pcount == 0 ) continue;

		m_physDevice = dev;
		m_graphicsQueueFamily = (uint32_t)graphics;
		m_presentQueueFamily = (uint32_t)present;
		return true;
	}

	cout << "ERROR: lsVulkan - no suitable physical device (graphics+present+swapchain)" << endl;
	return false;
}

bool lsVulkan::createDevice()
{
	std::set<uint32_t> families = { m_graphicsQueueFamily, m_presentQueueFamily };
	float priority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> qcis;
	for( uint32_t fam : families )
	{
		VkDeviceQueueCreateInfo qci{};
		qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qci.queueFamilyIndex = fam;
		qci.queueCount = 1;
		qci.pQueuePriorities = &priority;
		qcis.push_back( qci );
	}

	const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.queueCreateInfoCount = (uint32_t)qcis.size();
	ci.pQueueCreateInfos = qcis.data();
	ci.enabledExtensionCount = 1;
	ci.ppEnabledExtensionNames = devExts;

	m_lastError = vkCreateDevice( m_physDevice, &ci, nullptr, &m_device );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - vkCreateDevice failed (" << m_lastError << ")" << endl;
		return false;
	}

	vkGetDeviceQueue( m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue );
	vkGetDeviceQueue( m_device, m_presentQueueFamily, 0, &m_presentQueue );
	return true;
}

bool lsVulkan::createSwapchain( VkSwapchainKHR oldSwapchain )
{
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_physDevice, m_surface, &caps );

	// format: prefer B8G8R8A8_SRGB + nonlinear color space
	uint32_t fcount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( m_physDevice, m_surface, &fcount, nullptr );
	std::vector<VkSurfaceFormatKHR> formats( fcount );
	vkGetPhysicalDeviceSurfaceFormatsKHR( m_physDevice, m_surface, &fcount, formats.data() );

	VkSurfaceFormatKHR chosen = formats[0];
	for( const auto& f : formats )
	{
		if( f.format == VK_FORMAT_B8G8R8A8_SRGB &&
			f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
		{
			chosen = f;
			break;
		}
	}
	m_swapchainFormat = chosen.format;

	// extent: caps.currentExtent or the window framebuffer size
	if( caps.currentExtent.width != std::numeric_limits<uint32_t>::max() )
	{
		m_swapchainExtent = caps.currentExtent;
	} else
	{
		int w = 0, h = 0;
		glfwGetFramebufferSize( (GLFWwindow*)m_window, &w, &h );
		m_swapchainExtent.width = std::clamp( (uint32_t)w, caps.minImageExtent.width,
											  caps.maxImageExtent.width );
		m_swapchainExtent.height = std::clamp( (uint32_t)h, caps.minImageExtent.height,
											   caps.maxImageExtent.height );
	}

	uint32_t imageCount = caps.minImageCount + 1;
	if( caps.maxImageCount > 0 && imageCount > caps.maxImageCount )
		imageCount = caps.maxImageCount;

	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = m_surface;
	ci.minImageCount = imageCount;
	ci.imageFormat = m_swapchainFormat;
	ci.imageColorSpace = chosen.colorSpace;
	ci.imageExtent = m_swapchainExtent;
	ci.imageArrayLayers = 1;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t families[] = { m_graphicsQueueFamily, m_presentQueueFamily };
	if( m_graphicsQueueFamily != m_presentQueueFamily )
	{
		ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		ci.queueFamilyIndexCount = 2;
		ci.pQueueFamilyIndices = families;
	} else
	{
		ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	ci.preTransform = caps.currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;		// vsync, always available
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = oldSwapchain;

	m_lastError = vkCreateSwapchainKHR( m_device, &ci, nullptr, &m_swapchain );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - vkCreateSwapchainKHR failed (" << m_lastError << ")" << endl;
		return false;
	}

	uint32_t realCount = 0;
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &realCount, nullptr );
	m_swapchainImages.resize( realCount );
	vkGetSwapchainImagesKHR( m_device, m_swapchain, &realCount, m_swapchainImages.data() );
	return true;
}

bool lsVulkan::createImageViews()
{
	m_swapchainImageViews.resize( m_swapchainImages.size() );
	for( size_t i = 0; i < m_swapchainImages.size(); i++ )
	{
		VkImageViewCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.image = m_swapchainImages[i];
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.format = m_swapchainFormat;
		ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
						  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
		ci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		m_lastError = vkCreateImageView( m_device, &ci, nullptr, &m_swapchainImageViews[i] );
		if( m_lastError != VK_SUCCESS )
		{
			cout << "ERROR: lsVulkan - vkCreateImageView failed (" << m_lastError << ")" << endl;
			return false;
		}
	}
	return true;
}

bool lsVulkan::createRenderPass()
{
	// Single color attachment, cleared on load, presented at the end.
	// Depth attachment arrives with the FBO/shadow milestones (V4/V5).
	VkAttachmentDescription color{};
	color.format = m_swapchainFormat;
	color.samples = VK_SAMPLE_COUNT_1_BIT;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// depth attachment (V2)
	VkAttachmentDescription depth{};
	depth.format = m_depthFormat;
	depth.samples = VK_SAMPLE_COUNT_1_BIT;
	depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorRef{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthRef{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = &depthRef;

	VkSubpassDependency dep{};
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;
	dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.srcAccessMask = 0;
	dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	VkAttachmentDescription attachments[2] = { color, depth };
	ci.attachmentCount = 2;
	ci.pAttachments = attachments;
	ci.subpassCount = 1;
	ci.pSubpasses = &subpass;
	ci.dependencyCount = 1;
	ci.pDependencies = &dep;

	m_lastError = vkCreateRenderPass( m_device, &ci, nullptr, &m_renderPass );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - vkCreateRenderPass failed (" << m_lastError << ")" << endl;
		return false;
	}
	return true;
}

bool lsVulkan::createFramebuffers()
{
	m_framebuffers.resize( m_swapchainImageViews.size() );
	for( size_t i = 0; i < m_swapchainImageViews.size(); i++ )
	{
		VkImageView attachments[] = { m_swapchainImageViews[i], m_depthView };

		VkFramebufferCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ci.renderPass = m_renderPass;
		ci.attachmentCount = 2;
		ci.pAttachments = attachments;
		ci.width = m_swapchainExtent.width;
		ci.height = m_swapchainExtent.height;
		ci.layers = 1;

		m_lastError = vkCreateFramebuffer( m_device, &ci, nullptr, &m_framebuffers[i] );
		if( m_lastError != VK_SUCCESS )
		{
			cout << "ERROR: lsVulkan - vkCreateFramebuffer failed (" << m_lastError << ")" << endl;
			return false;
		}
	}
	return true;
}

bool lsVulkan::createCommandPoolAndBuffers()
{
	VkCommandPoolCreateInfo pi{};
	pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pi.queueFamilyIndex = m_graphicsQueueFamily;

	m_lastError = vkCreateCommandPool( m_device, &pi, nullptr, &m_commandPool );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - vkCreateCommandPool failed (" << m_lastError << ")" << endl;
		return false;
	}

	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = m_commandPool;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = FRAMES_IN_FLIGHT;

	m_lastError = vkAllocateCommandBuffers( m_device, &ai, m_commandBuffers );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - vkAllocateCommandBuffers failed (" << m_lastError << ")" << endl;
		return false;
	}
	return true;
}

bool lsVulkan::createSyncObjects()
{
	VkSemaphoreCreateInfo si{};
	si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fi{};
	fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;	// first frame must not block

	for( uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++ )
	{
		if( vkCreateSemaphore( m_device, &si, nullptr, &m_imageAvailable[i] ) != VK_SUCCESS ||
			vkCreateSemaphore( m_device, &si, nullptr, &m_renderFinished[i] ) != VK_SUCCESS ||
			vkCreateFence( m_device, &fi, nullptr, &m_inFlight[i] ) != VK_SUCCESS )
		{
			cout << "ERROR: lsVulkan - sync object creation failed" << endl;
			return false;
		}
	}
	return true;
}

// ---------------------------------------------------------------------
// Swapchain recreation (resize / out-of-date)
// ---------------------------------------------------------------------
void lsVulkan::cleanupSwapchain()
{
	for( auto fb : m_framebuffers )
		vkDestroyFramebuffer( m_device, fb, nullptr );
	m_framebuffers.clear();

	for( auto v : m_swapchainImageViews )
		vkDestroyImageView( m_device, v, nullptr );
	m_swapchainImageViews.clear();

	if( m_swapchain )
	{
		vkDestroySwapchainKHR( m_device, m_swapchain, nullptr );
		m_swapchain = VK_NULL_HANDLE;
	}

	destroyDepthResources();	// V2
}

bool lsVulkan::recreateSwapchain()
{
	vkDeviceWaitIdle( m_device );

	// Window minimized (0-size): nothing to do until it comes back
	int w = 0, h = 0;
	glfwGetFramebufferSize( (GLFWwindow*)m_window, &w, &h );
	if( w == 0 || h == 0 ) return true;

	VkSwapchainKHR oldSwapchain = m_swapchain;

	for( auto fb : m_framebuffers ) vkDestroyFramebuffer( m_device, fb, nullptr );
	m_framebuffers.clear();
	for( auto v : m_swapchainImageViews ) vkDestroyImageView( m_device, v, nullptr );
	m_swapchainImageViews.clear();

	if(! createSwapchain( oldSwapchain ) ) return false;
	if( oldSwapchain ) vkDestroySwapchainKHR( m_device, oldSwapchain, nullptr );

	if(! createImageViews() ) return false;
	if(! createDepthResources() ) return false;	// V2
	if(! createFramebuffers() ) return false;

	m_swapchainDirty = false;
	cout << "INFO: lsVulkan - swapchain recreated (" << m_swapchainExtent.width
		 << "x" << m_swapchainExtent.height << ")" << endl;
	return true;
}

// ---------------------------------------------------------------------
// Frame lifecycle
// ---------------------------------------------------------------------
void lsVulkan::BeginFrame()
{
	if(! m_initialized || m_inFrame ) return;

	if( m_swapchainDirty )
		if(! recreateSwapchain() ) return;

	vkWaitForFences( m_device, 1, &m_inFlight[m_currentFrame], VK_TRUE, UINT64_MAX );

	m_lastError = vkAcquireNextImageKHR( m_device, m_swapchain, UINT64_MAX,
										 m_imageAvailable[m_currentFrame],
										 VK_NULL_HANDLE, &m_acquiredImage );
	if( m_lastError == VK_ERROR_OUT_OF_DATE_KHR )
	{
		m_swapchainDirty = true;
		return;		// skip this frame; recreate happens next BeginFrame
	}

	vkResetFences( m_device, 1, &m_inFlight[m_currentFrame] );

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
	vkResetCommandBuffer( cmd, 0 );

	VkCommandBufferBeginInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer( cmd, &bi );

	VkClearValue clearValues[2];
	clearValues[0].color.float32[0] = m_clearColor[0];
	clearValues[0].color.float32[1] = m_clearColor[1];
	clearValues[0].color.float32[2] = m_clearColor[2];
	clearValues[0].color.float32[3] = m_clearColor[3];
	clearValues[1].depthStencil.depth = (float)m_clearDepth;
	clearValues[1].depthStencil.stencil = 0;

	m_uboCursor = 0;	// V2: reset the per-frame uniform ring

	VkRenderPassBeginInfo rp{};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp.renderPass = m_renderPass;
	rp.framebuffer = m_framebuffers[m_acquiredImage];
	rp.renderArea.offset = { 0, 0 };
	rp.renderArea.extent = m_swapchainExtent;
	rp.clearValueCount = 2;
	rp.pClearValues = clearValues;

	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );

	// Dynamic viewport/scissor (used by draws in V2+; harmless now).
	// NOTE: Vulkan NDC is Y-down vs GL — V2 flips via negative height.
	VkViewport vp{};
	vp.x = 0.0f;
	vp.y = (float)m_swapchainExtent.height;		// Y-flip for GL conventions
	vp.width = (float)m_swapchainExtent.width;
	vp.height = -(float)m_swapchainExtent.height;
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vkCmdSetViewport( cmd, 0, 1, &vp );

	VkRect2D sc{};
	sc.offset = { 0, 0 };
	sc.extent = m_swapchainExtent;
	vkCmdSetScissor( cmd, 0, 1, &sc );

	m_inFrame = true;
}

void lsVulkan::EndFrame()
{
	if(! m_initialized || ! m_inFrame ) return;

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];

	vkCmdEndRenderPass( cmd );
	vkEndCommandBuffer( cmd );

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &m_imageAvailable[m_currentFrame];
	submit.pWaitDstStageMask = &waitStage;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &m_renderFinished[m_currentFrame];

	m_lastError = vkQueueSubmit( m_graphicsQueue, 1, &submit, m_inFlight[m_currentFrame] );
	if( m_lastError != VK_SUCCESS )
		cout << "ERROR: lsVulkan - vkQueueSubmit failed (" << m_lastError << ")" << endl;
}

void lsVulkan::Present()
{
	if(! m_initialized || ! m_inFrame ) return;

	VkPresentInfoKHR pi{};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores = &m_renderFinished[m_currentFrame];
	pi.swapchainCount = 1;
	pi.pSwapchains = &m_swapchain;
	pi.pImageIndices = &m_acquiredImage;

	m_lastError = vkQueuePresentKHR( m_presentQueue, &pi );
	if( m_lastError == VK_ERROR_OUT_OF_DATE_KHR ||
		m_lastError == VK_SUBOPTIMAL_KHR )
		m_swapchainDirty = true;

	m_inFrame = false;
	m_currentFrame = ( m_currentFrame + 1 ) % FRAMES_IN_FLIGHT;
}

void lsVulkan::Resize( int width, int height )
{
	(void)width; (void)height;	// extent re-queried from the window at recreate
	m_swapchainDirty = true;
}

// ---------------------------------------------------------------------
// Clear / state
// ---------------------------------------------------------------------
void lsVulkan::ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a )
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

void lsVulkan::ClearDepth( lsDouble d )
{
	m_clearDepth = d;	// used once depth attachments land (V4/V5)
}

void lsVulkan::Clear( lsBitfield mask )
{
	// The render pass already clears on load with the stored clear color,
	// so Clear() only needs to act for explicit mid-frame clears.
	if(! m_inFrame ) return;

	if( mask & LS_COLOR_BUFFER_BIT )
	{
		VkClearValue value;
		value.color.float32[0] = m_clearColor[0];
		value.color.float32[1] = m_clearColor[1];
		value.color.float32[2] = m_clearColor[2];
		value.color.float32[3] = m_clearColor[3];

		VkClearAttachment att{};
		att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		att.colorAttachment = 0;
		att.clearValue = value;

		VkClearRect rect{};
		rect.rect.offset = { 0, 0 };
		rect.rect.extent = m_swapchainExtent;
		rect.baseArrayLayer = 0;
		rect.layerCount = 1;

		vkCmdClearAttachments( m_commandBuffers[m_currentFrame], 1, &att, 1, &rect );
	}
	// LS_DEPTH_BUFFER_BIT: no depth attachment in the V1 render pass.
}

void lsVulkan::Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h )
{
	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = w;
	m_viewport[3] = h;
	// Applied to draw state in V2 (no pipelines yet in V1).
}

void lsVulkan::StoreDefaultFramebuffer()
{
	// No-op: the swapchain IS the default target. FBO redirection
	// (Store/Restore around offscreen passes) arrives in V4.
}

void lsVulkan::RestoreDefaultFramebuffer()
{
	// No-op (see StoreDefaultFramebuffer).
}

// ---------------------------------------------------------------------
// Diagnostics
// ---------------------------------------------------------------------
void lsVulkan::CheckErrors( const string& name )
{
	if( m_lastError != VK_SUCCESS )
	{
		cout << "VULKAN ERROR at " << name << ": VkResult " << m_lastError << endl;
		m_lastError = VK_SUCCESS;
	}
}

lsEnum lsVulkan::GetError()
{
	lsEnum err = (lsEnum)m_lastError;
	m_lastError = VK_SUCCESS;
	return err;
}


// =====================================================================
// V2 helpers: depth attachment, dummy texture, descriptor pool, UBO ring
// =====================================================================

bool lsVulkan::createDepthResources()
{
	VkImageCreateInfo ii{};
	ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = m_depthFormat;
	ii.extent = { m_swapchainExtent.width, m_swapchainExtent.height, 1 };
	ii.mipLevels = 1;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	m_lastError = vkCreateImage( m_device, &ii, nullptr, &m_depthImage );
	if( m_lastError != VK_SUCCESS ) return false;

	VkMemoryRequirements reqs;
	vkGetImageMemoryRequirements( m_device, m_depthImage, &reqs );
	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = reqs.size;
	ai.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits,
										 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
	m_lastError = vkAllocateMemory( m_device, &ai, nullptr, &m_depthMemory );
	if( m_lastError != VK_SUCCESS ) return false;
	vkBindImageMemory( m_device, m_depthImage, m_depthMemory, 0 );

	VkImageViewCreateInfo vi{};
	vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vi.image = m_depthImage;
	vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vi.format = m_depthFormat;
	vi.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
	m_lastError = vkCreateImageView( m_device, &vi, nullptr, &m_depthView );
	return m_lastError == VK_SUCCESS;
}

void lsVulkan::destroyDepthResources()
{
	if( m_depthView ) vkDestroyImageView( m_device, m_depthView, nullptr );
	if( m_depthImage ) vkDestroyImage( m_device, m_depthImage, nullptr );
	if( m_depthMemory ) vkFreeMemory( m_device, m_depthMemory, nullptr );
	m_depthView = VK_NULL_HANDLE;
	m_depthImage = VK_NULL_HANDLE;
	m_depthMemory = VK_NULL_HANDLE;
}

bool lsVulkan::createDummyTexture()
{
	// staging buffer with one white RGBA pixel
	const uint8_t pixel[4] = { 255, 255, 255, 255 };

	VkBufferCreateInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = 4;
	bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkBuffer staging;
	if( vkCreateBuffer( m_device, &bi, nullptr, &staging ) != VK_SUCCESS )
		return false;
	VkMemoryRequirements sreq;
	vkGetBufferMemoryRequirements( m_device, staging, &sreq );
	VkMemoryAllocateInfo sai{};
	sai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	sai.allocationSize = sreq.size;
	sai.memoryTypeIndex = findMemoryType( sreq.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	VkDeviceMemory stagingMem;
	if( vkAllocateMemory( m_device, &sai, nullptr, &stagingMem ) != VK_SUCCESS )
		return false;
	vkBindBufferMemory( m_device, staging, stagingMem, 0 );
	void* mapped = nullptr;
	vkMapMemory( m_device, stagingMem, 0, 4, 0, &mapped );
	memcpy( mapped, pixel, 4 );
	vkUnmapMemory( m_device, stagingMem );

	// optimal-tiling 1x1 image
	VkImageCreateInfo ii{};
	ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = VK_FORMAT_R8G8B8A8_UNORM;
	ii.extent = { 1, 1, 1 };
	ii.mipLevels = 1;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if( vkCreateImage( m_device, &ii, nullptr, &m_dummyImage ) != VK_SUCCESS )
		return false;
	VkMemoryRequirements ireq;
	vkGetImageMemoryRequirements( m_device, m_dummyImage, &ireq );
	VkMemoryAllocateInfo iai{};
	iai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	iai.allocationSize = ireq.size;
	iai.memoryTypeIndex = findMemoryType( ireq.memoryTypeBits,
										  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
	if( vkAllocateMemory( m_device, &iai, nullptr, &m_dummyMemory ) != VK_SUCCESS )
		return false;
	vkBindImageMemory( m_device, m_dummyImage, m_dummyMemory, 0 );

	// one-shot command buffer: transition + copy + transition
	VkCommandBufferAllocateInfo cai{};
	cai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cai.commandPool = m_commandPool;
	cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cai.commandBufferCount = 1;
	VkCommandBuffer cmd;
	vkAllocateCommandBuffers( m_device, &cai, &cmd );
	VkCommandBufferBeginInfo cbi{};
	cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( cmd, &cbi );

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_dummyImage;
	barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
						  0, nullptr, 1, &barrier );

	VkBufferImageCopy region{};
	region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	region.imageExtent = { 1, 1, 1 };
	vkCmdCopyBufferToImage( cmd, staging, m_dummyImage,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
						  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
						  0, nullptr, 1, &barrier );

	vkEndCommandBuffer( cmd );
	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	vkQueueSubmit( m_graphicsQueue, 1, &submit, VK_NULL_HANDLE );
	vkQueueWaitIdle( m_graphicsQueue );
	vkFreeCommandBuffers( m_device, m_commandPool, 1, &cmd );
	vkDestroyBuffer( m_device, staging, nullptr );
	vkFreeMemory( m_device, stagingMem, nullptr );

	// view + sampler
	VkImageViewCreateInfo vi{};
	vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vi.image = m_dummyImage;
	vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vi.format = VK_FORMAT_R8G8B8A8_UNORM;
	vi.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	if( vkCreateImageView( m_device, &vi, nullptr, &m_dummyView ) != VK_SUCCESS )
		return false;

	VkSamplerCreateInfo si{};
	si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	si.magFilter = VK_FILTER_LINEAR;
	si.minFilter = VK_FILTER_LINEAR;
	si.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	si.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	si.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	if( vkCreateSampler( m_device, &si, nullptr, &m_dummySampler ) != VK_SUCCESS )
		return false;
	return true;
}

// =====================================================================
// V2: buffers, VAOs, shaders, programs, uniforms, pipelines, draws
// =====================================================================

uint32_t lsVulkan::findMemoryType( uint32_t typeBits, VkMemoryPropertyFlags props )
{
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties( m_physDevice, &memProps );
	for( uint32_t i = 0; i < memProps.memoryTypeCount; i++ )
		if(( typeBits & ( 1u << i ) ) &&
		   ( memProps.memoryTypes[i].propertyFlags & props ) == props )
			return i;
	cout << "ERROR: lsVulkan - no suitable memory type" << endl;
	return 0;
}

bool lsVulkan::createBufferStorage( VkBuf& buf, VkDeviceSize size, const void* data )
{
	// Recreate semantics: BufferData on an existing GL buffer replaces it
	if( buf.buffer ) vkDestroyBuffer( m_device, buf.buffer, nullptr );
	if( buf.memory ) vkFreeMemory( m_device, buf.memory, nullptr );
	buf.buffer = VK_NULL_HANDLE;
	buf.memory = VK_NULL_HANDLE;
	buf.size = size;

	VkBufferCreateInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	m_lastError = vkCreateBuffer( m_device, &bi, nullptr, &buf.buffer );
	if( m_lastError != VK_SUCCESS ) return false;

	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements( m_device, buf.buffer, &reqs );

	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = reqs.size;
	ai.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

	m_lastError = vkAllocateMemory( m_device, &ai, nullptr, &buf.memory );
	if( m_lastError != VK_SUCCESS ) return false;
	vkBindBufferMemory( m_device, buf.buffer, buf.memory, 0 );

	if( data )
	{
		void* mapped = nullptr;
		vkMapMemory( m_device, buf.memory, 0, size, 0, &mapped );
		memcpy( mapped, data, size );
		vkUnmapMemory( m_device, buf.memory );
	}
	return true;
}

// ---------------------------------------------------------------------
// Buffer objects
// ---------------------------------------------------------------------
void lsVulkan::GenBuffers( lsSizei n, lsBufferID* buffers )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		uint32_t id = m_nextBufferID++;
		m_buffers[id] = VkBuf{};
		buffers[i] = id;
	}
}

void lsVulkan::DeleteBuffers( lsSizei n, lsBufferID* buffers )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		auto it = m_buffers.find( buffers[i] );
		if( it != m_buffers.end() )
		{
			if( it->second.buffer ) vkDestroyBuffer( m_device, it->second.buffer, nullptr );
			if( it->second.memory ) vkFreeMemory( m_device, it->second.memory, nullptr );
			m_buffers.erase( it );
		}
	}
}

void lsVulkan::BindBuffer( lsEnum target, lsBufferID buffer )
{
	if( target == LS_ARRAY_BUFFER )
	{
		m_boundArrayBuffer = buffer;
	} else if( target == LS_ELEMENT_ARRAY_BUFFER )
	{
		m_boundElementBuffer = buffer;
		// GL: the element binding is VAO state
		if( m_currentVao ) m_vaos[m_currentVao].elementBuffer = buffer;
	}
}

void lsVulkan::BufferData( lsEnum target, lsSizeiptr size, const void* data, lsEnum usage )
{
	(void)usage;	// host-visible coherent buffers: usage hint not needed
	uint32_t id = ( target == LS_ELEMENT_ARRAY_BUFFER ) ? m_boundElementBuffer
														: m_boundArrayBuffer;
	auto it = m_buffers.find( id );
	if( it == m_buffers.end() ) return;
	if(! createBufferStorage( it->second, (VkDeviceSize)size, data ) )
		cout << "ERROR: lsVulkan::BufferData failed (VkResult " << m_lastError << ")" << endl;
}

// ---------------------------------------------------------------------
// Vertex array objects (attribute layout capture)
// ---------------------------------------------------------------------
void lsVulkan::GenVertexArrays( lsSizei n, lsVAOID* arrays )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		uint32_t id = m_nextVaoID++;
		m_vaos[id] = VkVao{};
		arrays[i] = id;
	}
}

void lsVulkan::DeleteVertexArrays( lsSizei n, lsVAOID* arrays )
{
	for( lsSizei i = 0; i < n; i++ )
		m_vaos.erase( arrays[i] );
}

void lsVulkan::BindVertexArray( lsVAOID vao )
{
	m_currentVao = vao;
	if( vao )
		m_boundElementBuffer = m_vaos[vao].elementBuffer;
}

void lsVulkan::EnableVertexAttribArray( lsUInt index )
{
	if( m_currentVao )
		m_vaos[m_currentVao].attribs[index].enabled = true;
}

void lsVulkan::VertexAttribPointer( lsUInt index, lsInt size, lsEnum type,
									lsBoolean normalized, lsSizei stride,
									const void* pointer )
{
	if(! m_currentVao ) return;
	VkVaoAttrib& a = m_vaos[m_currentVao].attribs[index];
	a.buffer = m_boundArrayBuffer;		// GL: captured from current ARRAY_BUFFER
	a.size = size;
	a.type = type;
	a.normalized = normalized;
	a.stride = stride;
	a.offset = (size_t)pointer;
}

// ---------------------------------------------------------------------
// Shader objects (parse now; SPIR-V compile deferred to LinkProgram so
// both stages share one canonical uniform-block layout)
// ---------------------------------------------------------------------
lsShaderID lsVulkan::CreateShader( lsEnum type )
{
	uint32_t id = m_nextShaderID++;
	VkShaderObj obj;
	obj.stage = ( type == LS_VERTEX_SHADER ) ? 0 : 1;
	m_shaders[id] = obj;
	return id;
}

void lsVulkan::ShaderSource( lsShaderID shader, lsSizei count,
							 const lsChar** string, const lsInt* length )
{
	auto it = m_shaders.find( shader );
	if( it == m_shaders.end() ) return;
	std::string src;
	for( lsSizei i = 0; i < count; i++ )
	{
		if( length && length[i] > 0 )
			src.append( string[i], length[i] );
		else
			src.append( string[i] );
	}
	it->second.source = src;
}

void lsVulkan::CompileShader( lsShaderID shader )
{
	auto it = m_shaders.find( shader );
	if( it == m_shaders.end() ) return;
	it->second.parsed = lsVkShader::parse( it->second.source );
	it->second.hasSource = true;
}

void lsVulkan::GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params )
{
	auto it = m_shaders.find( shader );
	if( it == m_shaders.end() || ! params ) return;
	if( pname == LS_COMPILE_STATUS )
		params[0] = it->second.hasSource ? 1 : 0;
	else if( pname == LS_INFO_LOG_LENGTH )
		params[0] = 0;
}

void lsVulkan::GetShaderInfoLog( lsShaderID shader, lsSizei bufSize,
								 lsSizei* length, lsChar* infoLog )
{
	(void)shader;
	if( length ) *length = 0;
	if( infoLog && bufSize > 0 ) infoLog[0] = '\0';
}

void lsVulkan::DeleteShader( lsShaderID shader )
{
	m_shaders.erase( shader );
}

// ---------------------------------------------------------------------
// Program objects
// ---------------------------------------------------------------------
lsProgramID lsVulkan::CreateProgram()
{
	uint32_t id = m_nextProgramID++;
	m_programs[id] = VkProgramObj{};
	return id;
}

void lsVulkan::AttachShader( lsProgramID program, lsShaderID shader )
{
	auto pit = m_programs.find( program );
	auto sit = m_shaders.find( shader );
	if( pit == m_programs.end() || sit == m_shaders.end() ) return;
	if( sit->second.stage == 0 ) pit->second.vs = shader;
	else						 pit->second.fs = shader;
}

void lsVulkan::DetachShader( lsProgramID program, lsShaderID shader )
{
	auto pit = m_programs.find( program );
	if( pit == m_programs.end() ) return;
	if( pit->second.vs == shader ) pit->second.vs = 0;
	if( pit->second.fs == shader ) pit->second.fs = 0;
}

bool lsVulkan::linkProgramResources( VkProgramObj& prog )
{
	auto vit = m_shaders.find( prog.vs );
	auto fit = m_shaders.find( prog.fs );
	if( vit == m_shaders.end() || fit == m_shaders.end() ||
		! vit->second.hasSource || ! fit->second.hasSource )
	{
		prog.log = "missing vertex or fragment shader";
		return false;
	}

	// canonical merged layout -> identical block in both stages
	prog.layout = lsVkShader::mergeLayouts( vit->second.parsed, fit->second.parsed );

	const std::string vsSrc = lsVkShader::assemble( vit->second.parsed, prog.layout );
	const std::string fsSrc = lsVkShader::assemble( fit->second.parsed, prog.layout );

	std::string log;
	const auto vsSpirv = lsVkShader::compile( vsSrc, 0, log );
	if( vsSpirv.empty() ) { prog.log = "VS: " + log; return false; }
	const auto fsSpirv = lsVkShader::compile( fsSrc, 1, log );
	if( fsSpirv.empty() ) { prog.log = "FS: " + log; return false; }

	VkShaderModuleCreateInfo mi{};
	mi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	mi.codeSize = vsSpirv.size() * sizeof( uint32_t );
	mi.pCode = vsSpirv.data();
	if( vkCreateShaderModule( m_device, &mi, nullptr, &prog.vsModule ) != VK_SUCCESS )
	{ prog.log = "vkCreateShaderModule (VS) failed"; return false; }
	mi.codeSize = fsSpirv.size() * sizeof( uint32_t );
	mi.pCode = fsSpirv.data();
	if( vkCreateShaderModule( m_device, &mi, nullptr, &prog.fsModule ) != VK_SUCCESS )
	{ prog.log = "vkCreateShaderModule (FS) failed"; return false; }

	// lookup tables
	for( uint32_t i = 0; i < prog.layout.members.size(); i++ )
		prog.uniformIndex[prog.layout.members[i].name] = i;
	for( uint32_t i = 0; i < prog.layout.samplers.size(); i++ )
		prog.samplerBinding[prog.layout.samplers[i]] = 1 + i;
	prog.uboStaging.assign( std::max<VkDeviceSize>( prog.layout.blockSize, 16 ), 0 );

	// descriptor set layout: 0 = dynamic UBO, 1..N = combined samplers
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	VkDescriptorSetLayoutBinding ubo{};
	ubo.binding = 0;
	ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	ubo.descriptorCount = 1;
	ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings.push_back( ubo );
	for( size_t i = 0; i < prog.layout.samplers.size(); i++ )
	{
		VkDescriptorSetLayoutBinding s{};
		s.binding = 1 + (uint32_t)i;
		s.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		s.descriptorCount = ( i < prog.layout.samplerArrays.size() &&
							  prog.layout.samplerArrays[i] > 0 )
			? prog.layout.samplerArrays[i] : 1;
		s.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back( s );
	}

	VkDescriptorSetLayoutCreateInfo li{};
	li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	li.bindingCount = (uint32_t)bindings.size();
	li.pBindings = bindings.data();
	if( vkCreateDescriptorSetLayout( m_device, &li, nullptr, &prog.setLayout ) != VK_SUCCESS )
	{ prog.log = "vkCreateDescriptorSetLayout failed"; return false; }

	VkPipelineLayoutCreateInfo pli{};
	pli.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pli.setLayoutCount = 1;
	pli.pSetLayouts = &prog.setLayout;
	if( vkCreatePipelineLayout( m_device, &pli, nullptr, &prog.pipelineLayout ) != VK_SUCCESS )
	{ prog.log = "vkCreatePipelineLayout failed"; return false; }

	// descriptor set + initial writes (UBO ring + dummy textures)
	VkDescriptorSetAllocateInfo dai{};
	dai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dai.descriptorPool = m_descriptorPool;
	dai.descriptorSetCount = 1;
	dai.pSetLayouts = &prog.setLayout;
	if( vkAllocateDescriptorSets( m_device, &dai, &prog.descriptorSet ) != VK_SUCCESS )
	{ prog.log = "vkAllocateDescriptorSets failed"; return false; }

	std::vector<VkWriteDescriptorSet> writes;
	VkDescriptorBufferInfo uboInfo{};
	uboInfo.buffer = m_uboBuffer;
	uboInfo.offset = 0;
	uboInfo.range = std::max<VkDeviceSize>( prog.layout.blockSize, 16 );
	VkWriteDescriptorSet uw{};
	uw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uw.dstSet = prog.descriptorSet;
	uw.dstBinding = 0;
	uw.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uw.descriptorCount = 1;
	uw.pBufferInfo = &uboInfo;
	writes.push_back( uw );

	for( size_t i = 0; i < prog.layout.samplers.size(); i++ )
	{
		const uint32_t count = ( i < prog.layout.samplerArrays.size() &&
								 prog.layout.samplerArrays[i] > 0 )
			? prog.layout.samplerArrays[i] : 1;
		// one info per array element (dummy texture everywhere in V2)
		std::vector<VkDescriptorImageInfo>* infos =
			new std::vector<VkDescriptorImageInfo>( count );
		for( auto& info : *infos )
		{
			info.sampler = m_dummySampler;
			info.imageView = m_dummyView;
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		m_descriptorImageInfos.push_back( infos );	// owned, freed in dtor
		VkWriteDescriptorSet sw{};
		sw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		sw.dstSet = prog.descriptorSet;
		sw.dstBinding = 1 + (uint32_t)i;
		sw.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sw.descriptorCount = count;
		sw.pImageInfo = infos->data();
		writes.push_back( sw );
	}
	vkUpdateDescriptorSets( m_device, (uint32_t)writes.size(), writes.data(), 0, nullptr );

	prog.linked = true;
	return true;
}

void lsVulkan::LinkProgram( lsProgramID program )
{
	auto it = m_programs.find( program );
	if( it == m_programs.end() ) return;
	it->second.linked = linkProgramResources( it->second );
	if(! it->second.linked )
		cout << "ERROR: lsVulkan::LinkProgram(" << program << "): "
			 << it->second.log << endl;
}

void lsVulkan::UseProgram( lsProgramID program )
{
	m_currentProgram = program;
}

void lsVulkan::DeleteProgram( lsProgramID program )
{
	auto it = m_programs.find( program );
	if( it == m_programs.end() ) return;
	VkProgramObj& p = it->second;
	if( p.vsModule ) vkDestroyShaderModule( m_device, p.vsModule, nullptr );
	if( p.fsModule ) vkDestroyShaderModule( m_device, p.fsModule, nullptr );
	if( p.setLayout ) vkDestroyDescriptorSetLayout( m_device, p.setLayout, nullptr );
	if( p.pipelineLayout ) vkDestroyPipelineLayout( m_device, p.pipelineLayout, nullptr );
	m_programs.erase( it );
}

void lsVulkan::GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params )
{
	auto it = m_programs.find( program );
	if( it == m_programs.end() || ! params ) return;
	if( pname == LS_LINK_STATUS )
		params[0] = it->second.linked ? 1 : 0;
	else if( pname == LS_INFO_LOG_LENGTH )
		params[0] = (lsInt)it->second.log.size();
}

void lsVulkan::GetProgramInfoLog( lsProgramID program, lsSizei bufSize,
								  lsSizei* length, lsChar* infoLog )
{
	auto it = m_programs.find( program );
	if( it == m_programs.end() || ! infoLog || bufSize <= 0 ) return;
	const std::string& log = it->second.log;
	const size_t n = std::min( (size_t)bufSize - 1, log.size() );
	memcpy( infoLog, log.data(), n );
	infoLog[n] = '\0';
	if( length ) *length = (lsSizei)n;
}

lsUniformID lsVulkan::GetUniformLocation( lsProgramID program, const lsChar* name )
{
	auto it = m_programs.find( program );
	if( it == m_programs.end() || ! name ) return -1;
	auto si = it->second.samplerBinding.find( name );
	if( si != it->second.samplerBinding.end() )
		return 1000 + (lsUniformID)si->second;		// sampler: 1000+binding
	auto mi = it->second.uniformIndex.find( name );
	if( mi != it->second.uniformIndex.end() )
		return (lsUniformID)mi->second;
	return -1;
}

// ---------------------------------------------------------------------
// Uniform setters (write into the program's CPU-side block image)
// ---------------------------------------------------------------------
void lsVulkan::writeUniformMember( VkProgramObj& prog, lsUniformID location,
								   const void* data, uint32_t bytes )
{
	if( location < 0 || location >= (lsUniformID)prog.layout.members.size() )
		return;
	const lsVkUniformMember& mem = prog.layout.members[location];
	if( mem.offset + bytes > prog.uboStaging.size() ) return;
	memcpy( prog.uboStaging.data() + mem.offset, data, bytes );
}

void lsVulkan::Uniform1i( lsUniformID location, lsInt v0 )
{
	auto it = m_programs.find( m_currentProgram );
	if( it == m_programs.end() ) return;
	if( location >= 1000 ) return;		// sampler unit assignment: V2 binds dummies
	writeUniformMember( it->second, location, &v0, sizeof( v0 ) );
}

void lsVulkan::Uniform1f( lsUniformID location, lsFloat v0 )
{
	auto it = m_programs.find( m_currentProgram );
	if( it == m_programs.end() ) return;
	writeUniformMember( it->second, location, &v0, sizeof( v0 ) );
}

void lsVulkan::Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 )
{
	auto it = m_programs.find( m_currentProgram );
	if( it == m_programs.end() ) return;
	const float v[3] = { v0, v1, v2 };
	writeUniformMember( it->second, location, v, sizeof( v ) );
}

void lsVulkan::Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1,
						  lsFloat v2, lsFloat v3 )
{
	auto it = m_programs.find( m_currentProgram );
	if( it == m_programs.end() ) return;
	const float v[4] = { v0, v1, v2, v3 };
	writeUniformMember( it->second, location, v, sizeof( v ) );
}

void lsVulkan::UniformMatrix3fv( lsUniformID location, lsSizei count,
								 lsBoolean transpose, const lsFloat* value )
{
	(void)transpose;	// engine never transposes
	auto it = m_programs.find( m_currentProgram );
	if( it == m_programs.end() || location < 0 ||
		location >= (lsUniformID)it->second.layout.members.size() ) return;
	const lsVkUniformMember& mem = it->second.layout.members[location];
	// std140 mat3: 3 columns at 16-byte stride (packed 9 floats in)
	for( lsSizei m = 0; m < count; m++ )
	{
		uint8_t* dst = it->second.uboStaging.data() + mem.offset + m * 48;
		const float* src = value + m * 9;
		for( int col = 0; col < 3; col++ )
			memcpy( dst + col * 16, src + col * 3, 12 );
	}
}

void lsVulkan::UniformMatrix4fv( lsUniformID location, lsSizei count,
								 lsBoolean transpose, const lsFloat* value )
{
	(void)transpose;
	auto it = m_programs.find( m_currentProgram );
	if( it == m_programs.end() ) return;
	writeUniformMember( it->second, location, value,
						(uint32_t)( count * 64 ) );
}

// ---------------------------------------------------------------------
// Pipeline / rasterizer state (cached, baked into pipelines at draw)
// ---------------------------------------------------------------------
void lsVulkan::Enable( lsEnum cap )
{
	if( cap == LS_DEPTH_TEST ) m_depthTestEnabled = true;
	else if( cap == LS_CULL_FACE ) m_cullEnabled = true;
	else if( cap == LS_BLEND ) m_blendEnabled = true;
}

void lsVulkan::Disable( lsEnum cap )
{
	if( cap == LS_DEPTH_TEST ) m_depthTestEnabled = false;
	else if( cap == LS_CULL_FACE ) m_cullEnabled = false;
	else if( cap == LS_BLEND ) m_blendEnabled = false;
}

void lsVulkan::CullFace( lsEnum mode )	{ m_cullFace = mode; }
void lsVulkan::BlendFunc( lsEnum sfactor, lsEnum dfactor )
{
	m_blendSrc = sfactor;
	m_blendDst = dfactor;
}
void lsVulkan::DepthFunc( lsEnum func )	{ m_depthFunc = func; }
void lsVulkan::DepthMask( lsBoolean flag ) { m_depthWrite = ( flag != 0 ); }

// ---------------------------------------------------------------------
// Pipeline creation (cached per program+vao+state)
// ---------------------------------------------------------------------
VkPipeline lsVulkan::getPipeline( VkProgramObj& prog, const VkVao& vao,
								  lsEnum topologyMode )
{
	// cache key
	char key[128];
	snprintf( key, sizeof( key ), "%u:%u:%x:%d%d%x%d%x%x",
			  m_currentProgram, m_currentVao, topologyMode,
			  m_depthTestEnabled, m_depthWrite, m_depthFunc,
			  m_cullEnabled, ( m_cullEnabled ? m_cullFace : 0 ),
			  m_blendEnabled ? m_blendSrc : 0, m_blendEnabled ? m_blendDst : 0 );
	auto cit = m_pipelineCache.find( key );
	if( cit != m_pipelineCache.end() ) return cit->second;

	// shader stages
	VkPipelineShaderStageCreateInfo stages[2]{};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = prog.vsModule;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = prog.fsModule;
	stages[1].pName = "main";

	// vertex input from the captured VAO layout
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;
	for( const auto& [idx, a] : vao.attribs )
	{
		if(! a.enabled ) continue;
		VkVertexInputBindingDescription b{};
		b.binding = idx;
		b.stride = a.stride ? (uint32_t)a.stride : (uint32_t)( a.size * 4 );
		b.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindings.push_back( b );

		VkVertexInputAttributeDescription ad{};
		ad.location = idx;
		ad.binding = idx;
		ad.format = lsVkMap::attribFormat( a.size, a.type );
		ad.offset = (uint32_t)a.offset;
		attributes.push_back( ad );
	}

	VkPipelineVertexInputStateCreateInfo vi{};
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi.vertexBindingDescriptionCount = (uint32_t)bindings.size();
	vi.pVertexBindingDescriptions = bindings.data();
	vi.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
	vi.pVertexAttributeDescriptions = attributes.data();

	VkPipelineInputAssemblyStateCreateInfo ia{};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = lsVkMap::topology( topologyMode );

	VkPipelineViewportStateCreateInfo vp{};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount = 1;

	VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dyn{};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates = dynStates;

	VkPipelineRasterizationStateCreateInfo rs{};
	rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rs.polygonMode = VK_POLYGON_MODE_FILL;
	rs.cullMode = m_cullEnabled ? lsVkMap::cullMode( m_cullFace )
								: VK_CULL_MODE_NONE;
	rs.frontFace = VK_FRONT_FACE_CLOCKWISE;		// GL CCW after Y-flip
	rs.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo ms{};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo ds{};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = m_depthTestEnabled ? VK_TRUE : VK_FALSE;
	ds.depthWriteEnable = m_depthWrite ? VK_TRUE : VK_FALSE;
	ds.depthCompareOp = lsVkMap::compareOp( m_depthFunc );
	ds.depthBoundsTestEnable = VK_FALSE;
	ds.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState blendAtt{};
	blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
							  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAtt.blendEnable = m_blendEnabled ? VK_TRUE : VK_FALSE;
	blendAtt.srcColorBlendFactor = lsVkMap::blendFactor( m_blendSrc );
	blendAtt.dstColorBlendFactor = lsVkMap::blendFactor( m_blendDst );
	blendAtt.colorBlendOp = VK_BLEND_OP_ADD;
	blendAtt.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAtt.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAtt.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo cb{};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments = &blendAtt;

	VkGraphicsPipelineCreateInfo pi{};
	pi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pi.stageCount = 2;
	pi.pStages = stages;
	pi.pVertexInputState = &vi;
	pi.pInputAssemblyState = &ia;
	pi.pViewportState = &vp;
	pi.pDynamicState = &dyn;
	pi.pRasterizationState = &rs;
	pi.pMultisampleState = &ms;
	pi.pDepthStencilState = &ds;
	pi.pColorBlendState = &cb;
	pi.layout = prog.pipelineLayout;
	pi.renderPass = m_renderPass;
	pi.subpass = 0;

	VkPipeline pipeline = VK_NULL_HANDLE;
	m_lastError = vkCreateGraphicsPipelines( m_device, VK_NULL_HANDLE, 1, &pi,
											 nullptr, &pipeline );
	if( m_lastError != VK_SUCCESS )
	{
		cout << "ERROR: lsVulkan - vkCreateGraphicsPipelines failed ("
			 << m_lastError << ")" << endl;
		return VK_NULL_HANDLE;
	}

	m_pipelineCache[key] = pipeline;
	return pipeline;
}

// ---------------------------------------------------------------------
// UBO ring
// ---------------------------------------------------------------------
uint32_t lsVulkan::allocUboSlice( uint32_t bytes )
{
	VkDeviceSize aligned = ( m_uboCursor + m_uboAlignment - 1 ) &
						   ~( m_uboAlignment - 1 );
	if( aligned + bytes > UBO_RING_SIZE )
	{
		aligned = 0;	// overflow: wrap (frames are small; log once)
		static bool warned = false;
		if(! warned ) { cout << "WARN: lsVulkan UBO ring wrapped" << endl; warned = true; }
	}
	m_uboCursor = aligned + bytes;
	return (uint32_t)aligned;
}

// ---------------------------------------------------------------------
// Draw calls
// ---------------------------------------------------------------------
void lsVulkan::DrawArrays( lsEnum mode, lsInt first, lsSizei count )
{
	if(! m_inFrame || ! m_currentProgram || ! m_currentVao ) return;
	auto pit = m_programs.find( m_currentProgram );
	auto vit = m_vaos.find( m_currentVao );
	if( pit == m_programs.end() || ! pit->second.linked ) return;
	if( vit == m_vaos.end() ) return;

	VkPipeline pipeline = getPipeline( pit->second, vit->second, mode );
	if(! pipeline ) return;

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
	vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );

	for( const auto& [idx, a] : vit->second.attribs )
	{
		if(! a.enabled ) continue;
		auto bit = m_buffers.find( a.buffer );
		if( bit == m_buffers.end() || ! bit->second.buffer ) continue;
		VkDeviceSize zero = 0;
		vkCmdBindVertexBuffers( cmd, idx, 1, &bit->second.buffer, &zero );
	}

	uint32_t uboOffset = 0;
	if( pit->second.layout.blockSize > 0 )
	{
		uboOffset = allocUboSlice( pit->second.layout.blockSize );
		memcpy( (uint8_t*)m_uboMapped + uboOffset, pit->second.uboStaging.data(),
				pit->second.layout.blockSize );
	}
	vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
							 pit->second.pipelineLayout, 0, 1,
							 &pit->second.descriptorSet, 1, &uboOffset );

	vkCmdDraw( cmd, (uint32_t)count, 1, (uint32_t)first, 0 );
}

void lsVulkan::DrawElements( lsEnum mode, lsSizei count, lsEnum type,
							 const void* indices )
{
	if(! m_inFrame || ! m_currentProgram || ! m_currentVao ) return;
	auto pit = m_programs.find( m_currentProgram );
	auto vit = m_vaos.find( m_currentVao );
	if( pit == m_programs.end() || ! pit->second.linked ) return;
	if( vit == m_vaos.end() ) return;

	VkPipeline pipeline = getPipeline( pit->second, vit->second, mode );
	if(! pipeline ) return;

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];
	vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );

	for( const auto& [idx, a] : vit->second.attribs )
	{
		if(! a.enabled ) continue;
		auto bit = m_buffers.find( a.buffer );
		if( bit == m_buffers.end() || ! bit->second.buffer ) continue;
		VkDeviceSize zero = 0;
		vkCmdBindVertexBuffers( cmd, idx, 1, &bit->second.buffer, &zero );
	}

	// index buffer: GL's `indices` is a byte offset into the bound buffer
	auto eit = m_buffers.find( vit->second.elementBuffer );
	if( eit == m_buffers.end() || ! eit->second.buffer ) return;
	const uint32_t indexSize = ( type == LS_UNSIGNED_INT ) ? 4 : 2;
	vkCmdBindIndexBuffer( cmd, eit->second.buffer, 0, lsVkMap::indexType( type ) );

	uint32_t uboOffset = 0;
	if( pit->second.layout.blockSize > 0 )
	{
		uboOffset = allocUboSlice( pit->second.layout.blockSize );
		memcpy( (uint8_t*)m_uboMapped + uboOffset, pit->second.uboStaging.data(),
				pit->second.layout.blockSize );
	}
	vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
							 pit->second.pipelineLayout, 0, 1,
							 &pit->second.descriptorSet, 1, &uboOffset );

	const uint32_t firstIndex = (uint32_t)( (size_t)indices / indexSize );
	vkCmdDrawIndexed( cmd, (uint32_t)count, 1, firstIndex, 0, 0 );
}


// =====================================================================
// STUBS — not yet implemented (V3+ milestones)
// =====================================================================

void lsVulkan::GenTextures( lsSizei, lsTextureID* )
{
	// V3+ stub
}

void lsVulkan::DeleteTextures( lsSizei, lsTextureID* )
{
	// V3+ stub
}

void lsVulkan::BindTexture( lsEnum, lsTextureID )
{
	// V3+ stub
}

void lsVulkan::ActiveTexture( lsEnum )
{
	// V3+ stub
}

void lsVulkan::TexImage2D( lsEnum, lsInt, lsInt, lsSizei, lsSizei, lsInt, lsEnum, lsEnum, const void* )
{
	// V3+ stub
}

void lsVulkan::TexParameteri( lsEnum, lsEnum, lsInt )
{
	// V3+ stub
}

void lsVulkan::TexParameterf( lsEnum, lsEnum, lsFloat )
{
	// V3+ stub
}

void lsVulkan::TexParameterfv( lsEnum, lsEnum, const lsFloat* )
{
	// V3+ stub
}

void lsVulkan::GenerateMipmap( lsEnum )
{
	// V3+ stub
}

void lsVulkan::PixelStorei( lsEnum, lsInt )
{
	// V3+ stub
}

void lsVulkan::GenFramebuffers( lsSizei, lsFBOID* )
{
	// V3+ stub
}

void lsVulkan::DeleteFramebuffers( lsSizei, lsFBOID* )
{
	// V3+ stub
}

void lsVulkan::BindFramebuffer( lsEnum, lsFBOID )
{
	// V3+ stub
}

void lsVulkan::FramebufferTexture2D( lsEnum, lsEnum, lsEnum, lsTextureID, lsInt )
{
	// V3+ stub
}

lsEnum lsVulkan::CheckFramebufferStatus( lsEnum )
{
	return (lsEnum){};
}

void lsVulkan::DrawBuffer( lsEnum )
{
	// V3+ stub
}

void lsVulkan::ReadBuffer( lsEnum )
{
	// V3+ stub
}

void lsVulkan::ReadPixels( lsInt, lsInt, lsSizei, lsSizei, lsEnum, lsEnum, void* )
{
	// V3+ stub
}

void lsVulkan::DepthRangef( lsFloat, lsFloat )
{
	// V3+ stub
}

void lsVulkan::ColorMask( lsBoolean, lsBoolean, lsBoolean, lsBoolean )
{
	// V3+ stub
}

void lsVulkan::PolygonOffset( lsFloat, lsFloat )
{
	// V3+ stub
}

void lsVulkan::ShadeModel( lsEnum )
{
	// V3+ stub
}

void lsVulkan::GenQueries( lsSizei, lsQueryID* )
{
	// V3+ stub
}

void lsVulkan::DeleteQueries( lsSizei, lsQueryID* )
{
	// V3+ stub
}

void lsVulkan::BeginQuery( lsEnum, lsQueryID )
{
	// V3+ stub
}

void lsVulkan::EndQuery( lsEnum )
{
	// V3+ stub
}

void lsVulkan::GetQueryObjectiv( lsQueryID, lsEnum, lsInt* )
{
	// V3+ stub
}

void lsVulkan::GetIntegerv( lsEnum, lsInt* )
{
	// V3+ stub
}
