/**
 * @file lsBatch_Normals.h
 * @brief Normal visualization batch for debugging
 * 
 * lsBatch_Normals extends lsBatch_Lines to provide a utility for
 * visualizing surface normals and tangents. This is invaluable for:
 * - Debugging mesh orientation
 * - Verifying normal map generation
 * - Checking tangent space continuity
 * - Educational visualization of surface properties
 * 
 * @section visualization Normal Visualization
 * 
 * Each normal is typically drawn as a line starting at the vertex
 * position and extending in the direction of the normal. The length
 * can be scaled for better visibility.
 * 
 * @section usage Usage
 * 
 * @code
 * lsBatch_Normals* normals = new lsBatch_Normals(renderer);
 * normals->setShader(debugShader);
 * // Build from mesh vertex data
 * normals->Draw(camera, object, pass);
 * @endcode
 * 
 * @see lsBatch_Lines for base line functionality
 * @see lsBatch_Mesh for source mesh data
 */

#ifndef vortxGE_IOS_lsBatch_Normats_h
#define vortxGE_IOS_lsBatch_Normats_h

#include "lsBatch_Lines.h"

/**
 * @class lsBatch_Normals
 * @brief Normal visualization batch
 * 
 * lsBatch_Normals is used to visualize surface normals
 * and tangent vectors. It inherits from lsBatch_Lines
 * and provides the same line rendering capabilities
 * optimized for normal visualization.
 * 
 * @note Typically rendered in a distinctive color (e.g., green)
 * @note Line length can be adjusted for visualization preference
 */

class lsBatch_Normals : public lsBatch_Lines
{
public:
    /**
     * @brief Construct a normals batch
     * @param render Pointer to the renderer
     */
    lsBatch_Normals( lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsBatch_Normals();

private:
    // Implementation inherited from lsBatch_Lines

};


#endif
