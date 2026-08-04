/**
 * @file lsVulkan.h
 * @brief Vulkan implementation of the lsRender interface (PRIMARY backend)
 *
 * Status: V1 (Plan.md Part 2) — windowed rendering core: instance,
 * surface, device, swapchain, render pass, per-frame sync, Clear +
 * Present. All draw/buffer/texture/shader methods remain stubs until
 * V2+.
 *
 * Design rules (Plan.md Part 2):
 * - LS_* constants keep their GL numeric values; this backend maps
 *   them to Vk* enums at its boundary.
 * - <vulkan/vulkan.h> is included ONLY here and in lsVulkan.cpp
 *   (same rule as <GL/glew.h> for lsOpenGL).
 */
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "lsRender.h"

/**
 * @class lsVulkan
 * @brief Concrete Vulkan backend (primary renderer).
 */
class lsVulkan : public lsRender
{
public:
	lsVulkan();
	virtual ~lsVulkan();

	/**
	 * @brief Cheap availability probe — can we create an instance and
	 * find a physical device? Used by vortxGE::PreferredRendererType()
	 * BEFORE the window is created (the window type depends on it).
	 */
	static bool Probe();

	bool Init(  ) override;
	void SetWindow( void* nativeWindow ) override;
	void BeginFrame(  ) override;
	void EndFrame(  ) override;
	void Present(  ) override;
	void Resize( int width, int height ) override;
	void CheckErrors( const string& name ) override;
	void StoreDefaultFramebuffer(  ) override;
	void RestoreDefaultFramebuffer(  ) override;
	void GenTextures( lsSizei n, lsTextureID* textures ) override;
	void DeleteTextures( lsSizei n, lsTextureID* textures ) override;
	void BindTexture( lsEnum target, lsTextureID texture ) override;
	void ActiveTexture( lsEnum texture ) override;
	void TexImage2D( lsEnum target, lsInt level, lsInt internalformat, lsSizei width, lsSizei height, lsInt border, lsEnum format, lsEnum type, const void* data ) override;
	void TexParameteri( lsEnum target, lsEnum pname, lsInt param ) override;
	void TexParameterf( lsEnum target, lsEnum pname, lsFloat param ) override;
	void TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params ) override;
	void GenerateMipmap( lsEnum target ) override;
	void PixelStorei( lsEnum pname, lsInt param ) override;
	void GenFramebuffers( lsSizei n, lsFBOID* fbos ) override;
	void DeleteFramebuffers( lsSizei n, lsFBOID* fbos ) override;
	void BindFramebuffer( lsEnum target, lsFBOID fbo ) override;
	void FramebufferTexture2D( lsEnum target, lsEnum attachment, lsEnum textarget, lsTextureID texture, lsInt level ) override;
	lsEnum CheckFramebufferStatus( lsEnum target ) override;
	void DrawBuffer( lsEnum mode ) override;
	void ReadBuffer( lsEnum mode ) override;
	void ReadPixels( lsInt x, lsInt y, lsSizei width, lsSizei height, lsEnum format, lsEnum type, void* data ) override;
	void GenBuffers( lsSizei n, lsBufferID* buffers ) override;
	void DeleteBuffers( lsSizei n, lsBufferID* buffers ) override;
	void BindBuffer( lsEnum target, lsBufferID buffer ) override;
	void BufferData( lsEnum target, lsSizeiptr size, const void* data, lsEnum usage ) override;
	void GenVertexArrays( lsSizei n, lsVAOID* arrays ) override;
	void DeleteVertexArrays( lsSizei n, lsVAOID* arrays ) override;
	void BindVertexArray( lsVAOID vao ) override;
	void EnableVertexAttribArray( lsUInt index ) override;
	void VertexAttribPointer( lsUInt index, lsInt size, lsEnum type, lsBoolean normalized, lsSizei stride, const void* pointer ) override;
	void DrawArrays( lsEnum mode, lsInt first, lsSizei count ) override;
	void DrawElements( lsEnum mode, lsSizei count, lsEnum type, const void* indices ) override;
	lsShaderID CreateShader( lsEnum type ) override;
	lsProgramID CreateProgram(  ) override;
	void ShaderSource( lsShaderID shader, lsSizei count, const lsChar** string, const lsInt* length ) override;
	void CompileShader( lsShaderID shader ) override;
	void AttachShader( lsProgramID program, lsShaderID shader ) override;
	void DetachShader( lsProgramID program, lsShaderID shader ) override;
	void LinkProgram( lsProgramID program ) override;
	void UseProgram( lsProgramID program ) override;
	void DeleteShader( lsShaderID shader ) override;
	void DeleteProgram( lsProgramID program ) override;
	lsUniformID GetUniformLocation( lsProgramID program, const lsChar* name ) override;
	void GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params ) override;
	void GetShaderInfoLog( lsShaderID shader, lsSizei bufSize, lsSizei* length, lsChar* infoLog ) override;
	void GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params ) override;
	void GetProgramInfoLog( lsProgramID program, lsSizei bufSize, lsSizei* length, lsChar* infoLog ) override;
	void Uniform1i( lsUniformID location, lsInt v0 ) override;
	void Uniform1f( lsUniformID location, lsFloat v0 ) override;
	void Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 ) override;
	void Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2, lsFloat v3 ) override;
	void UniformMatrix3fv( lsUniformID location, lsSizei count, lsBoolean transpose, const lsFloat* value ) override;
	void UniformMatrix4fv( lsUniformID location, lsSizei count, lsBoolean transpose, const lsFloat* value ) override;
	void Enable( lsEnum cap ) override;
	void Disable( lsEnum cap ) override;
	void CullFace( lsEnum mode ) override;
	void BlendFunc( lsEnum sfactor, lsEnum dfactor ) override;
	void DepthFunc( lsEnum func ) override;
	void DepthMask( lsBoolean flag ) override;
	void DepthRangef( lsFloat near, lsFloat far ) override;
	void ColorMask( lsBoolean r, lsBoolean g, lsBoolean b, lsBoolean a ) override;
	void PolygonOffset( lsFloat factor, lsFloat units ) override;
	void ShadeModel( lsEnum mode ) override;
	void Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h ) override;
	void Clear( lsBitfield mask ) override;
	void ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a ) override;
	void ClearDepth( lsDouble d ) override;
	void GenQueries( lsSizei n, lsQueryID* ids ) override;
	void DeleteQueries( lsSizei n, lsQueryID* ids ) override;
	void BeginQuery( lsEnum target, lsQueryID id ) override;
	void EndQuery( lsEnum target ) override;
	void GetQueryObjectiv( lsQueryID id, lsEnum pname, lsInt* params ) override;
	lsEnum GetError(  ) override;
	void GetIntegerv( lsEnum pname, lsInt* params ) override;

private:
	// -----------------------------------------------------------------
	// V1: init helpers
	// -----------------------------------------------------------------
	bool createInstance();
	bool createSurface();
	bool pickPhysicalDevice();
	bool createDevice();
	bool createSwapchain( VkSwapchainKHR oldSwapchain );
	bool createImageViews();
	bool createRenderPass();
	bool createFramebuffers();
	bool createCommandPoolAndBuffers();
	bool createSyncObjects();
	void cleanupSwapchain();
	bool recreateSwapchain();

	void* m_window = nullptr;			// GLFWwindow*
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_physDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	uint32_t m_graphicsQueueFamily = 0;
	uint32_t m_presentQueueFamily = 0;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkFormat m_swapchainFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swapchainExtent = { 0, 0 };
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> m_framebuffers;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	static constexpr uint32_t FRAMES_IN_FLIGHT = 2;
	VkCommandBuffer m_commandBuffers[FRAMES_IN_FLIGHT] = {};
	VkSemaphore m_imageAvailable[FRAMES_IN_FLIGHT] = {};
	VkSemaphore m_renderFinished[FRAMES_IN_FLIGHT] = {};
	VkFence m_inFlight[FRAMES_IN_FLIGHT] = {};
	uint32_t m_currentFrame = 0;
	uint32_t m_acquiredImage = 0;
	bool m_inFrame = false;
	bool m_swapchainDirty = false;
	float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	double m_clearDepth = 1.0;
	int m_viewport[4] = { 0, 0, 0, 0 };		// x, y, w, h (0/0 = full swapchain)
	VkResult m_lastError = VK_SUCCESS;
	bool m_initialized = false;
};
