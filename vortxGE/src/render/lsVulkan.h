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
#include <unordered_map>
#include <string>

#include "lsVulkanShader.h"

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
	void TexStorage2D( lsEnum target, lsSizei levels, lsEnum internalformat,
					   lsSizei width, lsSizei height ) override;
	lsTextureID CreateDepthRawView( lsTextureID src ) override;
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
	std::vector<VkSemaphore> m_renderFinished;	// PER SWAPCHAIN IMAGE (present wait not fence-covered)
	VkFence m_inFlight[FRAMES_IN_FLIGHT] = {};
	uint32_t m_currentFrame = 0;

#if defined(VORTX_DEBUG_TOOLS)
	// D3 profiler: per-slot timestamp query pools (2 slots per stage,
	// 2-frame readback latency so the fence wait guarantees results).
	void profilerFrameBegin() override;
	void profilerGpuBegin( const std::string& name ) override;
	void profilerGpuEnd( const std::string& name ) override;
	VkQueryPool m_profPool[FRAMES_IN_FLIGHT] = {};
	std::unordered_map<std::string, uint32_t> m_profSlots; // name -> first slot
	std::vector<std::string> m_profActive[FRAMES_IN_FLIGHT];
	uint32_t m_profNext = 0;
	float m_profPeriodNs = 0.0f;
	static constexpr uint32_t PROF_SLOTS = 128;
	void profilerEnsurePool();
#endif
	uint32_t m_acquiredImage = 0;
	bool m_inFrame = false;
	bool m_swapchainDirty = false;
	float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	double m_clearDepth = 1.0;
	double m_depthRangeN = 0.0, m_depthRangeF = 1.0;	// GL-convention depth range
	int m_viewport[4] = { 0, 0, 0, 0 };		// x, y, w, h (0/0 = full swapchain)
	VkResult m_lastError = VK_SUCCESS;
	bool m_initialized = false;

	// -----------------------------------------------------------------
	// V2: buffers, VAOs, shaders, programs, pipelines, descriptors
	// -----------------------------------------------------------------
	struct VkBuf
	{
		VkBuffer		buffer = VK_NULL_HANDLE;
		VkDeviceMemory	memory = VK_NULL_HANDLE;
		VkDeviceSize	size = 0;
	};
	struct VkVaoAttrib
	{
		uint32_t	buffer = 0;		// lsBufferID captured at VertexAttribPointer time
		lsInt		size = 3;
		lsEnum		type = 0;
		lsBoolean	normalized = 0;
		lsSizei		stride = 0;
		size_t		offset = 0;
		bool		enabled = false;
	};
	struct VkVao
	{
		std::unordered_map<uint32_t, VkVaoAttrib> attribs;	// key: attrib index
		uint32_t elementBuffer = 0;
	};
	struct VkShaderObj
	{
		int				stage = 0;			// 0 = vertex, 1 = fragment
		std::string		source;
		lsVkStageParse	parsed;
		bool			hasSource = false;
	};
	struct VkProgramObj
	{
		uint32_t		vs = 0, fs = 0;
		VkShaderModule	vsModule = VK_NULL_HANDLE;
		VkShaderModule	fsModule = VK_NULL_HANDLE;
		lsVkProgramLayout layout;
		std::unordered_map<std::string, uint32_t> uniformIndex;	// name -> layout.members idx
		std::unordered_map<std::string, uint32_t> samplerBinding; // name -> descriptor binding
		std::vector<uint8_t> uboStaging;					// CPU-side uniform block image
		VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
		VkPipelineLayout	  pipelineLayout = VK_NULL_HANDLE;
		// one set per frame in flight: a set may only be updated when no
		// pending command buffer references it (fence-waited in BeginFrame)
		VkDescriptorSet		  descriptorSets[FRAMES_IN_FLIGHT] = {};
		bool				linked = false;
		std::string			log;
		// V3: sampler binding -> GL texture unit (from Uniform1i), and
		// binding -> last texture ID written to the descriptor
		std::unordered_map<uint32_t, uint32_t> samplerUnits;
		// sampler ARRAY element units: binding -> element -> GL texture unit
		// (tex_shadowmaps[0..2] etc. — one descriptor binding, N elements)
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> samplerUnitsElem;
		std::unordered_map<uint32_t, uint32_t> lastBoundTex[FRAMES_IN_FLIGHT];
		std::vector<uint32_t> vsInputLocations;	// all vertex-shader input locations (assembled source scan)
	};

	// V3: texture object
	struct VkTex
	{
		VkImage			image = VK_NULL_HANDLE;
		VkDeviceMemory	memory = VK_NULL_HANDLE;
		VkImageView		view = VK_NULL_HANDLE;
		VkSampler		sampler = VK_NULL_HANDLE;
		VkFormat		format = VK_FORMAT_R8G8B8A8_UNORM;
		uint32_t		width = 0, height = 0;
		uint32_t		mipLevels = 1;
		bool			mipsGenerated = false;
		// GL parameter state (sampler recreated on change)
		lsEnum	wrapS = 0x2901;		// LS_REPEAT
		lsEnum	wrapT = 0x2901;
		lsEnum	minFilter = 0x2601;	// LS_LINEAR
		lsEnum	magFilter = 0x2601;
		bool	compareEnable = false;	// L1.3: shadow-map depth compare
		bool	isCube = false;			// L3.2: cube map (6-layer image, cube view)
		lsEnum	wrapR = 0x2901;		// LS_REPEAT (cube maps clamp to edge)
		bool	sharedView = false;		// L4.1: raw-depth alias — owns only the sampler
	};

	bool createBufferStorage( VkBuf& buf, VkDeviceSize size, const void* data );
	uint32_t findMemoryType( uint32_t typeBits, VkMemoryPropertyFlags props );
	bool linkProgramResources( VkProgramObj& prog );
	VkPipeline getPipeline( VkProgramObj& prog, const VkVao& vao, lsEnum topologyMode );
	bool createDepthResources();
	void destroyDepthResources();
	bool createDummyTexture();
	void writeUniformMember( VkProgramObj& prog, lsUniformID location,
							const void* data, uint32_t bytes );
	uint32_t allocUboSlice( uint32_t bytes );

	// V4: offscreen framebuffer object
	struct VkFbo
	{
		uint32_t	colorTex = 0;			// lsTextureID attachments
		uint32_t	depthTex = 0;
		VkFramebuffer fb = VK_NULL_HANDLE;
		int			variant = -1;			// 0=color+depth, 1=depth-only
		uint32_t	w = 0, h = 0;
		bool		built = false;
		int			depthLayer = -1;			// L3.2: >=0 = cube face layer (no V-flip)
		VkImageView	depthLayerView = VK_NULL_HANDLE;	// per-face 2D view
	};

	// V3 helpers
	bool uploadTexture( VkTex& tex, lsEnum internalFormat, lsEnum format,
						lsEnum type, uint32_t w, uint32_t h, const void* pixels );
	bool createTextureViewAndSampler( VkTex& tex );
	VkCommandBuffer beginOneShot();
	void endOneShot( VkCommandBuffer cmd );
	void bindProgramTextures( VkProgramObj& prog );
	VkDescriptorSet allocDrawSet( VkProgramObj& prog );
	void generateMipmaps( VkTex& tex );

	// V4 helpers
	bool createOffscreenRenderPasses();
	bool buildFbo( VkFbo& fbo );
	void ensurePassActive();
	/// Builtin screenshot: read back the last presented swapchain image.
	bool Screenshot( const char* path ) override;
	void captureFbo( uint32_t fboID, const char* path );	// DIAGNOSTIC
	void captureFboDepth( uint32_t fboID, const char* path );	// DIAGNOSTIC
	bool readbackDepthImage( VkImage img, VkImageLayout layout, uint32_t w,
							 uint32_t h, const char* path );	// DIAGNOSTIC
	bool readbackImage( VkImage img, VkImageLayout layout, uint32_t w,
						uint32_t h, bool isBGR, const char* path );	// DIAGNOSTIC
	void beginPassForTarget( uint32_t fboID );	// 0 = swapchain
	void endActivePass();
#if defined(VORTX_DEBUG_TOOLS)
	/// D2 diagnostic: snapshot this draw into lsFrameDump from the
	/// backend's INTERNAL state (uboStaging, sampler maps, tex units).
	void dumpDraw( lsEnum mode, lsSizei count, uint32_t first, bool indexed );
#endif

	std::unordered_map<uint32_t, VkBuf>		m_buffers;
	std::unordered_map<uint32_t, VkVao>		m_vaos;
	std::unordered_map<uint32_t, VkShaderObj>	m_shaders;
	std::unordered_map<uint32_t, VkProgramObj>	m_programs;
	std::unordered_map<uint32_t, VkTex>		m_textures;
	uint32_t m_nextTexID = 1;
	uint32_t m_textureUnits[32] = {};		// texture ID bound per unit (0 = none)
	uint32_t m_activeUnit = 0;

	// V4: FBOs + deferred render-pass state
	std::unordered_map<uint32_t, VkFbo> m_fbos;
	uint32_t m_nextFboID = 1;
	uint32_t m_boundFBO = 0;				// GL-side bound framebuffer (0 = default)
	uint32_t m_activeTarget = 0;			// target of the active pass (0 = swapchain)
	bool m_passActive = false;
	VkRenderPass m_activeRenderPass = VK_NULL_HANDLE;
	VkRenderPass m_rpColorDepth = VK_NULL_HANDLE;	// offscreen variants
	VkRenderPass m_rpDepthOnly = VK_NULL_HANDLE;
	uint32_t m_savedFBO = 0;				// StoreDefaultFramebuffer
	uint32_t m_nextBufferID = 1, m_nextVaoID = 1;
	uint32_t m_nextShaderID = 1, m_nextProgramID = 1;
	uint32_t m_boundArrayBuffer = 0;			// lsBufferID bound to LS_ARRAY_BUFFER
	uint32_t m_boundElementBuffer = 0;
	uint32_t m_currentVao = 0;
	uint32_t m_currentProgram = 0;

	// GL-style state cache (baked into pipelines at draw time)
	bool	m_depthTestEnabled = false;
	bool	m_depthWrite = true;
	lsEnum	m_depthFunc = 0x0203;			// LS_LEQUAL
	bool	m_cullEnabled = false;
	lsEnum	m_cullFace = 0x0405;			// LS_BACK
	bool	m_blendEnabled = false;
	lsEnum	m_blendSrc = 0x0302;			// LS_SRC_ALPHA
	lsEnum	m_blendDst = 0x0303;			// LS_ONE_MINUS_SRC_ALPHA
	bool	m_polyOffsetEnabled = false;	// L1.2: LS_POLYGON_OFFSET_FILL
	float	m_polyOffsetFactor = 0.0f;	// glPolygonOffset(factor, units)
	float	m_polyOffsetUnits = 0.0f;	// -> VK depthBias slope/constant

	// per-frame uniform upload ring (dynamic UBO)
	VkBuffer		m_uboBuffer = VK_NULL_HANDLE;
	VkDeviceMemory	m_uboMemory = VK_NULL_HANDLE;
	void*			m_uboMapped = nullptr;
	VkDeviceSize	m_uboCursor = 0;
	VkDeviceSize	m_uboAlignment = 256;
	static constexpr VkDeviceSize UBO_RING_SIZE = 1024 * 1024;

	// descriptor pool (shared)
	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	// per-frame pools for PER-DRAW descriptor sets: fresh sets are written
	// and bound immediately, so texture state matches GL's per-draw binding
	// semantics exactly (pool reset at BeginFrame after the fence wait)
	VkDescriptorPool m_drawPools[FRAMES_IN_FLIGHT] = {};

	// Deferred destruction: GL delete calls map to retire buckets that are
	// only destroyed after the owning frame slot's fence has been waited.
	// Freeing GPU memory while an in-flight command buffer still references
	// it is a GPU page fault (RADV reset -> hard crash).
	struct RetiredBuffer { VkBuffer buffer; VkDeviceMemory memory; };
	struct RetiredImage  { VkImage image;  VkDeviceMemory memory; };
	std::vector<RetiredBuffer> m_retiredBufs[FRAMES_IN_FLIGHT];
	std::vector<RetiredImage>  m_retiredImgs[FRAMES_IN_FLIGHT];
	std::vector<VkImageView>   m_retiredViews[FRAMES_IN_FLIGHT];
	std::vector<VkSampler>     m_retiredSamplers[FRAMES_IN_FLIGHT];
	std::vector<VkFramebuffer> m_retiredFbos[FRAMES_IN_FLIGHT];

	// GL vertex-attrib default: a shader input with no enabled attrib array
	// reads (0,0,0,1). Emulated with a stride-0 INSTANCE-rate dummy binding
	// fed by this 16-byte buffer (VUID-...-Input-07904).
	static constexpr uint32_t DUMMY_ATTRIB_BINDING = 15;
	VkBuffer		m_dummyAttribBuffer = VK_NULL_HANDLE;
	VkDeviceMemory	m_dummyAttribMemory = VK_NULL_HANDLE;
	void ensureDummyAttribBuffer();
	void retireBuffer( VkBuffer b, VkDeviceMemory m );
	void retireImage( VkImage i, VkDeviceMemory m );
	void retireView( VkImageView v );
	void retireSampler( VkSampler s );
	void retireFbo( VkFramebuffer f );
	void drainRetired( uint32_t slot );

	// dummy 1x1 white texture for all sampler bindings (real uploads in V3)
	VkImage		m_dummyImage = VK_NULL_HANDLE;
	VkDeviceMemory	m_dummyMemory = VK_NULL_HANDLE;
	VkImageView		m_dummyView = VK_NULL_HANDLE;
	VkSampler		m_dummySampler = VK_NULL_HANDLE;

	// depth attachment for the window render pass
	VkImage		m_depthImage = VK_NULL_HANDLE;
	VkDeviceMemory	m_depthMemory = VK_NULL_HANDLE;
	VkImageView		m_depthView = VK_NULL_HANDLE;
	VkFormat		m_depthFormat = VK_FORMAT_D32_SFLOAT;

	// pipeline cache
	struct PipelineEntry { VkPipeline pipeline = VK_NULL_HANDLE; };
	std::unordered_map<std::string, VkPipeline> m_pipelineCache;

	// growth diagnostics (progressive-slowdown hunt)
	uint32_t m_swapchainRecreates = 0;

	// owned descriptor-image-info arrays (sampler arrays need stable
	// per-element VkDescriptorImageInfo lists; freed in the destructor)
	std::vector<std::vector<VkDescriptorImageInfo>*> m_descriptorImageInfos;
};
