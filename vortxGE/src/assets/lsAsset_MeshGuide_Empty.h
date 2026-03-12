/**
 * @file lsAsset_MeshGuide_Empty.h
 * @brief Debug axis guide asset
 * 
 * Asset for rendering a 3-axis guide at a point, useful for debugging
 * to visualize orientation and position.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 */

#ifndef vortxGE_lsAsset_MeshGuide_Empty_h
#define vortxGE_lsAsset_MeshGuide_Empty_h

#include "lsAsset.h"

class lsShaderManager;

/**
 * @class lsAsset_MeshGuide_Empty
 * @brief Debug axis visualization
 * 
 * Renders a visual guide showing the three world axes at a point:
 * - X axis: Red line (typically points right)
 * - Y axis: Green line (typically points up)
 * - Z axis: Blue line (typically points forward)
 * 
 * Useful for debugging:
 * - Object positions
 * - Camera orientation
 * - Coordinate system visualization
 * 
 * @note Asset type: LSAT_MESHGUIDE_EMPTY
 * @note Only for debugging - not for production use
 * 
 * @example
 * @code
 * // Show axis at player position
 * lsAsset* guide = am->Add_Asset(LSAT_MESHGUIDE_EMPTY);
 * guide->Set_Pos(player->Get_Pos());
 * @endcode
 */

class lsAsset_MeshGuide_Empty : public lsAsset
{
public:
	/**
	 * @brief Construct empty guide
	 * @param render Pointer to renderer
	 */
    lsAsset_MeshGuide_Empty( lsRender *render );
    
    /// Destructor
    virtual ~lsAsset_MeshGuide_Empty();
    
    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Create axis geometry
     */
    void AllocAsset();
    
    /**
     * @brief Free geometry
     */
    void FreeAsset();
    
private:
    
};


#endif
