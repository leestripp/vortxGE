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
#include "lsScreenshot.h"
#include "lsVulkanTypes.h"
#include "../debug/lsDebug.h"
#include "../debug/lsFrameDump.h"

#define GLFW_INCLUDE_NONE		// keep glfw3.h from pulling in GL headers
#include <GLFW/glfw3.h>

#include <cstring>
#if defined(VORTX_DEBUG_TOOLS)
#include <dlfcn.h>
#include "renderdoc_app.h" // vendored (MIT) — in-app capture API, VORTX_RDC
#endif
#include <set>
#include <algorithm>
#include <limits>

using namespace std;

namespace
{
#ifndef NDEBUG
	const bool kValidationDbg = true;	// Debug builds: request the Khronos layer if present
#else
	const bool kValidationDbg = false;
#endif
	// DIAGNOSTIC: VORTX_VK_VALIDATION=1 force-enables in any build type
	const bool kValidation = kValidationDbg || getenv( "VORTX_VK_VALIDATION" ) != nullptr;

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

// DIAGNOSTIC draw counters (VORTX_VK_DEBUG=1 prints at frame 30)
static int s_callsDE = 0, s_skipFrame = 0, s_skipProg = 0, s_skipVao = 0,
		   s_skipPass = 0, s_skipPipe = 0, s_issued = 0;
static long s_indices = 0;
static int s_issuedSwapchain = 0, s_issuedFBO = 0, s_callsDA = 0, s_issuedDA = 0;

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
	VORTX_DEBUG_ONLY( lsFrameDump::setBackend( "vulkan" ); )
}

lsVulkan::~lsVulkan()
{
	if(! m_initialized ) return;

	vkDeviceWaitIdle( m_device );

	// device idle -> every retire bucket is safe to destroy
	for( uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++ ) drainRetired( i );

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
	if( m_dummyAttribBuffer ) vkDestroyBuffer( m_device, m_dummyAttribBuffer, nullptr );
	if( m_dummyAttribMemory ) vkFreeMemory( m_device, m_dummyAttribMemory, nullptr );
	m_dummyAttribBuffer = VK_NULL_HANDLE;
	m_dummyAttribMemory = VK_NULL_HANDLE;
	for( auto& [id, tex] : m_textures )
	{
		if( tex.sampler ) vkDestroySampler( m_device, tex.sampler, nullptr );
		if( tex.view ) vkDestroyImageView( m_device, tex.view, nullptr );
		if( tex.image ) vkDestroyImage( m_device, tex.image, nullptr );
		if( tex.memory ) vkFreeMemory( m_device, tex.memory, nullptr );
	}
	m_textures.clear();
	if( m_uboBuffer ) vkDestroyBuffer( m_device, m_uboBuffer, nullptr );
	if( m_uboMemory ) vkFreeMemory( m_device, m_uboMemory, nullptr );
	if( m_descriptorPool ) vkDestroyDescriptorPool( m_device, m_descriptorPool, nullptr );
	for( uint32_t f = 0; f < FRAMES_IN_FLIGHT; f++ )
		if( m_drawPools[f] ) vkDestroyDescriptorPool( m_device, m_drawPools[f], nullptr );
	if( m_dummySampler ) vkDestroySampler( m_device, m_dummySampler, nullptr );
	if( m_dummyView ) vkDestroyImageView( m_device, m_dummyView, nullptr );
	if( m_dummyImage ) vkDestroyImage( m_device, m_dummyImage, nullptr );
	if( m_dummyMemory ) vkFreeMemory( m_device, m_dummyMemory, nullptr );

	cleanupSwapchain();

	for( auto& [id, fbo] : m_fbos )
		if( fbo.fb ) vkDestroyFramebuffer( m_device, fbo.fb, nullptr );
	m_fbos.clear();
	if( m_rpColorDepth ) vkDestroyRenderPass( m_device, m_rpColorDepth, nullptr );
	if( m_rpDepthOnly ) vkDestroyRenderPass( m_device, m_rpDepthOnly, nullptr );

	for( uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++ )
	{
		if( m_imageAvailable[i] ) vkDestroySemaphore( m_device, m_imageAvailable[i], nullptr );
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
		sizes[0].descriptorCount = 128;
		sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sizes[1].descriptorCount = 512;
		VkDescriptorPoolCreateInfo pi{};
		pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pi.maxSets = 256;
		pi.poolSizeCount = 2;
		pi.pPoolSizes = sizes;
		if( vkCreateDescriptorPool( m_device, &pi, nullptr, &m_descriptorPool ) != VK_SUCCESS )
			return false;

		// per-frame pools for PER-DRAW descriptor sets
		VkDescriptorPoolSize drawSizes[2]{};
		drawSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		drawSizes[0].descriptorCount = 4096;
		drawSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		drawSizes[1].descriptorCount = 16384;
		for( uint32_t f = 0; f < FRAMES_IN_FLIGHT; f++ )
		{
			VkDescriptorPoolCreateInfo dpi{};
			dpi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			dpi.maxSets = 4096;
			dpi.poolSizeCount = 2;
			dpi.pPoolSizes = drawSizes;
			if( vkCreateDescriptorPool( m_device, &dpi, nullptr,
										&m_drawPools[f] ) != VK_SUCCESS )
				return false;
		}

		if(! createDummyTexture() ) return false;
		if(! createOffscreenRenderPasses() ) return false;	// V4
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

	// VK_EXT_index_type_uint8 when available: the engine draws uint8
	// index buffers (VK_INDEX_TYPE_UINT8 needs this extension)
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties( m_physDevice, nullptr, &extCount, nullptr );
	std::vector<VkExtensionProperties> avail( extCount );
	vkEnumerateDeviceExtensionProperties( m_physDevice, nullptr, &extCount, avail.data() );
	bool hasUint8Index = false;
	for( const auto& e : avail )
		if( strcmp( e.extensionName, VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME ) == 0 )
			hasUint8Index = true;

	const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
							  VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME };

	// the extension also requires its feature to be enabled
	VkPhysicalDeviceIndexTypeUint8FeaturesEXT uint8Feat{};
	uint8Feat.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT;
	uint8Feat.indexTypeUint8 = VK_TRUE;

	VkDeviceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ci.pNext = hasUint8Index ? &uint8Feat : nullptr;
	ci.queueCreateInfoCount = (uint32_t)qcis.size();
	ci.pQueueCreateInfos = qcis.data();
	ci.enabledExtensionCount = hasUint8Index ? 2 : 1;
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

	// format: prefer B8G8R8A8_UNORM + nonlinear color space.
	// UNORM, not SRGB: the engine encodes linear->sRGB IN THE SHADER
	// (quad.frag / lsBlitStage toSRGB) exactly like the GL default
	// framebuffer path — an SRGB swapchain would hardware-encode a
	// second time (double gamma, washed-out image).
	uint32_t fcount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( m_physDevice, m_surface, &fcount, nullptr );
	std::vector<VkSurfaceFormatKHR> formats( fcount );
	vkGetPhysicalDeviceSurfaceFormatsKHR( m_physDevice, m_surface, &fcount, formats.data() );

	VkSurfaceFormatKHR chosen = formats[0];
	for( const auto& f : formats )
	{
		if( f.format == VK_FORMAT_B8G8R8A8_UNORM &&
			f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
		{
			chosen = f;
			break;
		}
	}
	m_swapchainFormat = chosen.format;

	// extent: the LIVE framebuffer size first (tracks fullscreen/resize
	// immediately; caps.currentExtent can lag behind on Wayland),
	// caps.currentExtent as fallback
	int fw = 0, fh = 0;
	glfwGetFramebufferSize( (GLFWwindow*)m_window, &fw, &fh );
	if( fw > 0 && fh > 0 )
	{
		m_swapchainExtent.width = std::clamp( (uint32_t)fw, caps.minImageExtent.width,
									  caps.maxImageExtent.width );
		m_swapchainExtent.height = std::clamp( (uint32_t)fh, caps.minImageExtent.height,
									   caps.maxImageExtent.height );
	} else if( caps.currentExtent.width != std::numeric_limits<uint32_t>::max() )
	{
		m_swapchainExtent = caps.currentExtent;
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
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT;	// screenshot readback

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

	// present-complete semaphores are PER SWAPCHAIN IMAGE: the present
	// engine's wait is not covered by the frame fence, so a per-frame-slot
	// semaphore can be re-signalled while still in use
	// (VUID-vkQueueSubmit-pSignalSemaphores-00067)
	for( auto s : m_renderFinished )
		if( s ) vkDestroySemaphore( m_device, s, nullptr );
	m_renderFinished.clear();
	m_renderFinished.resize( m_swapchainImages.size(), VK_NULL_HANDLE );
	{
		VkSemaphoreCreateInfo sci{};
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for( auto& s : m_renderFinished )
			if( vkCreateSemaphore( m_device, &sci, nullptr, &s ) != VK_SUCCESS )
				return false;
	}
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
	for( auto s : m_renderFinished )
		if( s ) vkDestroySemaphore( m_device, s, nullptr );
	m_renderFinished.clear();

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
	// no-op guard: callers (Resize paths, Wayland configure events) can
	// fire every frame — recreating the swapchain when the extent has
	// NOT changed allocates/leaks GBs per minute (the 'swapchain
	// recreated' storm) and eventually segfaults the driver
	int cw = 0, ch = 0;
	glfwGetFramebufferSize( (GLFWwindow*)m_window, &cw, &ch );
	if( cw == 0 || ch == 0 ) return true;	// minimized
	if( (uint32_t)cw == m_swapchainExtent.width &&
		(uint32_t)ch == m_swapchainExtent.height && m_swapchain )
		return true;	// nothing to do

	m_swapchainRecreates++;
	vkDeviceWaitIdle( m_device );
	for( uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++ ) drainRetired( i );

	int w = cw, h = ch;

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

	// the fence also guarantees nothing in flight references resources
	// retired into this slot — safe to actually destroy them now
	drainRetired( m_currentFrame );

	// the fence guarantees nothing pending references sets allocated from
	// this frame slot's draw pool — reset it for this frame's per-draw sets
	vkResetDescriptorPool( m_device, m_drawPools[m_currentFrame], 0 );

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

	m_uboCursor = 0;	// V2: reset the per-frame uniform ring
	m_passActive = false;	// V4: render pass opens lazily on first use
	m_activeTarget = 0xFFFFFFFF;

	// (render pass opens lazily — V4 deferred model)

	m_inFrame = true;
}

void lsVulkan::EndFrame()
{
	if(! m_initialized || ! m_inFrame ) return;

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];

	endActivePass();	// V4: close whichever pass is open (if any)
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
	submit.pSignalSemaphores = &m_renderFinished[m_acquiredImage];

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
	pi.pWaitSemaphores = &m_renderFinished[m_acquiredImage];
	pi.swapchainCount = 1;
	pi.pSwapchains = &m_swapchain;
	pi.pImageIndices = &m_acquiredImage;

	m_lastError = vkQueuePresentKHR( m_presentQueue, &pi );
	if( m_lastError == VK_ERROR_OUT_OF_DATE_KHR ||
		m_lastError == VK_SUBOPTIMAL_KHR )
		m_swapchainDirty = true;

	m_inFrame = false;
	m_currentFrame = ( m_currentFrame + 1 ) % FRAMES_IN_FLIGHT;
	VORTX_DEBUG_ONLY( lsFrameDump::frameTick(); )

	// growth probe: VORTX_VK_STATS=1 -> one line every 300 frames
	static int s_statFrame = 0;
	if( getenv( "VORTX_VK_STATS" ) && ++s_statFrame % 300 == 0 )
	{
		long rssKb = -1;
		if( FILE* f = fopen( "/proc/self/status", "r" ) )
		{
			char line[256];
			while( fgets( line, sizeof(line), f ) )
				if( sscanf( line, "VmRSS: %ld kB", &rssKb ) == 1 ) break;
			fclose( f );
		}
		cout << "VK STATS f=" << s_statFrame
			 << " pipes=" << m_pipelineCache.size()
			 << " progs=" << m_programs.size()
			 << " shaders=" << m_shaders.size()
			 << " texs=" << m_textures.size()
			 << " bufs=" << m_buffers.size()
			 << " vaos=" << m_vaos.size()
			 << " fbos=" << m_fbos.size()
			 << " swapRecreate=" << m_swapchainRecreates
			 << " rssMB=" << ( rssKb / 1024 ) << endl;
	}

#if defined(VORTX_DEBUG_TOOLS)
	// DIAGNOSTIC: one-shot swapchain readback (VORTX_SHOT=path.ppm)
	static int s_shotFrames = -1;
	if( s_shotFrames < 0 )
		s_shotFrames = ( getenv( "VORTX_SHOT" ) || getenv( "VORTX_VK_DEBUG" ) ||
						 getenv( "VORTX_FBO_SHOT" ) || getenv( "VORTX_DEPTH_SHOT" ) ||
						 getenv( "VORTX_RDC" ) ) ? 30 : 0x7FFFFF;
	if( s_shotFrames < 0x7FFFFF && --s_shotFrames == 0 )
	{
		s_shotFrames = 0x7FFFFF; // disarm: the whole block is one-shot
		if( getenv( "VORTX_VK_DEBUG" ) )
		{
			cout << "VK DRAWS: calls=" << s_callsDE
				 << " skipFrame=" << s_skipFrame << " skipProg=" << s_skipProg
				 << " skipVao=" << s_skipVao << " skipPass=" << s_skipPass
				 << " skipPipe=" << s_skipPipe << " issued=" << s_issued
				 << " indices=" << s_indices
				 << " | swap=" << s_issuedSwapchain << " fbo=" << s_issuedFBO
				 << " DA: calls=" << s_callsDA << " issued=" << s_issuedDA
				 << " target=" << m_activeTarget << endl;
			// dump first linked program's UBO staging (are matrices real?)
			for( auto& [pid, prog] : m_programs )
			{
				if(! prog.linked ) continue;
				cout << "PROG " << pid << " members:";
				for( auto& mem : prog.layout.members )
					cout << " " << mem.name << "@" << mem.offset;
				cout << endl << "  staging[0..63]:";
				const float* f = (const float*)prog.uboStaging.data();
				const int n = std::min<int>( 16, (int)prog.uboStaging.size() / 4 );
				for( int i = 0; i < n; i++ )
					printf( " %.3g", f[i] );
				// also dump the lighting region (offsets >= 480 bytes)
				if( prog.uboStaging.size() >= 820 )
				{
					cout << endl << "  staging[480..820]:";
					const float* g = f + 120;
					for( int i = 0; i < 85; i++ )
						printf( " %.3g", g[i] );
				}
				cout << endl;
			}
			// dump VAO attribute records (are buffers bound?)
			for( auto& [vid, vao] : m_vaos )
			{
				cout << "VAO " << vid << " elementBuf=" << vao.elementBuffer;
				for( auto& [ai, a] : vao.attribs )
					cout << " | attr" << ai << ": buf=" << a.buffer
						 << " sz=" << a.size << " stride=" << a.stride
						 << " off=" << a.offset << " en=" << a.enabled;
				cout << endl;
			}
			// dump the POSITION buffer of the first VAO with attributes
			for( auto& [vid, vao] : m_vaos )
			{
				auto ait = vao.attribs.find( 0 );
				if( ait == vao.attribs.end() || ! ait->second.enabled ) continue;
				auto bit = m_buffers.find( ait->second.buffer );
				if( bit == m_buffers.end() || ! bit->second.buffer ) continue;
				cout << "POSBUF vao=" << vid << " buf=" << ait->second.buffer
					 << " size=" << bit->second.size << " floats:";
				void* m = nullptr;
				vkMapMemory( m_device, bit->second.memory, 0,
							 std::min<VkDeviceSize>( bit->second.size, 48 ), 0, &m );
				const float* f = (const float*)m;
				for( int i = 0; i < 12; i++ ) printf( " %.3g", f[i] );
				cout << endl;
				vkUnmapMemory( m_device, bit->second.memory );
				break;
			}
			// dump first vertex buffer's first floats (is vertex data real?)
			for( auto& [bid, buf] : m_buffers )
			{
				if(! buf.buffer || buf.size < 32 ) continue;
				void* m = nullptr;
				vkMapMemory( m_device, buf.memory, 0, 32, 0, &m );
				const float* f = (const float*)m;
				cout << "BUF " << bid << " (" << buf.size << "B):";
				for( int i = 0; i < 8; i++ ) printf( " %.3g", f[i] );
				cout << endl;
				vkUnmapMemory( m_device, buf.memory );
				break;
			}
		}
		if( getenv( "VORTX_SHOT" ) )
			Screenshot( getenv( "VORTX_SHOT" ) );
		if( getenv( "VORTX_RDC" ) ) // RenderDoc capture: VORTX_RDC=1 or =file-template
		{
			static RENDERDOC_API_1_4_2* rdoc = nullptr;
			static bool rdocTried = false;
			if(! rdocTried )
			{
				rdocTried = true;
				// RTLD_NOLOAD: only succeed when injected by renderdoccmd/qrenderdoc
				void* mod = dlopen( "librenderdoc.so", RTLD_NOW | RTLD_NOLOAD );
				if( mod )
				{
					pRENDERDOC_GetAPI getAPI =
						(pRENDERDOC_GetAPI)dlsym( mod, "RENDERDOC_GetAPI" );
					if( getAPI )
						getAPI( eRENDERDOC_API_Version_1_4_2, (void**)&rdoc );
				}
				if( rdoc )
				{
					const char* tpl = getenv( "VORTX_RDC" );
					if( tpl && tpl[0] && !( tpl[0] == '1' && ! tpl[1] ) )
						rdoc->SetCaptureFilePathTemplate( tpl );
					else
						rdoc->SetCaptureFilePathTemplate( "/tmp/vortx_vk" );
				}
			}
			if( rdoc )
			{
				rdoc->TriggerCapture();
				cout << "RDC: capture triggered" << endl;
			}
			else
				cout << "RDC: librenderdoc.so not loaded "
						 "(launch under renderdoccmd capture)" << endl;
		}
		if( getenv( "VORTX_FBO_SHOT" ) )	// format: "id:path.ppm"
		{
			std::string spec = getenv( "VORTX_FBO_SHOT" );
			const size_t colon = spec.find( ':' );
			if( colon != std::string::npos )
				captureFbo( (uint32_t)atoi( spec.c_str() ),
							spec.c_str() + colon + 1 );
		}
		if( getenv( "VORTX_DEPTH_SHOT" ) )	// format: "id:path.png" (depth FBO)
		{
			std::string spec = getenv( "VORTX_DEPTH_SHOT" );
			const size_t colon = spec.find( ':' );
			if( colon != std::string::npos )
				captureFboDepth( (uint32_t)atoi( spec.c_str() ),
								 spec.c_str() + colon + 1 );
		}
	}
#endif
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
	ensurePassActive();	// V4
	if(! m_passActive ) return;

	const bool depthOnly = ( m_activeTarget != 0 &&
		m_fbos[m_activeTarget].variant == 1 );

	VkClearAttachment atts[2]{};
	uint32_t count = 0;
	if(( mask & LS_COLOR_BUFFER_BIT ) && ! depthOnly )
	{
		VkClearAttachment& a = atts[count++];
		a.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		a.colorAttachment = 0;
		a.clearValue.color.float32[0] = m_clearColor[0];
		a.clearValue.color.float32[1] = m_clearColor[1];
		a.clearValue.color.float32[2] = m_clearColor[2];
		a.clearValue.color.float32[3] = m_clearColor[3];
	}
	if( mask & LS_DEPTH_BUFFER_BIT )
	{
		VkClearAttachment& a = atts[count++];
		a.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		a.clearValue.depthStencil.depth = (float)m_clearDepth;
		a.clearValue.depthStencil.stencil = 0;
	}
	if(! count ) return;

	VkExtent2D extent = m_swapchainExtent;
	if( m_activeTarget != 0 )
		extent = { m_fbos[m_activeTarget].w, m_fbos[m_activeTarget].h };

	VkClearRect rect{};
	rect.rect.offset = { 0, 0 };
	rect.rect.extent = extent;
	rect.baseArrayLayer = 0;
	rect.layerCount = 1;
	vkCmdClearAttachments( m_commandBuffers[m_currentFrame], count, atts, 1, &rect );
}

void lsVulkan::Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h )
{
	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = w;
	m_viewport[3] = h;

	// V4: passes apply this at open; if one is already active, re-apply now
	if( m_passActive && w > 0 && h > 0 )
	{
		VkViewport vp{};
		vp.x = (float)x;
		vp.y = (float)( y + h );		// Y-flip for GL conventions
		vp.width = (float)w;
		vp.height = -(float)h;
		vp.minDepth = (float)( ( m_depthRangeN + m_depthRangeF ) * 0.5 );
		vp.maxDepth = (float)( vp.minDepth + ( m_depthRangeF - m_depthRangeN ) * 0.5 );
		vkCmdSetViewport( m_commandBuffers[m_currentFrame], 0, 1, &vp );
	}
}

void lsVulkan::StoreDefaultFramebuffer()
{
	m_savedFBO = m_boundFBO;	// V4
	BindFramebuffer( LS_FRAMEBUFFER, 0 );
}

void lsVulkan::RestoreDefaultFramebuffer()
{
	BindFramebuffer( LS_FRAMEBUFFER, m_savedFBO );	// V4
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
	if( buf.buffer || buf.memory ) retireBuffer( buf.buffer, buf.memory );
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

// ---------------------------------------------------------------------
// Deferred destruction (retire buckets)
// ---------------------------------------------------------------------
// Bucket selection: a resource deleted DURING frame M may be referenced
// by frame M's own submission -> bucket slot M, drained when slot M's
// fence is next waited (frame M+FRAMES_IN_FLIGHT). A resource deleted
// OUTSIDE a frame (m_currentFrame already advanced past the last
// submission that could reference it) must wait one more slot.
static inline uint32_t retireSlotFor( bool inFrame, uint32_t current, uint32_t n )
{
	return inFrame ? current : ( current + 1 ) % n;
}

void lsVulkan::retireBuffer( VkBuffer b, VkDeviceMemory m )
{
	if( m_device == VK_NULL_HANDLE ) return;
	if( ! m_initialized )
	{
		if( b ) vkDestroyBuffer( m_device, b, nullptr );
		if( m ) vkFreeMemory( m_device, m, nullptr );
		return;
	}
	m_retiredBufs[retireSlotFor( m_inFrame, m_currentFrame, FRAMES_IN_FLIGHT )]
		.push_back( { b, m } );
}

void lsVulkan::retireImage( VkImage i, VkDeviceMemory m )
{
	if( m_device == VK_NULL_HANDLE ) return;
	if( ! m_initialized )
	{
		if( i ) vkDestroyImage( m_device, i, nullptr );
		if( m ) vkFreeMemory( m_device, m, nullptr );
		return;
	}
	m_retiredImgs[retireSlotFor( m_inFrame, m_currentFrame, FRAMES_IN_FLIGHT )]
		.push_back( { i, m } );
}

void lsVulkan::retireView( VkImageView v )
{
	if( m_device == VK_NULL_HANDLE ) return;
	if( ! m_initialized ) { if( v ) vkDestroyImageView( m_device, v, nullptr ); return; }
	m_retiredViews[retireSlotFor( m_inFrame, m_currentFrame, FRAMES_IN_FLIGHT )]
		.push_back( v );
}

void lsVulkan::retireSampler( VkSampler s )
{
	if( m_device == VK_NULL_HANDLE ) return;
	if( ! m_initialized ) { if( s ) vkDestroySampler( m_device, s, nullptr ); return; }
	m_retiredSamplers[retireSlotFor( m_inFrame, m_currentFrame, FRAMES_IN_FLIGHT )]
		.push_back( s );
}

void lsVulkan::retireFbo( VkFramebuffer f )
{
	if( m_device == VK_NULL_HANDLE ) return;
	if( ! m_initialized ) { if( f ) vkDestroyFramebuffer( m_device, f, nullptr ); return; }
	m_retiredFbos[retireSlotFor( m_inFrame, m_currentFrame, FRAMES_IN_FLIGHT )]
		.push_back( f );
}

void lsVulkan::drainRetired( uint32_t slot )
{
	for( auto& r : m_retiredBufs[slot] )
	{
		if( r.buffer ) vkDestroyBuffer( m_device, r.buffer, nullptr );
		if( r.memory ) vkFreeMemory( m_device, r.memory, nullptr );
	}
	m_retiredBufs[slot].clear();
	for( auto& r : m_retiredImgs[slot] )
	{
		if( r.image ) vkDestroyImage( m_device, r.image, nullptr );
		if( r.memory ) vkFreeMemory( m_device, r.memory, nullptr );
	}
	m_retiredImgs[slot].clear();
	for( auto v : m_retiredViews[slot] )
		if( v ) vkDestroyImageView( m_device, v, nullptr );
	m_retiredViews[slot].clear();
	for( auto s : m_retiredSamplers[slot] )
		if( s ) vkDestroySampler( m_device, s, nullptr );
	m_retiredSamplers[slot].clear();
	for( auto f : m_retiredFbos[slot] )
		if( f ) vkDestroyFramebuffer( m_device, f, nullptr );
	m_retiredFbos[slot].clear();
}

void lsVulkan::ensureDummyAttribBuffer()
{
	if( m_dummyAttribBuffer ) return;

	VkBufferCreateInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = 16;
	bi.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if( vkCreateBuffer( m_device, &bi, nullptr, &m_dummyAttribBuffer ) != VK_SUCCESS )
		return;

	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements( m_device, m_dummyAttribBuffer, &reqs );
	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = reqs.size;
	ai.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	if( vkAllocateMemory( m_device, &ai, nullptr, &m_dummyAttribMemory ) != VK_SUCCESS )
		return;
	vkBindBufferMemory( m_device, m_dummyAttribBuffer, m_dummyAttribMemory, 0 );

	const float def[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// GL attrib default
	void* mapped = nullptr;
	vkMapMemory( m_device, m_dummyAttribMemory, 0, 16, 0, &mapped );
	memcpy( mapped, def, 16 );
	vkUnmapMemory( m_device, m_dummyAttribMemory );
}

void lsVulkan::DeleteBuffers( lsSizei n, lsBufferID* buffers )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		auto it = m_buffers.find( buffers[i] );
		if( it != m_buffers.end() )
		{
			retireBuffer( it->second.buffer, it->second.memory );
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
	VORTX_DEBUG_ONLY( lsFrameDump::trackShaderSource( (uint32_t)shader, src ); )
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
	VORTX_DEBUG_ONLY( lsFrameDump::trackAttach( (uint32_t)program, (uint32_t)shader ); )
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
	lsVkShader::assignInterface( vit->second.parsed, fit->second.parsed );
	prog.layout = lsVkShader::mergeLayouts( vit->second.parsed, fit->second.parsed );

	const std::string vsSrc = lsVkShader::assemble( vit->second.parsed, prog.layout );

	// collect every vertex-shader input location from the assembled source
	// (explicit layouts pass through; implicit ones got sequential numbers)
	{
		prog.vsInputLocations.clear();
		// engine sources write "layout (location = N)" (space before the
		// paren); the assembler emits "layout(location = N)" — handle both
		for( const char* key : { "layout(location = ", "layout (location = " } )
		{
			size_t pos = 0;
			const size_t keyLen = strlen( key );
			while( ( pos = vsSrc.find( key, pos ) ) != std::string::npos )
			{
				size_t numStart = pos + keyLen;
				size_t numEnd = vsSrc.find( ')', numStart );
				if( numEnd == std::string::npos ) break;
				uint32_t loc = (uint32_t)atoi( vsSrc.substr( numStart, numEnd - numStart ).c_str() );
				// only count `in` declarations (skip varyings written as `out`)
				size_t declStart = vsSrc.find_first_not_of( " \t", numEnd + 1 );
				if( declStart != std::string::npos && vsSrc.compare( declStart, 3, "in " ) == 0 )
					prog.vsInputLocations.push_back( loc );
				pos = numEnd;
			}
		}
	}
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

	// descriptor sets: ONE PER FRAME IN FLIGHT (a set may only be written
	// when no pending command buffer references it) + initial writes
	// (UBO ring + dummy textures)
	VkDescriptorSetLayout layouts[FRAMES_IN_FLIGHT];
	for( uint32_t f = 0; f < FRAMES_IN_FLIGHT; f++ ) layouts[f] = prog.setLayout;
	VkDescriptorSetAllocateInfo dai{};
	dai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dai.descriptorPool = m_descriptorPool;
	dai.descriptorSetCount = FRAMES_IN_FLIGHT;
	dai.pSetLayouts = layouts;
	if( vkAllocateDescriptorSets( m_device, &dai, prog.descriptorSets ) != VK_SUCCESS )
	{ prog.log = "vkAllocateDescriptorSets failed"; return false; }

	for( uint32_t f = 0; f < FRAMES_IN_FLIGHT; f++ )
	{
		std::vector<VkWriteDescriptorSet> writes;
		VkDescriptorBufferInfo uboInfo{};
		uboInfo.buffer = m_uboBuffer;
		uboInfo.offset = 0;
		uboInfo.range = std::max<VkDeviceSize>( prog.layout.blockSize, 16 );
		VkWriteDescriptorSet uw{};
		uw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uw.dstSet = prog.descriptorSets[f];
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
			sw.dstSet = prog.descriptorSets[f];
			sw.dstBinding = 1 + (uint32_t)i;
			sw.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			sw.descriptorCount = count;
			sw.pImageInfo = infos->data();
			writes.push_back( sw );
		}
		vkUpdateDescriptorSets( m_device, (uint32_t)writes.size(), writes.data(), 0, nullptr );
	}

	prog.linked = true;
	return true;
}

void lsVulkan::LinkProgram( lsProgramID program )
{
	auto it = m_programs.find( program );
	if( it == m_programs.end() ) return;
	it->second.linked = linkProgramResources( it->second );
	VORTX_DEBUG_ONLY( lsFrameDump::trackLink( (uint32_t)program ); )
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

	// GL-style array element lookup: "vxLightPos[2]". Members are stored
	// by base name with arraySize/elemStride; encode the element index in
	// bits 16+ of the location (member indices are < 1000, sampler
	// locations are 1000+binding, element locations are >= 65536).
	const std::string n = name;
	const size_t lb = n.find( '[' );
	if( lb != std::string::npos && n.back() == ']' )
	{
		const uint32_t elem = (uint32_t)strtoul( n.c_str() + lb + 1, nullptr, 10 );
		auto bi = it->second.uniformIndex.find( n.substr( 0, lb ) );
		if( bi != it->second.uniformIndex.end() )
		{
			const lsVkUniformMember& mem = it->second.layout.members[bi->second];
			if( mem.arraySize && elem < mem.arraySize )
				return (lsUniformID)( bi->second + ( elem << 16 ) );
		}
		// sampler array element: "tex_shadowmaps[1]" — one descriptor
		// binding with N elements; encode 1000+binding + (elem << 16)
		// (Uniform1i checks low16 >= 1000 before the UBO route)
		auto ai = it->second.samplerBinding.find( n.substr( 0, lb ) );
		if( ai != it->second.samplerBinding.end() )
		{
			const uint32_t binding = ai->second;
			uint32_t count = 1;
			const size_t k = (size_t)binding - 1;
			if( k < it->second.layout.samplerArrays.size() &&
				it->second.layout.samplerArrays[k] > 0 )
				count = it->second.layout.samplerArrays[k];
			if( elem < count )
				return (lsUniformID)( 1000 + binding + ( elem << 16 ) );
		}
	}
	return -1;
}

// ---------------------------------------------------------------------
// Uniform setters (write into the program's CPU-side block image)
// ---------------------------------------------------------------------
void lsVulkan::writeUniformMember( VkProgramObj& prog, lsUniformID location,
								   const void* data, uint32_t bytes )
{
	if( location < 0 ) return;
	const uint32_t memIdx = (uint32_t)location & 0xFFFF;
	const uint32_t elem = (uint32_t)location >> 16;
	if( memIdx >= prog.layout.members.size() ) return;
	const lsVkUniformMember& mem = prog.layout.members[memIdx];
	if( elem && ( !mem.arraySize || elem >= mem.arraySize ) ) return;
	const uint32_t off = mem.offset + elem * mem.elemStride;
	if( off + bytes > prog.uboStaging.size() ) return;
	memcpy( prog.uboStaging.data() + off, data, bytes );
}

void lsVulkan::Uniform1i( lsUniformID location, lsInt v0 )
{
	auto it = m_programs.find( m_currentProgram );
	if( it == m_programs.end() ) return;
	if( location >= 65536 )
	{
		const uint32_t low = (uint32_t)location & 0xFFFF;
		if( low >= 1000 )
		{
			// sampler array element assignment (e.g. tex_shadowmaps[1])
			const uint32_t binding = low - 1000;
			const uint32_t elem = (uint32_t)location >> 16;
			it->second.samplerUnitsElem[binding][elem] = (uint32_t)v0;
			for( uint32_t f = 0; f < FRAMES_IN_FLIGHT; f++ )
				it->second.lastBoundTex[f].erase( binding );
			return;
		}
		// array element uniform (e.g. vxLightType[1]) — staged below
		writeUniformMember( it->second, location, &v0, sizeof( v0 ) );
		return;
	}
	if( location >= 1000 )
	{
		// sampler assignment: record binding -> GL texture unit (V3)
		const uint32_t binding = (uint32_t)( location - 1000 );
		it->second.samplerUnits[binding] = (uint32_t)v0;
		for( uint32_t f = 0; f < FRAMES_IN_FLIGHT; f++ )
			it->second.lastBoundTex[f].erase( binding );
		return;
	}
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
	if( it == m_programs.end() || location < 0 ) return;
	const uint32_t memIdx = (uint32_t)location & 0xFFFF;
	const uint32_t elem = (uint32_t)location >> 16;
	if( memIdx >= it->second.layout.members.size() ) return;
	const lsVkUniformMember& mem = it->second.layout.members[memIdx];
	if( elem && ( !mem.arraySize || elem >= mem.arraySize ) ) return;
	const uint32_t baseOff = mem.offset + elem * mem.elemStride;
	// std140 mat3: 3 columns at 16-byte stride (packed 9 floats in)
	for( lsSizei m = 0; m < count; m++ )
	{
		uint8_t* dst = it->second.uboStaging.data() + baseOff + m * 48;
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
	else if( cap == LS_POLYGON_OFFSET_FILL ) m_polyOffsetEnabled = true;
}

void lsVulkan::Disable( lsEnum cap )
{
	if( cap == LS_DEPTH_TEST ) m_depthTestEnabled = false;
	else if( cap == LS_CULL_FACE ) m_cullEnabled = false;
	else if( cap == LS_BLEND ) m_blendEnabled = false;
	else if( cap == LS_POLYGON_OFFSET_FILL ) m_polyOffsetEnabled = false;
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
	char key[160];
	uint32_t poFactorBits, poUnitsBits;
	memcpy( &poFactorBits, &m_polyOffsetFactor, 4 );
	memcpy( &poUnitsBits, &m_polyOffsetUnits, 4 );
	snprintf( key, sizeof( key ), "%u:%u:%x:%p:%d%d%x%d%x%x%x:%d%x%x",
			  m_currentProgram, m_currentVao, topologyMode, (void*)m_activeRenderPass,
			  m_depthTestEnabled, m_depthWrite, m_depthFunc,
			  m_cullEnabled, ( m_cullEnabled ? m_cullFace : 0 ),
			  ( m_blendEnabled ? m_blendSrc : 0 ),
			  ( m_blendEnabled ? m_blendDst : 0 ),
			  (int)m_polyOffsetEnabled, poFactorBits, poUnitsBits );
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

	// Shader inputs the VAO does not provide: GL reads the attrib default
	// (0,0,0,1). Vulkan has no default — back those locations with a
	// stride-0 instance-rate dummy binding (every vertex reads offset 0).
	{
		bool needDummy = false;
		for( uint32_t loc : prog.vsInputLocations )
		{
			bool covered = false;
			for( const auto& ad : attributes )
				if( ad.location == loc ) { covered = true; break; }
			if( covered ) continue;
			VkVertexInputAttributeDescription ad{};
			ad.location = loc;
			ad.binding = DUMMY_ATTRIB_BINDING;
			ad.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			ad.offset = 0;
			attributes.push_back( ad );
			needDummy = true;
		}
		if( needDummy )
		{
			VkVertexInputBindingDescription b{};
			b.binding = DUMMY_ATTRIB_BINDING;
			b.stride = 0;
			b.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
			bindings.push_back( b );
		}
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
	// Empirically CCW matches the engine's meshes AND the composite quad
	// with the Y-flipped viewport
#if defined(VORTX_DEBUG_TOOLS)
	// DIAGNOSTIC: VORTX_VK_NOCULL=1 disables culling entirely
	static const bool nocull = getenv( "VORTX_VK_NOCULL" ) != nullptr;
	if( nocull ) rs.cullMode = VK_CULL_MODE_NONE;
	// DIAGNOSTIC: VORTX_VK_CW=1 flips front-face back for testing
	static const bool forceCW = getenv( "VORTX_VK_CW" ) != nullptr;
#else
	constexpr bool forceCW = false;
#endif
	rs.frontFace = forceCW ? VK_FRONT_FACE_CLOCKWISE
						   : VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rs.lineWidth = 1.0f;
	// L1.2: depth bias from glPolygonOffset state (shadow depth pass)
	rs.depthBiasEnable = m_polyOffsetEnabled ? VK_TRUE : VK_FALSE;
	rs.depthBiasSlopeFactor = m_polyOffsetFactor;
	rs.depthBiasConstantFactor = m_polyOffsetUnits;

	VkPipelineMultisampleStateCreateInfo ms{};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo ds{};
	ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds.depthTestEnable = m_depthTestEnabled ? VK_TRUE : VK_FALSE;
#if defined(VORTX_DEBUG_TOOLS)
	static const bool nodepth = getenv( "VORTX_VK_NODEPTH" ) != nullptr;
	if( nodepth ) ds.depthTestEnable = VK_FALSE;	// DIAGNOSTIC
#endif
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
	pi.renderPass = m_activeRenderPass ? m_activeRenderPass : m_renderPass;	// V4
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
	s_callsDA++;
	if(! m_inFrame || ! m_currentProgram || ! m_currentVao ) return;
	ensurePassActive();	// V4
	if(! m_passActive ) return;
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
	// harmless when the pipeline has no dummy attributes
	ensureDummyAttribBuffer();
	if( m_dummyAttribBuffer )
	{
		VkDeviceSize zero = 0;
		vkCmdBindVertexBuffers( cmd, DUMMY_ATTRIB_BINDING, 1, &m_dummyAttribBuffer, &zero );
	}

	uint32_t uboOffset = 0;
	if( pit->second.layout.blockSize > 0 )
	{
		uboOffset = allocUboSlice( pit->second.layout.blockSize );
		memcpy( (uint8_t*)m_uboMapped + uboOffset, pit->second.uboStaging.data(),
				pit->second.layout.blockSize );
	}
	// per-draw set: written NOW with the current GL texture state
	// DIAGNOSTIC: dump texture resolution for swapchain-targeted draws
#if defined(VORTX_DEBUG_TOOLS)
	if( getenv( "VORTX_VK_DEBUG" ) && m_activeTarget == 0 )
	{
		static int s_dumped = 0;
		if( s_dumped < 6 )
		{
			s_dumped++;
			cout << "SWDRAW prog=" << m_currentProgram
				 << " vao=" << m_currentVao << " viewport=["
				 << m_viewport[0] << "," << m_viewport[1] << ","
				 << m_viewport[2] << "," << m_viewport[3] << "] samplers:";
			for( size_t k = 0; k < pit->second.layout.samplers.size(); k++ )
			{
				const uint32_t binding = 1 + (uint32_t)k;
				uint32_t unit = (uint32_t)k;
				auto uit = pit->second.samplerUnits.find( binding );
				if( uit != pit->second.samplerUnits.end() ) unit = uit->second;
				cout << " b" << binding << "->unit" << unit
					 << "->tex" << ( unit < 32 ? m_textureUnits[unit] : 0 );
			}
			cout << endl;
		}
	}
#endif
	VkDescriptorSet drawSet = allocDrawSet( pit->second );
	vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
							 pit->second.pipelineLayout, 0, 1,
							 &drawSet, 1, &uboOffset );

	s_issuedDA++;
	if( m_activeTarget == 0 ) s_issuedSwapchain++; else s_issuedFBO++;
	vkCmdDraw( cmd, (uint32_t)count, 1, (uint32_t)first, 0 );
	VORTX_DEBUG_ONLY( dumpDraw( mode, count, (uint32_t)first, false ); )
}

void lsVulkan::DrawElements( lsEnum mode, lsSizei count, lsEnum type,
							 const void* indices )
{
	s_callsDE++;
	if(! m_inFrame ) { s_skipFrame++; return; }
	if(! m_currentProgram ) { s_skipProg++; return; }
	if(! m_currentVao ) { s_skipVao++; return; }
	ensurePassActive();	// V4
	if(! m_passActive ) { s_skipPass++; return; }
	auto pit = m_programs.find( m_currentProgram );
	auto vit = m_vaos.find( m_currentVao );
	if( pit == m_programs.end() || ! pit->second.linked ) return;
	if( vit == m_vaos.end() ) return;

	VkPipeline pipeline = getPipeline( pit->second, vit->second, mode );
	if(! pipeline ) { s_skipPipe++; return; }
	s_issued++;
	s_indices += count;
	if( m_activeTarget == 0 ) s_issuedSwapchain++; else s_issuedFBO++;

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
	// harmless when the pipeline has no dummy attributes
	ensureDummyAttribBuffer();
	if( m_dummyAttribBuffer )
	{
		VkDeviceSize zero = 0;
		vkCmdBindVertexBuffers( cmd, DUMMY_ATTRIB_BINDING, 1, &m_dummyAttribBuffer, &zero );
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
	// per-draw set: written NOW with the current GL texture state
	VkDescriptorSet drawSet = allocDrawSet( pit->second );
	vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
							 pit->second.pipelineLayout, 0, 1,
							 &drawSet, 1, &uboOffset );

	const uint32_t firstIndex = (uint32_t)( (size_t)indices / indexSize );
	vkCmdDrawIndexed( cmd, (uint32_t)count, 1, firstIndex, 0, 0 );
	VORTX_DEBUG_ONLY( dumpDraw( mode, count, firstIndex, true ); )
}

// =====================================================================
// D2 diagnostic: per-draw frame dump (VORTX_FRAME_DUMP) — Vulkan side.
// Snapshots the backend's INTERNAL state at draw time (uboStaging at
// std140 offsets, sampler-unit maps, texture units) so the GL/VK diff
// exposes staging/layout bugs that API-level tracking would hide.
// =====================================================================
#if defined(VORTX_DEBUG_TOOLS)
void lsVulkan::dumpDraw( lsEnum mode, lsSizei count, uint32_t first,
						 bool indexed )
{
	lsFrameDump::DrawRec* r = lsFrameDump::openDraw();
	if(! r ) return;
	r->fbo = m_activeTarget;
	r->prog = m_currentProgram;
	r->vao = m_currentVao;
	r->mode = mode;
	r->count = (uint32_t)count;
	r->first = first;
	r->indexed = indexed;
	for( int i = 0; i < 4; i++ ) r->vp[i] = m_viewport[i];
	r->depthTest = m_depthTestEnabled;
	r->depthWrite = m_depthWrite;
	r->depthFunc = m_depthFunc;
	r->cull = m_cullEnabled;
	r->cullFace = m_cullFace;
	r->blend = m_blendEnabled;
	r->blendSrc = m_blendSrc;
	r->blendDst = m_blendDst;
	for( uint32_t u = 0; u < 32; u++ )
		if( m_textureUnits[u] ) r->tex[u] = m_textureUnits[u];

	auto pit = m_programs.find( m_currentProgram );
	if( pit == m_programs.end() || ! pit->second.linked )
	{ lsFrameDump::commitDraw(); return; }
	const VkProgramObj& prog = pit->second;

	// sampler bindings -> GL texture units (as int uniforms)
	for( size_t k = 0; k < prog.layout.samplers.size(); k++ )
	{
		const uint32_t binding = 1 + (uint32_t)k;
		const uint32_t arr = ( k < prog.layout.samplerArrays.size() )
							 ? prog.layout.samplerArrays[k] : 0;
		if( arr > 0 )
		{
			// mirror allocDrawSet: element units fall back to the
			// binding-level (scalar) unit, then to the default unit k
			int baseUnit = (int)k;
			auto bit = prog.samplerUnits.find( binding );
			if( bit != prog.samplerUnits.end() ) baseUnit = (int)bit->second;
			auto ae = prog.samplerUnitsElem.find( binding );
			for( uint32_t e = 0; e < arr; e++ )
			{
				int unit = baseUnit;
				if( ae != prog.samplerUnitsElem.end() )
				{
					auto ue = ae->second.find( e );
					if( ue != ae->second.end() ) unit = (int)ue->second;
				}
				r->iu[prog.layout.samplers[k] + "[" + std::to_string( e ) + "]"]
					= { unit };
			}
		}
		else
		{
			int unit = -1;
			auto uit = prog.samplerUnits.find( binding );
			if( uit != prog.samplerUnits.end() ) unit = (int)uit->second;
			r->iu[prog.layout.samplers[k]] = { unit };
		}
	}

	// UBO staging snapshot at std140 member offsets
	static const std::set<std::string> intTypes =
		{ "int", "uint", "bool", "ivec2", "ivec3", "ivec4",
		  "uvec2", "uvec3", "uvec4", "bvec2", "bvec3", "bvec4" };
	for( const auto& mem : prog.layout.members )
	{
		const uint32_t elems = mem.arraySize ? mem.arraySize : 1;
		const bool isInt = intTypes.count( mem.glslType ) != 0;
		const bool isMat3 = ( mem.glslType == "mat3" );
		if( isMat3 )
		{
			// std140 mat3: 3 column vec3s at 16B stride — un-pack to 9
			std::vector<float> vals;
			for( uint32_t e = 0; e < elems; e++ )
			{
				const uint8_t* base = prog.uboStaging.data() + mem.offset
									  + e * mem.elemStride;
				for( int col = 0; col < 3; col++ )
				{
					float c3[3];
					memcpy( c3, base + col * 16, 12 );
					vals.insert( vals.end(), c3, c3 + 3 );
				}
			}
			r->u[mem.name] = std::move( vals );
			continue;
		}
		const uint32_t perElem = mem.size / elems;	// packed bytes per element
		if( isInt )
		{
			std::vector<int> vals;
			for( uint32_t e = 0; e < elems; e++ )
			{
				const uint8_t* base = prog.uboStaging.data() + mem.offset
									  + e * mem.elemStride;
				for( uint32_t b = 0; b + 4 <= perElem; b += 4 )
				{
					int iv;
					memcpy( &iv, base + b, 4 );
					vals.push_back( iv );
				}
			}
			r->iu[mem.name] = std::move( vals );
		}
		else
		{
			std::vector<float> vals;
			for( uint32_t e = 0; e < elems; e++ )
			{
				const uint8_t* base = prog.uboStaging.data() + mem.offset
									  + e * mem.elemStride;
				for( uint32_t b = 0; b + 4 <= perElem; b += 4 )
				{
					float fv;
					memcpy( &fv, base + b, 4 );
					vals.push_back( fv );
				}
			}
			r->u[mem.name] = std::move( vals );
		}
	}
	lsFrameDump::commitDraw();
}
#endif // VORTX_DEBUG_TOOLS


// =====================================================================
// V3: textures — real uploads replace the dummy 1x1 white texture
// =====================================================================

VkCommandBuffer lsVulkan::beginOneShot()
{
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = m_commandPool;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = 1;
	VkCommandBuffer cmd;
	vkAllocateCommandBuffers( m_device, &ai, &cmd );
	VkCommandBufferBeginInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer( cmd, &bi );
	return cmd;
}

void lsVulkan::endOneShot( VkCommandBuffer cmd )
{
	vkEndCommandBuffer( cmd );
	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	vkQueueSubmit( m_graphicsQueue, 1, &submit, VK_NULL_HANDLE );
	vkQueueWaitIdle( m_graphicsQueue );
	vkFreeCommandBuffers( m_device, m_commandPool, 1, &cmd );
}

// ---------------------------------------------------------------------
// Texture objects
// ---------------------------------------------------------------------
void lsVulkan::GenTextures( lsSizei n, lsTextureID* textures )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		uint32_t id = m_nextTexID++;
		m_textures[id] = VkTex{};
		textures[i] = id;
	}
}

void lsVulkan::DeleteTextures( lsSizei n, lsTextureID* textures )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		auto it = m_textures.find( textures[i] );
		if( it == m_textures.end() ) continue;
		VkTex& t = it->second;
		if( t.sampler ) retireSampler( t.sampler );
		if( t.view ) retireView( t.view );
		if( t.image || t.memory ) retireImage( t.image, t.memory );
		m_textures.erase( it );
	}
}

void lsVulkan::ActiveTexture( lsEnum texture )
{
	const uint32_t unit = texture - LS_TEXTURE0;
	m_activeUnit = ( unit < 32 ) ? unit : 0;
}

void lsVulkan::BindTexture( lsEnum target, lsTextureID texture )
{
	(void)target;	// LS_TEXTURE_2D only for now
	m_textureUnits[m_activeUnit] = texture;
}

bool lsVulkan::createTextureViewAndSampler( VkTex& tex )
{
	if( tex.view ) retireView( tex.view );
	if( tex.sampler ) retireSampler( tex.sampler );
	tex.view = VK_NULL_HANDLE;
	tex.sampler = VK_NULL_HANDLE;

	const bool isDepth = ( tex.format == VK_FORMAT_D32_SFLOAT );

	VkImageViewCreateInfo vi{};
	vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vi.image = tex.image;
	vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vi.format = tex.format;
	vi.subresourceRange = { isDepth ? (VkImageAspectFlags)VK_IMAGE_ASPECT_DEPTH_BIT
									: (VkImageAspectFlags)VK_IMAGE_ASPECT_COLOR_BIT,
							0, tex.mipLevels, 0, 1 };
	m_lastError = vkCreateImageView( m_device, &vi, nullptr, &tex.view );
	if( m_lastError != VK_SUCCESS ) return false;

	auto filterOf = []( lsEnum f ) {
		return ( f == LS_NEAREST ) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
	};
	auto wrapOf = []( lsEnum w ) {
		switch( w )
		{
		case LS_CLAMP_TO_EDGE:	return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case LS_CLAMP_TO_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case LS_MIRRORED_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		default:				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	};

	VkSamplerCreateInfo si{};
	si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	si.magFilter = filterOf( tex.magFilter );
	si.minFilter = filterOf( tex.minFilter );
	si.mipmapMode = ( tex.minFilter == LS_LINEAR_MIPMAP_LINEAR )
		? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
	si.addressModeU = wrapOf( tex.wrapS );
	si.addressModeV = wrapOf( tex.wrapT );
	si.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	si.maxLod = tex.mipsGenerated ? (float)( tex.mipLevels - 1 ) : 0.0f;
	// L1.3: shadow maps sample through a comparison sampler
	// (GL_LEQUAL semantics — lit when ref <= stored depth)
	si.compareEnable = tex.compareEnable ? VK_TRUE : VK_FALSE;
	si.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	m_lastError = vkCreateSampler( m_device, &si, nullptr, &tex.sampler );
	return m_lastError == VK_SUCCESS;
}

bool lsVulkan::uploadTexture( VkTex& tex, lsEnum internalFormat, lsEnum format,
							  lsEnum type, uint32_t w, uint32_t h, const void* pixels )
{
	// Map (internalformat, format, type) -> VkFormat + source layout.
	// RGB sources are expanded to RGBA on upload (RGB8 has poor support).
	bool srgb = false, expand = false, depth = false;
	VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
	uint32_t srcBpp = 4, dstBpp = 4;
	bool wide = ( type == LS_UNSIGNED_SHORT );	// 16-bit channels

	srgb = ( internalFormat == LS_SRGB || internalFormat == LS_SRGB_ALPHA ||
			 internalFormat == LS_SRGB8 || internalFormat == LS_SRGB8_ALPHA8 );

	if( internalFormat == LS_DEPTH_COMPONENT || format == LS_DEPTH_COMPONENT )
	{
		depth = true;
		vkFormat = VK_FORMAT_D32_SFLOAT;
		srcBpp = dstBpp = 4;
	} else if( format == LS_ALPHA )
	{
		vkFormat = VK_FORMAT_R8_UNORM;
		srcBpp = dstBpp = 1;
	} else if( wide )
	{
		vkFormat = VK_FORMAT_R16G16B16A16_UNORM;
		srcBpp = ( format == LS_RGB ) ? 6 : 8;
		dstBpp = 8;
		expand = ( format == LS_RGB );
	} else
	{
		vkFormat = srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
		srcBpp = ( format == LS_RGB ) ? 3 : 4;
		dstBpp = 4;
		expand = ( format == LS_RGB );
	}

	// recreate image storage (retired: the old image may be referenced by
	// an in-flight frame's descriptor set — eager destruction faults the GPU)
	if( tex.image || tex.memory ) retireImage( tex.image, tex.memory );
	tex.image = VK_NULL_HANDLE;
	tex.memory = VK_NULL_HANDLE;
	tex.format = vkFormat;
	tex.width = w;
	tex.height = h;
	tex.mipLevels = 1;
	tex.mipsGenerated = false;
	if(! depth && ! pixels ) tex.mipLevels = 1;	// render targets: no mips (V4)
	else if(! depth )
	{
		uint32_t dim = std::max( w, h );
		while( dim > 1 ) { dim >>= 1; tex.mipLevels++; }
	}

	VkImageCreateInfo ii{};
	ii.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = vkFormat;
	ii.extent = { w, h, 1 };
	ii.mipLevels = tex.mipLevels;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			   VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if( depth ) ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT |
						   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	else if(! pixels ) ii.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	m_lastError = vkCreateImage( m_device, &ii, nullptr, &tex.image );
	if( m_lastError != VK_SUCCESS ) return false;

	VkMemoryRequirements reqs;
	vkGetImageMemoryRequirements( m_device, tex.image, &reqs );
	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = reqs.size;
	ai.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits,
										 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
	m_lastError = vkAllocateMemory( m_device, &ai, nullptr, &tex.memory );
	if( m_lastError != VK_SUCCESS ) return false;
	vkBindImageMemory( m_device, tex.image, tex.memory, 0 );

	VkCommandBuffer cmd = beginOneShot();

	// transition to TRANSFER_DST (or straight to attachment layout when
	// there is no pixel data — V4 render targets)
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = tex.image;
	barrier.subresourceRange = { depth ? (VkImageAspectFlags)VK_IMAGE_ASPECT_DEPTH_BIT
									   : (VkImageAspectFlags)VK_IMAGE_ASPECT_COLOR_BIT,
								 0, tex.mipLevels, 0, 1 };
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = pixels ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
							   : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = pixels ? VK_ACCESS_TRANSFER_WRITE_BIT
								   : VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						  pixels ? VK_PIPELINE_STAGE_TRANSFER_BIT
								 : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						  0, 0, nullptr, 0, nullptr, 1, &barrier );

	if( pixels )
	{
		// staging buffer (with optional 3->4 channel expansion)
		const VkDeviceSize dataSize = (VkDeviceSize)w * h * dstBpp;
		VkBufferCreateInfo bi{};
		bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bi.size = dataSize;
		bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkBuffer staging;
		if( vkCreateBuffer( m_device, &bi, nullptr, &staging ) != VK_SUCCESS )
		{ endOneShot( cmd ); return false; }
		VkMemoryRequirements sreq;
		vkGetBufferMemoryRequirements( m_device, staging, &sreq );
		VkMemoryAllocateInfo sai{};
		sai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		sai.allocationSize = sreq.size;
		sai.memoryTypeIndex = findMemoryType( sreq.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		VkDeviceMemory stagingMem;
		if( vkAllocateMemory( m_device, &sai, nullptr, &stagingMem ) != VK_SUCCESS )
		{ endOneShot( cmd ); return false; }
		vkBindBufferMemory( m_device, staging, stagingMem, 0 );

		void* mapped = nullptr;
		vkMapMemory( m_device, stagingMem, 0, dataSize, 0, &mapped );
		if( expand )
		{
			if( wide )
			{
				const uint16_t* src = (const uint16_t*)pixels;
				uint16_t* dst = (uint16_t*)mapped;
				for( uint32_t i = 0; i < w * h; i++ )
				{
					dst[i*4+0] = src[i*3+0];
					dst[i*4+1] = src[i*3+1];
					dst[i*4+2] = src[i*3+2];
					dst[i*4+3] = 65535;
				}
			} else
			{
				const uint8_t* src = (const uint8_t*)pixels;
				uint8_t* dst = (uint8_t*)mapped;
				for( uint32_t i = 0; i < w * h; i++ )
				{
					dst[i*4+0] = src[i*3+0];
					dst[i*4+1] = src[i*3+1];
					dst[i*4+2] = src[i*3+2];
					dst[i*4+3] = 255;
				}
			}
		} else
		{
			memcpy( mapped, pixels, dataSize );
		}
		vkUnmapMemory( m_device, stagingMem );

		VkBufferImageCopy region{};
		region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		region.imageExtent = { w, h, 1 };
		vkCmdCopyBufferToImage( cmd, staging, tex.image,
								VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

		// to SHADER_READ_ONLY (GenerateMipmap re-transitions per level)
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
							  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							  0, 0, nullptr, 0, nullptr, 1, &barrier );

		endOneShot( cmd );
		vkDestroyBuffer( m_device, staging, nullptr );
		vkFreeMemory( m_device, stagingMem, nullptr );
	} else
	{
		endOneShot( cmd );
	}

	return createTextureViewAndSampler( tex );
}

void lsVulkan::TexImage2D( lsEnum target, lsInt level, lsInt internalFormat,
						   lsSizei width, lsSizei height, lsInt border,
						   lsEnum format, lsEnum type, const void* data )
{
	(void)target; (void)border;
	if( level != 0 || width <= 0 || height <= 0 ) return;

	const uint32_t id = m_textureUnits[m_activeUnit];
	auto it = m_textures.find( id );
	if( it == m_textures.end() ) return;

	VORTX_DEBUG_ONLY( lsFrameDump::trackTexImage( id, (int)level,
						(int)internalFormat, (int)width, (int)height,
						(int)format, (int)type, data ); )
	if(! uploadTexture( it->second, (lsEnum)internalFormat, format, type,
						(uint32_t)width, (uint32_t)height, data ) )
		cout << "ERROR: lsVulkan::TexImage2D failed (VkResult " << m_lastError << ")" << endl;
}

void lsVulkan::TexParameteri( lsEnum target, lsEnum pname, lsInt param )
{
	(void)target;
	const uint32_t id = m_textureUnits[m_activeUnit];
	auto it = m_textures.find( id );
	if( it == m_textures.end() ) return;
	VkTex& t = it->second;

	if( pname == LS_TEXTURE_WRAP_S ) t.wrapS = (lsEnum)param;
	else if( pname == LS_TEXTURE_WRAP_T ) t.wrapT = (lsEnum)param;
	else if( pname == LS_TEXTURE_MIN_FILTER ) t.minFilter = (lsEnum)param;
	else if( pname == LS_TEXTURE_MAG_FILTER ) t.magFilter = (lsEnum)param;
	else if( pname == LS_TEXTURE_COMPARE_MODE )
		t.compareEnable = ( param == (lsInt)LS_COMPARE_REF_TO_TEXTURE );
	else if( pname == LS_TEXTURE_COMPARE_FUNC ) {} // LEQUAL only (below)
	else return;

	// recreate the sampler when the image already exists
	if( t.image ) createTextureViewAndSampler( t );
}

void lsVulkan::TexParameterf( lsEnum target, lsEnum pname, lsFloat param )
{
	TexParameteri( target, pname, (lsInt)param );
}

void lsVulkan::TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params )
{
	if( params ) TexParameteri( target, pname, (lsInt)params[0] );
}

void lsVulkan::PixelStorei( lsEnum pname, lsInt param )
{
	(void)pname; (void)param;	// uploads assume tight packing (engine uses alignment 1)
}

void lsVulkan::generateMipmaps( VkTex& tex )
{
	if( tex.mipLevels <= 1 || tex.mipsGenerated ) return;

	VkCommandBuffer cmd = beginOneShot();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = tex.image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipW = (int32_t)tex.width;
	int32_t mipH = (int32_t)tex.height;

	// Layout state on entry (set by uploadTexture):
	//   ALL levels = SHADER_READ_ONLY_OPTIMAL
	// The blit chain below wants destinations in TRANSFER_DST_OPTIMAL —
	// restore that invariant with one bulk transition of levels 1..N
	// (fixes VUID-vkCmdDraw-None-09600).
	barrier.subresourceRange.baseMipLevel = 1;
	barrier.subresourceRange.levelCount = tex.mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
						  0, nullptr, 0, nullptr, 1, &barrier );
	barrier.subresourceRange.levelCount = 1;

	// Per-level transitions:
	//   level i-1: (i==1 ? SHADER_READ_ONLY : TRANSFER_DST) -> TRANSFER_SRC
	//   blit i-1 -> i  (dst level i is now TRANSFER_DST)
	//   level i-1: TRANSFER_SRC -> SHADER_READ_ONLY
	// and finally the last level TRANSFER_DST -> SHADER_READ_ONLY.
	for( uint32_t i = 1; i < tex.mipLevels; i++ )
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = ( i == 1 )
			? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			: VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = ( i == 1 ) ? VK_ACCESS_SHADER_READ_BIT
										   : VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier( cmd,
							  ( i == 1 ) ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
										 : VK_PIPELINE_STAGE_TRANSFER_BIT,
							  VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
							  0, nullptr, 0, nullptr, 1, &barrier );

		VkImageBlit blit{};
		blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1 };
		blit.srcOffsets[1] = { mipW, mipH, 1 };
		blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1 };
		blit.dstOffsets[1] = { std::max( mipW / 2, 1 ), std::max( mipH / 2, 1 ), 1 };
		vkCmdBlitImage( cmd, tex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1, &blit, VK_FILTER_LINEAR );

		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
							  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
							  0, nullptr, 0, nullptr, 1, &barrier );

		mipW = std::max( mipW / 2, 1 );
		mipH = std::max( mipH / 2, 1 );
	}

	// final level was only ever a blit DESTINATION
	barrier.subresourceRange.baseMipLevel = tex.mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
						  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
						  0, nullptr, 0, nullptr, 1, &barrier );

	endOneShot( cmd );

	tex.mipsGenerated = true;
	createTextureViewAndSampler( tex );	// sampler maxLod now spans the chain
}

void lsVulkan::GenerateMipmap( lsEnum target )
{
	(void)target;
	const uint32_t id = m_textureUnits[m_activeUnit];
	auto it = m_textures.find( id );
	if( it == m_textures.end() ) return;
	generateMipmaps( it->second );
}

// ---------------------------------------------------------------------
// Per-draw descriptor set: fresh allocation written with the CURRENT
// GL texture-unit state, then bound immediately. Fresh sets are not yet
// bound to any command buffer, so writing them mid-recording is legal
// (unlike updating an already-bound set, which invalidates the CB).
// ---------------------------------------------------------------------
VkDescriptorSet lsVulkan::allocDrawSet( VkProgramObj& prog )
{
	static const bool s_lightDbg = !!getenv( "VORTX_VK_LIGHT_DEBUG" );
	static int s_n = 0;
	if( s_lightDbg && s_n < 10 && prog.uboStaging.size() >= 800 )
	{
		const float* g = (const float*)prog.uboStaging.data() + 120;
		printf( "PBR DRAW #%d size=%zu: count=%g pos0=(%.2f,%.2f,%.2f) diff0=(%.2f,%.2f,%.2f,%.2f) type0=%g i0=%g\n",
			s_n, prog.uboStaging.size(), g[84], g[0], g[1], g[2], g[12], g[13], g[14], g[15], g[36], g[24] );
		++s_n;
	}
	VkDescriptorSetAllocateInfo dai{};
	dai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dai.descriptorPool = m_drawPools[m_currentFrame];
	dai.descriptorSetCount = 1;
	dai.pSetLayouts = &prog.setLayout;

	VkDescriptorSet set = VK_NULL_HANDLE;
	if( vkAllocateDescriptorSets( m_device, &dai, &set ) != VK_SUCCESS )
		return prog.descriptorSets[m_currentFrame];	// pool exhausted: dummy fallback

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorImageInfo> imgInfos;
	imgInfos.reserve( 16 );

	VkDescriptorBufferInfo uboInfo{};
	uboInfo.buffer = m_uboBuffer;
	uboInfo.offset = 0;
	uboInfo.range = std::max<VkDeviceSize>( prog.layout.blockSize, 16 );
	VkWriteDescriptorSet uw{};
	uw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uw.dstSet = set;
	uw.dstBinding = 0;
	uw.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uw.descriptorCount = 1;
	uw.pBufferInfo = &uboInfo;
	writes.push_back( uw );

	for( size_t k = 0; k < prog.layout.samplers.size(); k++ )
	{
		const uint32_t binding = 1 + (uint32_t)k;

		// GL unit assigned via Uniform1i (default: sampler k -> unit k)
		uint32_t unit = (uint32_t)k;
		auto uit = prog.samplerUnits.find( binding );
		if( uit != prog.samplerUnits.end() ) unit = uit->second;

		const uint32_t texID = ( unit < 32 ) ? m_textureUnits[unit] : 0;

		VkImageView view = m_dummyView;
		VkSampler sampler = m_dummySampler;
		if( texID )
		{
			auto tit = m_textures.find( texID );
			if( tit != m_textures.end() && tit->second.view )
			{
				view = tit->second.view;
				sampler = tit->second.sampler;
			}
		}

		const uint32_t count = ( k < prog.layout.samplerArrays.size() &&
								 prog.layout.samplerArrays[k] > 0 )
			? prog.layout.samplerArrays[k] : 1;
		const size_t base = imgInfos.size();
		for( uint32_t e = 0; e < count; e++ )
		{
			// per-element unit (sampler arrays); falls back to the
			// binding-level unit resolved above
			uint32_t elemUnit = unit;
			if( count > 1 )
			{
				auto eit = prog.samplerUnitsElem.find( binding );
				if( eit != prog.samplerUnitsElem.end() )
				{
					auto uit2 = eit->second.find( e );
					if( uit2 != eit->second.end() ) elemUnit = uit2->second;
				}
			}
			const uint32_t elemTexID =
				( elemUnit < 32 ) ? m_textureUnits[elemUnit] : 0;
			VkImageView eView = view;
			VkSampler eSampler = sampler;
			if( elemTexID != texID )
			{
				eView = m_dummyView;
				eSampler = m_dummySampler;
				if( elemTexID )
				{
					auto tit2 = m_textures.find( elemTexID );
					if( tit2 != m_textures.end() && tit2->second.view )
					{
						eView = tit2->second.view;
						eSampler = tit2->second.sampler;
					}
				}
			}
			VkDescriptorImageInfo ii{};
			ii.sampler = eSampler;
			ii.imageView = eView;
			ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imgInfos.push_back( ii );
		}

		VkWriteDescriptorSet sw{};
		sw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		sw.dstSet = set;
		sw.dstBinding = binding;
		sw.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sw.descriptorCount = count;
		sw.pImageInfo = imgInfos.data() + base;
		writes.push_back( sw );
	}
	vkUpdateDescriptorSets( m_device, (uint32_t)writes.size(), writes.data(),
							0, nullptr );
	return set;
}

// ---------------------------------------------------------------------
// Draw-time sampler binding: GL texture units -> descriptor updates
// (SUPERSEDED by allocDrawSet; kept for reference during V5 bring-up)
// ---------------------------------------------------------------------
void lsVulkan::bindProgramTextures( VkProgramObj& prog )
{
	for( size_t k = 0; k < prog.layout.samplers.size(); k++ )
	{
		const uint32_t binding = 1 + (uint32_t)k;

		// GL unit assigned via Uniform1i (default: sampler k -> unit k)
		uint32_t unit = (uint32_t)k;
		auto uit = prog.samplerUnits.find( binding );
		if( uit != prog.samplerUnits.end() ) unit = uit->second;

		const uint32_t texID = ( unit < 32 ) ? m_textureUnits[unit] : 0;

		// per-frame cache: each frame's set has its own contents
		auto& frameCache = prog.lastBoundTex[m_currentFrame];
		auto lit = frameCache.find( binding );
		if( lit != frameCache.end() && lit->second == texID )
			continue;	// descriptor already points at this texture

		// resolve texture object (0/unknown -> dummy)
		VkImageView view = m_dummyView;
		VkSampler sampler = m_dummySampler;
		if( texID )
		{
			auto tit = m_textures.find( texID );
			if( tit != m_textures.end() && tit->second.view )
			{
				view = tit->second.view;
				sampler = tit->second.sampler;
			}
		}

		const uint32_t count = ( k < prog.layout.samplerArrays.size() &&
								 prog.layout.samplerArrays[k] > 0 )
			? prog.layout.samplerArrays[k] : 1;
		std::vector<VkDescriptorImageInfo>* infos =
			new std::vector<VkDescriptorImageInfo>( count );
		for( auto& info : *infos )
		{
			info.sampler = sampler;
			info.imageView = view;
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		m_descriptorImageInfos.push_back( infos );

		VkWriteDescriptorSet sw{};
		sw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// only the CURRENT frame's set: its previous use was fence-waited
		// in BeginFrame and its command buffer has not been submitted yet
		sw.dstSet = prog.descriptorSets[m_currentFrame];
		sw.dstBinding = binding;
		sw.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sw.descriptorCount = count;
		sw.pImageInfo = infos->data();
		vkUpdateDescriptorSets( m_device, 1, &sw, 0, nullptr );

		frameCache[binding] = texID;
	}
}


// =====================================================================
// V4: offscreen framebuffers + composite — deferred render-pass model
//
// One command buffer per frame, but the render pass instance is opened
// lazily (first draw/clear to a target) and switched when the engine
// binds a different framebuffer: end current pass, begin the new one.
// =====================================================================

bool lsVulkan::createOffscreenRenderPasses()
{
	// Shared variant: color + depth, both transitioning
	// SHADER_READ_ONLY -> attachment -> SHADER_READ_ONLY so the results
	// can be sampled by later passes (composite / shadows).
	{
		VkAttachmentDescription atts[2]{};
		atts[0].format = VK_FORMAT_R8G8B8A8_UNORM;
		atts[0].samples = VK_SAMPLE_COUNT_1_BIT;
		atts[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		atts[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		atts[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		atts[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		atts[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		atts[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		atts[1].format = m_depthFormat;
		atts[1].samples = VK_SAMPLE_COUNT_1_BIT;
		atts[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		atts[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		atts[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		atts[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		atts[1].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		atts[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
		dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
						   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
							VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		ci.attachmentCount = 2;
		ci.pAttachments = atts;
		ci.subpassCount = 1;
		ci.pSubpasses = &subpass;
		ci.dependencyCount = 1;
		ci.pDependencies = &dep;

		m_lastError = vkCreateRenderPass( m_device, &ci, nullptr, &m_rpColorDepth );
		if( m_lastError != VK_SUCCESS ) return false;
	}

	// Depth-only variant (shadow maps, V5)
	{
		VkAttachmentDescription depth{};
		depth.format = m_depthFormat;
		depth.samples = VK_SAMPLE_COUNT_1_BIT;
		depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		depth.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthRef{ 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		subpass.pDepthStencilAttachment = &depthRef;

		VkSubpassDependency dep{};
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dep.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		ci.attachmentCount = 1;
		ci.pAttachments = &depth;
		ci.subpassCount = 1;
		ci.pSubpasses = &subpass;
		ci.dependencyCount = 1;
		ci.pDependencies = &dep;

		m_lastError = vkCreateRenderPass( m_device, &ci, nullptr, &m_rpDepthOnly );
		if( m_lastError != VK_SUCCESS ) return false;
	}
	return true;
}

bool lsVulkan::buildFbo( VkFbo& fbo )
{
	if( fbo.fb ) vkDestroyFramebuffer( m_device, fbo.fb, nullptr );
	fbo.fb = VK_NULL_HANDLE;
	fbo.built = false;

	if( fbo.colorTex && fbo.depthTex ) fbo.variant = 0;
	else if( fbo.depthTex ) fbo.variant = 1;
	else
	{
		cout << "ERROR: lsVulkan::buildFbo - unsupported attachment set "
			 << "(color-only not implemented)" << endl;
		return false;
	}

	auto ct = m_textures.find( fbo.colorTex );
	auto dt = m_textures.find( fbo.depthTex );

	VkImageView atts[2]{};
	VkRenderPass rp = VK_NULL_HANDLE;
	uint32_t count = 0;

	if( fbo.variant == 0 )
	{
		if( ct == m_textures.end() || ! ct->second.view ||
			dt == m_textures.end() || ! dt->second.view ) return false;
		atts[0] = ct->second.view;
		atts[1] = dt->second.view;
		fbo.w = ct->second.width;
		fbo.h = ct->second.height;
		rp = m_rpColorDepth;
		count = 2;
	} else
	{
		if( dt == m_textures.end() || ! dt->second.view ) return false;
		atts[0] = dt->second.view;
		fbo.w = dt->second.width;
		fbo.h = dt->second.height;
		rp = m_rpDepthOnly;
		count = 1;
	}

	VkFramebufferCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = rp;
	ci.attachmentCount = count;
	ci.pAttachments = atts;
	ci.width = fbo.w;
	ci.height = fbo.h;
	ci.layers = 1;

	m_lastError = vkCreateFramebuffer( m_device, &ci, nullptr, &fbo.fb );
	if( m_lastError != VK_SUCCESS ) return false;
	fbo.built = true;
	return true;
}

// ---------------------------------------------------------------------
// Deferred pass management
// ---------------------------------------------------------------------
void lsVulkan::beginPassForTarget( uint32_t fboID )
{
	VkRenderPass rp = m_renderPass;
	VkFramebuffer fb = VK_NULL_HANDLE;
	VkExtent2D extent = m_swapchainExtent;
	bool hasDepth = true;

	if( fboID != 0 )
	{
		auto it = m_fbos.find( fboID );
		if( it == m_fbos.end() ) return;
		VkFbo& fbo = it->second;
		if(! fbo.built && ! buildFbo( fbo ) ) return;
		rp = ( fbo.variant == 0 ) ? m_rpColorDepth : m_rpDepthOnly;
		fb = fbo.fb;
		extent = { fbo.w, fbo.h };
	} else
	{
		fb = m_framebuffers[m_acquiredImage];
	}

	VkClearValue clearValues[2]{};
	clearValues[0].color.float32[0] = m_clearColor[0];
	clearValues[0].color.float32[1] = m_clearColor[1];
	clearValues[0].color.float32[2] = m_clearColor[2];
	clearValues[0].color.float32[3] = m_clearColor[3];
	clearValues[1].depthStencil.depth = (float)m_clearDepth;

	VkRenderPassBeginInfo rpi{};
	rpi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpi.renderPass = rp;
	rpi.framebuffer = fb;
	rpi.renderArea.offset = { 0, 0 };
	rpi.renderArea.extent = extent;
	rpi.clearValueCount = ( fboID != 0 && m_fbos[fboID].variant == 1 ) ? 1 : 2;
	// depth-only FBO: single (depth) clear value at slot 0
	if( fboID != 0 && m_fbos[fboID].variant == 1 )
	{
		clearValues[0].depthStencil.depth = (float)m_clearDepth;
		clearValues[0].depthStencil.stencil = 0;
	}
	rpi.pClearValues = clearValues;

	VkCommandBuffer cmd = m_commandBuffers[m_currentFrame];

#if defined(VORTX_DEBUG_TOOLS)
	// VORTX_PASS_TRACE=1: log the first render-pass begins (target, fb,
	// extent, depth-texture view) — catches passes that never begin,
	// stale image views, and wrong clear values.
	static const bool passTrace = getenv( "VORTX_PASS_TRACE" ) != nullptr;
	static uint32_t passTraceCount = 0;
	if( passTrace && passTraceCount < 40 )
	{
		passTraceCount++;
		cout << "PASSBEGIN fbo=" << fboID << " rp=" << (void*)rp
			 << " fb=" << (void*)fb << " " << extent.width << "x" << extent.height
			 << " nclear=" << rpi.clearValueCount
			 << " clear0.depth=" << rpi.pClearValues[0].depthStencil.depth;
		if( fboID != 0 )
		{
			VkFbo& fbo = m_fbos[fboID];
			auto dt = m_textures.find( fbo.depthTex );
			cout << " depthTex=" << fbo.depthTex << " variant=" << fbo.variant
				 << " texView="
				 << ( dt != m_textures.end() ? (void*)dt->second.view : nullptr );
		}
		cout << endl;
	}
#endif

	vkCmdBeginRenderPass( cmd, &rpi, VK_SUBPASS_CONTENTS_INLINE );

	// viewport: explicit Viewport() call wins, else full target extent.
	// Negative height keeps GL conventions (Y-up NDC).
	float vx = 0.0f, vy = 0.0f, vw = (float)extent.width, vh = (float)extent.height;
	if( m_viewport[2] > 0 && m_viewport[3] > 0 )
	{
		vx = (float)m_viewport[0];
		vy = (float)m_viewport[1];
		vw = (float)m_viewport[2];
		vh = (float)m_viewport[3];
	}
	VkViewport vp{};
	vp.x = vx;
	vp.y = vy + vh;
	vp.width = vw;
	vp.height = -vh;
	vp.minDepth = (float)( ( m_depthRangeN + m_depthRangeF ) * 0.5 );
	vp.maxDepth = (float)( vp.minDepth + ( m_depthRangeF - m_depthRangeN ) * 0.5 );
	vkCmdSetViewport( cmd, 0, 1, &vp );

	VkRect2D sc{};
	sc.offset = { 0, 0 };
	sc.extent = extent;
	vkCmdSetScissor( cmd, 0, 1, &sc );

	m_activeRenderPass = rp;
	m_activeTarget = fboID;
	m_passActive = true;
	(void)hasDepth;

	// DIAGNOSTIC: pass-open state for the swapchain target
#if defined(VORTX_DEBUG_TOOLS)
	if( getenv( "VORTX_VK_DEBUG" ) && fboID == 0 )
	{
		static int s_passDump = 0;
		if( s_passDump++ < 4 )
		{
			cout << "SWPASS open: extent=" << extent.width << "x" << extent.height
				 << " m_viewport=[" << m_viewport[0] << "," << m_viewport[1]
				 << "," << m_viewport[2] << "," << m_viewport[3] << "] units:";
			for( int u = 0; u < 8; u++ ) cout << " " << m_textureUnits[u];
			cout << endl;
		}
	}
#endif
}

void lsVulkan::endActivePass()
{
	if(! m_passActive ) return;
	vkCmdEndRenderPass( m_commandBuffers[m_currentFrame] );
	m_passActive = false;
	m_activeRenderPass = VK_NULL_HANDLE;
}

void lsVulkan::ensurePassActive()
{
	if(! m_inFrame || m_passActive ) return;
	beginPassForTarget( m_boundFBO );
}

// ---------------------------------------------------------------------
// Framebuffer objects (GL API surface)
// ---------------------------------------------------------------------
void lsVulkan::GenFramebuffers( lsSizei n, lsFBOID* framebuffers )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		uint32_t id = m_nextFboID++;
		m_fbos[id] = VkFbo{};
		framebuffers[i] = id;
	}
}

void lsVulkan::DeleteFramebuffers( lsSizei n, lsFBOID* framebuffers )
{
	for( lsSizei i = 0; i < n; i++ )
	{
		auto it = m_fbos.find( framebuffers[i] );
		if( it == m_fbos.end() ) continue;
		if( it->second.fb ) retireFbo( it->second.fb );
		m_fbos.erase( it );
	}
}

void lsVulkan::BindFramebuffer( lsEnum target, lsFBOID fbo )
{
	(void)target;
	m_boundFBO = fbo;

	// Mid-frame target switch: close the current pass; the new one opens
	// lazily on the next draw/clear (ensurePassActive).
	if( m_inFrame )
	{
		if( fbo == m_activeTarget && m_passActive ) return;
		endActivePass();
	}
}

void lsVulkan::FramebufferTexture2D( lsEnum target, lsEnum attachment,
									 lsEnum texTarget, lsTextureID texture,
									 lsInt level )
{
	(void)target; (void)texTarget; (void)level;
	auto it = m_fbos.find( m_boundFBO );
	if( it == m_fbos.end() ) return;

	if( attachment == LS_COLOR_ATTACHMENT0 )
		it->second.colorTex = texture;
	else if( attachment == LS_DEPTH_ATTACHMENT )
		it->second.depthTex = texture;
	else
		return;
	it->second.built = false;	// attachments changed: rebuild on next use
}

lsEnum lsVulkan::CheckFramebufferStatus( lsEnum target )
{
	(void)target;
	auto it = m_fbos.find( m_boundFBO );
	if( it == m_fbos.end() ) return 0;
	if(! it->second.built )
		if(! buildFbo( it->second ) ) return 0;
	return LS_FRAMEBUFFER_COMPLETE;
}

void lsVulkan::DrawBuffer( lsEnum mode )
{
	(void)mode;		// single color attachment per FBO — nothing to select
}

void lsVulkan::ReadBuffer( lsEnum mode )
{
	(void)mode;
}


// =====================================================================
// STUBS — not yet implemented (V5+ milestones)
// =====================================================================

void lsVulkan::ReadPixels( lsInt, lsInt, lsSizei, lsSizei, lsEnum, lsEnum, void* )
{
	// V5+ stub
}

void lsVulkan::DepthRangef( lsFloat near, lsFloat far )
{
	// FIX 2026-08-11 (Vulkan dark floor, 3/4 — see Plan.md "D1").
	// GL->VK depth-range translation at the backend boundary.
	// GL: z_w = (f-n)/2 * z_ndc + (n+f)/2  (z_ndc in [-1,1])
	// VK: z_f = (max-min) * z_ndc + min    (applied in Viewport)
	// Storing GL (n,f); the viewport transform makes VK depth content
	// match GL conventions exactly, so GL-convention shadow bias
	// matrices (sMatrices) compare correctly on both backends.
	m_depthRangeN = near;
	m_depthRangeF = far;
}

void lsVulkan::ColorMask( lsBoolean, lsBoolean, lsBoolean, lsBoolean )
{
	// V5+ stub
}

void lsVulkan::PolygonOffset( lsFloat factor, lsFloat units )
{
	// L1.2: GL polygon offset maps to Vulkan depth bias
	// (slopeFactor = factor, constantFactor = units)
	m_polyOffsetFactor = factor;
	m_polyOffsetUnits = units;
}

void lsVulkan::ShadeModel( lsEnum )
{
	// V5+ stub
}

void lsVulkan::GenQueries( lsSizei, lsQueryID* )
{
	// V5+ stub
}

void lsVulkan::DeleteQueries( lsSizei, lsQueryID* )
{
	// V5+ stub
}

void lsVulkan::BeginQuery( lsEnum, lsQueryID )
{
	// V5+ stub
}

void lsVulkan::EndQuery( lsEnum )
{
	// V5+ stub
}

void lsVulkan::GetQueryObjectiv( lsQueryID, lsEnum, lsInt* )
{
	// V5+ stub
}

void lsVulkan::GetIntegerv( lsEnum, lsInt* )
{
	// V5+ stub
}

// ---------------------------------------------------------------------
// DIAGNOSTIC: swapchain readback (VORTX_SHOT=path.ppm, frame 30)
// ---------------------------------------------------------------------
// DIAGNOSTIC: read any color image back to a PPM (transitions out of and
// back into `layout`)
bool lsVulkan::readbackImage( VkImage img, VkImageLayout layout,
							  uint32_t w, uint32_t h, bool isBGR,
							  const char* path )
{
	vkDeviceWaitIdle( m_device );

	const VkDeviceSize size = (VkDeviceSize)w * h * 4;

	VkBufferCreateInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkBuffer buf;
	if( vkCreateBuffer( m_device, &bi, nullptr, &buf ) != VK_SUCCESS ) return false;
	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements( m_device, buf, &reqs );
	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = reqs.size;
	ai.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	VkDeviceMemory mem;
	if( vkAllocateMemory( m_device, &ai, nullptr, &mem ) != VK_SUCCESS ) return false;
	vkBindBufferMemory( m_device, buf, mem, 0 );

	VkCommandBuffer cmd = beginOneShot();

	VkImageMemoryBarrier b{};
	b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = img;
	b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	b.oldLayout = layout;
	b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	b.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	b.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
						  0, nullptr, 1, &b );

	VkBufferImageCopy copy{};
	copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copy.imageExtent = { w, h, 1 };
	vkCmdCopyImageToBuffer( cmd, img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							buf, 1, &copy );

	b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	b.newLayout = layout;
	b.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
						  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
						  0, nullptr, 1, &b );
	endOneShot( cmd );

	bool ok = false;
	void* mapped = nullptr;
	vkMapMemory( m_device, mem, 0, size, 0, &mapped );
	if( mapped )
	{
		const uint8_t* px = (const uint8_t*)mapped;
		// 4-channel readback -> packed top-down RGB for saveRGB
		// (B8G8R8A8 swapchains need B/R swapped; R8G8B8A8 don't)
		std::vector<uint8_t> rgb( (size_t)w * h * 3 );
		for( uint32_t y = 0; y < h; y++ )
		{
			const uint8_t* src = px + (size_t)y * w * 4;
			uint8_t* dst = rgb.data() + (size_t)y * w * 3;
			for( uint32_t x = 0; x < w; x++ )
			{
				dst[x*3+0] = src[x*4 + (isBGR?2:0)];
				dst[x*3+1] = src[x*4 + 1];
				dst[x*3+2] = src[x*4 + (isBGR?0:2)];
			}
		}
		ok = lsScreenshot::saveRGB( path, (int)w, (int)h, rgb.data() );
	}
	vkUnmapMemory( m_device, mem );
	vkDestroyBuffer( m_device, buf, nullptr );
	vkFreeMemory( m_device, mem, nullptr );
	return ok;
}

bool lsVulkan::Screenshot( const char* path )
{
	return readbackImage( m_swapchainImages[m_acquiredImage],
				   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				   m_swapchainExtent.width, m_swapchainExtent.height,
				   true, path );
}

// DIAGNOSTIC: read back an FBO's colour attachment (VORTX_FBO_SHOT=id:path)
void lsVulkan::captureFbo( uint32_t fboID, const char* path )
{
	auto fit = m_fbos.find( fboID );
	if( fit == m_fbos.end() || ! fit->second.built || ! fit->second.colorTex )
	{
		cout << "captureFbo: FBO " << fboID << " not found; m_fbos:";
		for( auto& [id, f] : m_fbos )
			cout << " [" << id << " built=" << f.built
				 << " colorTex=" << f.colorTex << " " << f.w << "x" << f.h << "]";
		cout << endl;
		return;
	}
	auto tit = m_textures.find( fit->second.colorTex );
	if( tit == m_textures.end() || ! tit->second.image )
	{ cout << "captureFbo: FBO " << fboID << " has no colour image" << endl; return; }
	readbackImage( tit->second.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				   tit->second.width, tit->second.height, false, path );
}

// DIAGNOSTIC (D-series): read back a depth image (D32_SFLOAT) as grayscale
bool lsVulkan::readbackDepthImage( VkImage img, VkImageLayout layout,
								   uint32_t w, uint32_t h, const char* path )
{
	vkDeviceWaitIdle( m_device );

	const VkDeviceSize size = (VkDeviceSize)w * h * 4;

	VkBufferCreateInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkBuffer buf;
	if( vkCreateBuffer( m_device, &bi, nullptr, &buf ) != VK_SUCCESS ) return false;
	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements( m_device, buf, &reqs );
	VkMemoryAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = reqs.size;
	ai.memoryTypeIndex = findMemoryType( reqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	VkDeviceMemory mem;
	if( vkAllocateMemory( m_device, &ai, nullptr, &mem ) != VK_SUCCESS ) return false;
	vkBindBufferMemory( m_device, buf, mem, 0 );

	VkCommandBuffer cmd = beginOneShot();

	VkImageMemoryBarrier b{};
	b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = img;
	b.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
	b.oldLayout = layout;
	b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	b.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	b.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						  VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
						  0, nullptr, 1, &b );

	VkBufferImageCopy copy{};
	copy.imageSubresource = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1 };
	copy.imageExtent = { w, h, 1 };
	vkCmdCopyImageToBuffer( cmd, img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							buf, 1, &copy );

	b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	b.newLayout = layout;
	b.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	b.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
						  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr,
						  0, nullptr, 1, &b );
	endOneShot( cmd );

	bool ok = false;
	void* mapped = nullptr;
	vkMapMemory( m_device, mem, 0, size, 0, &mapped );
	if( mapped )
	{
		// D32 float depth -> grayscale with auto-contrast stretch.
		// DIRECT-light ortho maps often cluster near 1.0; raw 8-bit
		// quantization would hide real silhouettes, so report min/max
		// and stretch the occupied range to full contrast.
		const float* px = (const float*)mapped;
		float dmin = 1e30f, dmax = -1e30f;
		for( uint32_t i = 0; i < w * h; i++ )
		{
			const float d = px[i];
			if( d < dmin ) dmin = d;
			if( d > dmax ) dmax = d;
		}
		cout << "readbackDepthImage " << path << ": depth min=" << dmin
			 << " max=" << dmax << " (uniform if min==max)" << endl;
		const float range = ( dmax - dmin > 1e-6f ) ? ( dmax - dmin ) : 1.0f;
		std::vector<uint8_t> rgb( (size_t)w * h * 3 );
		for( uint32_t i = 0; i < w * h; i++ )
		{
			float d = ( px[i] - dmin ) / range; // stretched 0..1
			if( d < 0.0f ) d = 0.0f;
			if( d > 1.0f ) d = 1.0f;
			const uint8_t g = (uint8_t)( d * 255.0f );
			rgb[i*3+0] = g; rgb[i*3+1] = g; rgb[i*3+2] = g;
		}
		ok = lsScreenshot::saveRGB( path, (int)w, (int)h, rgb.data() );
	}
	vkUnmapMemory( m_device, mem );
	vkDestroyBuffer( m_device, buf, nullptr );
	vkFreeMemory( m_device, mem, nullptr );
	return ok;
}

// DIAGNOSTIC (D-series): read back an FBO's DEPTH attachment (VORTX_DEPTH_SHOT=id:path)
void lsVulkan::captureFboDepth( uint32_t fboID, const char* path )
{
	auto fit = m_fbos.find( fboID );
	if( fit == m_fbos.end() || ! fit->second.built || ! fit->second.depthTex )
	{
		cout << "captureFboDepth: FBO " << fboID << " not found; m_fbos:";
		for( auto& [id, f] : m_fbos )
			cout << " [" << id << " built=" << f.built
				 << " depthTex=" << f.depthTex << " " << f.w << "x" << f.h << "]";
		cout << endl;
		return;
	}
	auto tit = m_textures.find( fit->second.depthTex );
	if( tit == m_textures.end() || ! tit->second.image )
	{ cout << "captureFboDepth: FBO " << fboID << " has no depth image" << endl; return; }
	readbackDepthImage( tit->second.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						tit->second.width, tit->second.height, path );
}
