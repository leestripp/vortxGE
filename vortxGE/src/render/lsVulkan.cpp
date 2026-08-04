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
	if(! createFramebuffers() )		return false;
	if(! createCommandPoolAndBuffers() )	return false;
	if(! createSyncObjects() )		return false;

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

	VkAttachmentReference colorRef{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;

	VkSubpassDependency dep{};
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;
	dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.srcAccessMask = 0;
	dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	ci.attachmentCount = 1;
	ci.pAttachments = &color;
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
		VkImageView attachments[] = { m_swapchainImageViews[i] };

		VkFramebufferCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ci.renderPass = m_renderPass;
		ci.attachmentCount = 1;
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

	VkClearValue clearValue;
	clearValue.color.float32[0] = m_clearColor[0];
	clearValue.color.float32[1] = m_clearColor[1];
	clearValue.color.float32[2] = m_clearColor[2];
	clearValue.color.float32[3] = m_clearColor[3];

	VkRenderPassBeginInfo rp{};
	rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp.renderPass = m_renderPass;
	rp.framebuffer = m_framebuffers[m_acquiredImage];
	rp.renderArea.offset = { 0, 0 };
	rp.renderArea.extent = m_swapchainExtent;
	rp.clearValueCount = 1;
	rp.pClearValues = &clearValue;

	vkCmdBeginRenderPass( cmd, &rp, VK_SUBPASS_CONTENTS_INLINE );

	// Dynamic viewport/scissor (used by draws in V2+; harmless now).
	// NOTE: Vulkan NDC is Y-down vs GL — V2 flips via negative height.
	VkViewport vp{};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)m_swapchainExtent.width;
	vp.height = (float)m_swapchainExtent.height;
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
// STUBS — not yet implemented (V2+ milestones)
// =====================================================================

void lsVulkan::GenTextures( lsSizei, lsTextureID* )
{
	// V2+ stub
}

void lsVulkan::DeleteTextures( lsSizei, lsTextureID* )
{
	// V2+ stub
}

void lsVulkan::BindTexture( lsEnum, lsTextureID )
{
	// V2+ stub
}

void lsVulkan::ActiveTexture( lsEnum )
{
	// V2+ stub
}

void lsVulkan::TexImage2D( lsEnum, lsInt, lsInt, lsSizei, lsSizei, lsInt, lsEnum, lsEnum, const void* )
{
	// V2+ stub
}

void lsVulkan::TexParameteri( lsEnum, lsEnum, lsInt )
{
	// V2+ stub
}

void lsVulkan::TexParameterf( lsEnum, lsEnum, lsFloat )
{
	// V2+ stub
}

void lsVulkan::TexParameterfv( lsEnum, lsEnum, const lsFloat* )
{
	// V2+ stub
}

void lsVulkan::GenerateMipmap( lsEnum )
{
	// V2+ stub
}

void lsVulkan::PixelStorei( lsEnum, lsInt )
{
	// V2+ stub
}

void lsVulkan::GenFramebuffers( lsSizei, lsFBOID* )
{
	// V2+ stub
}

void lsVulkan::DeleteFramebuffers( lsSizei, lsFBOID* )
{
	// V2+ stub
}

void lsVulkan::BindFramebuffer( lsEnum, lsFBOID )
{
	// V2+ stub
}

void lsVulkan::FramebufferTexture2D( lsEnum, lsEnum, lsEnum, lsTextureID, lsInt )
{
	// V2+ stub
}

lsEnum lsVulkan::CheckFramebufferStatus( lsEnum )
{
	return (lsEnum){};
}

void lsVulkan::DrawBuffer( lsEnum )
{
	// V2+ stub
}

void lsVulkan::ReadBuffer( lsEnum )
{
	// V2+ stub
}

void lsVulkan::ReadPixels( lsInt, lsInt, lsSizei, lsSizei, lsEnum, lsEnum, void* )
{
	// V2+ stub
}

void lsVulkan::GenBuffers( lsSizei, lsBufferID* )
{
	// V2+ stub
}

void lsVulkan::DeleteBuffers( lsSizei, lsBufferID* )
{
	// V2+ stub
}

void lsVulkan::BindBuffer( lsEnum, lsBufferID )
{
	// V2+ stub
}

void lsVulkan::BufferData( lsEnum, lsSizeiptr, const void*, lsEnum )
{
	// V2+ stub
}

void lsVulkan::GenVertexArrays( lsSizei, lsVAOID* )
{
	// V2+ stub
}

void lsVulkan::DeleteVertexArrays( lsSizei, lsVAOID* )
{
	// V2+ stub
}

void lsVulkan::BindVertexArray( lsVAOID )
{
	// V2+ stub
}

void lsVulkan::EnableVertexAttribArray( lsUInt )
{
	// V2+ stub
}

void lsVulkan::VertexAttribPointer( lsUInt, lsInt, lsEnum, lsBoolean, lsSizei, const void* )
{
	// V2+ stub
}

void lsVulkan::DrawArrays( lsEnum, lsInt, lsSizei )
{
	// V2+ stub
}

void lsVulkan::DrawElements( lsEnum, lsSizei, lsEnum, const void* )
{
	// V2+ stub
}

lsShaderID lsVulkan::CreateShader( lsEnum )
{
	return (lsShaderID){};
}

lsProgramID lsVulkan::CreateProgram(  )
{
	return (lsProgramID){};
}

void lsVulkan::ShaderSource( lsShaderID, lsSizei, const lsChar**, const lsInt* )
{
	// V2+ stub
}

void lsVulkan::CompileShader( lsShaderID )
{
	// V2+ stub
}

void lsVulkan::AttachShader( lsProgramID, lsShaderID )
{
	// V2+ stub
}

void lsVulkan::DetachShader( lsProgramID, lsShaderID )
{
	// V2+ stub
}

void lsVulkan::LinkProgram( lsProgramID )
{
	// V2+ stub
}

void lsVulkan::UseProgram( lsProgramID )
{
	// V2+ stub
}

void lsVulkan::DeleteShader( lsShaderID )
{
	// V2+ stub
}

void lsVulkan::DeleteProgram( lsProgramID )
{
	// V2+ stub
}

lsUniformID lsVulkan::GetUniformLocation( lsProgramID, const lsChar* )
{
	return (lsUniformID){};
}

void lsVulkan::GetShaderiv( lsShaderID, lsEnum, lsInt* )
{
	// V2+ stub
}

void lsVulkan::GetShaderInfoLog( lsShaderID, lsSizei, lsSizei*, lsChar* )
{
	// V2+ stub
}

void lsVulkan::GetProgramiv( lsProgramID, lsEnum, lsInt* )
{
	// V2+ stub
}

void lsVulkan::GetProgramInfoLog( lsProgramID, lsSizei, lsSizei*, lsChar* )
{
	// V2+ stub
}

void lsVulkan::Uniform1i( lsUniformID, lsInt )
{
	// V2+ stub
}

void lsVulkan::Uniform1f( lsUniformID, lsFloat )
{
	// V2+ stub
}

void lsVulkan::Uniform3f( lsUniformID, lsFloat, lsFloat, lsFloat )
{
	// V2+ stub
}

void lsVulkan::Uniform4f( lsUniformID, lsFloat, lsFloat, lsFloat, lsFloat )
{
	// V2+ stub
}

void lsVulkan::UniformMatrix3fv( lsUniformID, lsSizei, lsBoolean, const lsFloat* )
{
	// V2+ stub
}

void lsVulkan::UniformMatrix4fv( lsUniformID, lsSizei, lsBoolean, const lsFloat* )
{
	// V2+ stub
}

void lsVulkan::Enable( lsEnum )
{
	// V2+ stub
}

void lsVulkan::Disable( lsEnum )
{
	// V2+ stub
}

void lsVulkan::CullFace( lsEnum )
{
	// V2+ stub
}

void lsVulkan::BlendFunc( lsEnum, lsEnum )
{
	// V2+ stub
}

void lsVulkan::DepthFunc( lsEnum )
{
	// V2+ stub
}

void lsVulkan::DepthMask( lsBoolean )
{
	// V2+ stub
}

void lsVulkan::DepthRangef( lsFloat, lsFloat )
{
	// V2+ stub
}

void lsVulkan::ColorMask( lsBoolean, lsBoolean, lsBoolean, lsBoolean )
{
	// V2+ stub
}

void lsVulkan::PolygonOffset( lsFloat, lsFloat )
{
	// V2+ stub
}

void lsVulkan::ShadeModel( lsEnum )
{
	// V2+ stub
}

void lsVulkan::GenQueries( lsSizei, lsQueryID* )
{
	// V2+ stub
}

void lsVulkan::DeleteQueries( lsSizei, lsQueryID* )
{
	// V2+ stub
}

void lsVulkan::BeginQuery( lsEnum, lsQueryID )
{
	// V2+ stub
}

void lsVulkan::EndQuery( lsEnum )
{
	// V2+ stub
}

void lsVulkan::GetQueryObjectiv( lsQueryID, lsEnum, lsInt* )
{
	// V2+ stub
}

void lsVulkan::GetIntegerv( lsEnum, lsInt* )
{
	// V2+ stub
}
