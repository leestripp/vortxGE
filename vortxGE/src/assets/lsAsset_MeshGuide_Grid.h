/**
 * @file lsAsset_MeshGuide_Grid.h
 * @brief Debug grid floor asset
 * 
 * Asset for rendering a ground grid, useful for perspective and
 * ground plane visualization during development.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 */

#ifndef vortxGE_lsAsset_MeshGuide_Grid_h
#define vortxGE_lsAsset_MeshGuide_Grid_h

#include "lsAsset.h"

class lsShaderManager;

/**
 * @class lsAsset_MeshGuide_Grid
 * @brief Debug grid floor visualization
 * 
 * Renders a grid on the XZ plane (ground) to help with:
 * - Perspective visualization
 * - Ground plane reference
 * - Scale estimation
 * - Object placement debugging
 * 
 * @note Asset type: LSAT_MESHGUIDE_GRID
 * @note Only for debugging - not for production use
 * 
 * @example
 * @code
 * // Add grid to scene for reference
 * lsAsset* grid = am->Add_Asset(LSAT_MESHGUIDE_GRID);
 * grid->Set_Pos(0, 0, 0);
 * @endcode
 */

class lsAsset_MeshGuide_Grid : public lsAsset
{
public:
	/**
	 * @brief Construct grid guide
	 * @param render Pointer to renderer
	 */
    lsAsset_MeshGuide_Grid( lsRender *render );
    
    /// Destructor
    virtual ~lsAsset_MeshGuide_Grid();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Create grid geometry
     */
    void AllocAsset();
    
    /**
     * @brief Free geometry
     */
    void FreeAsset();

private:

};


#endif
