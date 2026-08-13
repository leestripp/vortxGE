/**
 * @file lsFBO_DepthCube.h
 * @brief Cube depth target for omnidirectional (point light) shadows
 *
 * L3.2 (Plan.md). One cube-map depth texture + one framebuffer per
 * face. The point shadow stage renders the scene six times from the
 * light position (90° perspective per face); shading samples the cube
 * as samplerCubeShadow with the reference depth computed from the
 * fragment's major-axis distance through the same 90° projection.
 *
 * GL: pass-through constants. VK: the backend translates cube targets
 * to a 6-layer image + cube view + per-face layer framebuffers.
 */
#pragma once

#include "lsFBO.h"

class lsFBO_DepthCube : public lsFBO
{
public:
	/// size = per-face resolution (square)
	lsFBO_DepthCube( int size, lsRender* render );
	virtual ~lsFBO_DepthCube();

	/// Bind face 0 (lsFBO contract; stages use EnableFace)
	void Enable() override { EnableFace( 0 ); }

	/// Bind the framebuffer for cube face i (0..5 = +X,-X,+Y,-Y,+Z,-Z)
	void EnableFace( int face );

	void ClearBuffers() override;

	/// Recreate the cube at a new per-face size (square; height ignored)
	void Resize( double width, double height ) override;

	lsTextureID getDepthTexture() const { return depth_texture; }
	int size() const { return (int)getWidth(); }

private:
	void CreateGL();

	lsTextureID	depth_texture = 0;
	lsFBOID		m_faceFbos[6] = { 0, 0, 0, 0, 0, 0 };
};
