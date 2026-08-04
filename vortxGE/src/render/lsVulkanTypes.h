/**
 * @file lsVulkanTypes.h
 * @brief LS_* (GL-valued) constant → Vk* enum mappings for the Vulkan backend
 *
 * The engine passes GL-numeric constants through the lsRender API (see
 * lsRenderConstants.h). The Vulkan backend translates them at its
 * boundary using the helpers below. Anything unmapped logs once and
 * falls back to a sane default so a missed constant degrades instead
 * of crashing.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <iostream>

#include "lsRenderConstants.h"

namespace lsVkMap
{

inline VkPrimitiveTopology topology( lsEnum mode )
{
	switch( mode )
	{
	case LS_TRIANGLES:		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case LS_TRIANGLE_STRIP:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case LS_LINES:			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case LS_LINE_STRIP:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case LS_POINTS:			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	default:				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

inline VkIndexType indexType( lsEnum type )
{
	switch( type )
	{
	case LS_UNSIGNED_SHORT:	return VK_INDEX_TYPE_UINT16;
	case LS_UNSIGNED_INT:	return VK_INDEX_TYPE_UINT32;
	case LS_UNSIGNED_BYTE:	return VK_INDEX_TYPE_UINT8_EXT;
	default:				return VK_INDEX_TYPE_UINT32;
	}
}

inline VkCompareOp compareOp( lsEnum func )
{
	switch( func )
	{
	case LS_NEVER:		return VK_COMPARE_OP_NEVER;
	case LS_LESS:		return VK_COMPARE_OP_LESS;
	case LS_EQUAL:		return VK_COMPARE_OP_EQUAL;
	case LS_LEQUAL:		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case LS_GREATER:	return VK_COMPARE_OP_GREATER;
	case LS_NOTEQUAL:	return VK_COMPARE_OP_NOT_EQUAL;
	case LS_GEQUAL:		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case LS_ALWAYS:		return VK_COMPARE_OP_ALWAYS;
	default:			return VK_COMPARE_OP_LESS_OR_EQUAL;
	}
}

inline VkBlendFactor blendFactor( lsEnum f )
{
	switch( f )
	{
	case LS_ZERO:					return VK_BLEND_FACTOR_ZERO;
	case LS_ONE:					return VK_BLEND_FACTOR_ONE;
	case LS_SRC_COLOR:				return VK_BLEND_FACTOR_SRC_COLOR;
	case LS_ONE_MINUS_SRC_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case LS_DST_COLOR:				return VK_BLEND_FACTOR_DST_COLOR;
	case LS_ONE_MINUS_DST_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case LS_SRC_ALPHA:				return VK_BLEND_FACTOR_SRC_ALPHA;
	case LS_ONE_MINUS_SRC_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case LS_DST_ALPHA:				return VK_BLEND_FACTOR_DST_ALPHA;
	case LS_ONE_MINUS_DST_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	default:						return VK_BLEND_FACTOR_SRC_ALPHA;
	}
}

inline VkCullModeFlags cullMode( lsEnum face )
{
	switch( face )
	{
	case LS_FRONT:			return VK_CULL_MODE_FRONT_BIT;
	case LS_BACK:			return VK_CULL_MODE_BACK_BIT;
	case LS_FRONT_AND_BACK:	return VK_CULL_MODE_FRONT_AND_BACK;
	default:				return VK_CULL_MODE_BACK_BIT;
	}
}

/// Vertex attribute element count + GL type → VkFormat (float attrs only
/// for now — engine meshes are float32 throughout).
inline VkFormat attribFormat( lsInt size, lsEnum type )
{
	if( type == LS_FLOAT )
	{
		switch( size )
		{
		case 1: return VK_FORMAT_R32_SFLOAT;
		case 2: return VK_FORMAT_R32G32_SFLOAT;
		case 3: return VK_FORMAT_R32G32B32_SFLOAT;
		case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
	}
	std::cout << "lsVkMap::attribFormat - unmapped size/type " << size
			  << "/" << type << " (using vec3 float)" << std::endl;
	return VK_FORMAT_R32G32B32_SFLOAT;
}

} // namespace lsVkMap
