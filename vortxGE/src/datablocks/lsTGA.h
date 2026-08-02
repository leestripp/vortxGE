/**
 * @file lsTGA.h
 * @brief TGA image file loader
 * 
 * Provides TGA (Targa) image file format support for loading
 * texture data. Supports uncompressed and RLE-compressed TGA files.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsTexture
 */

#ifndef vortxGE_lsTGA_h
#define vortxGE_lsTGA_h

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

#pragma pack(push,1)
/**
 * @struct tga_header
 * @brief TGA file header structure
 * 
 * 18-byte header at the start of all TGA files.
 */
typedef struct TGA_Header
{
    unsigned char  m_ImageIdent;			///< Size of image identification field
    unsigned char  m_ColourMapType;       	///< 0 = No color map, 1 = Has color map
    unsigned char  m_ImageType;           	///< Image type (2 = RGB, 10 = RLE RGB, etc.)
    unsigned short m_ColourMapOrigin;		///< Index of first color map entry
    unsigned short m_ColourMapSize;       	///< Number of color map entries
    unsigned char  m_ColourMapESize;		///< Bits per color map entry
    unsigned short m_Xorigin;				///< X origin of image
    unsigned short m_Yorigin;				///< Y origin of image
    unsigned short m_Width;               	///< Image width in pixels
    unsigned short m_Height;				///< Image height in pixels
    unsigned char  m_PixelIndexSize;		///< Bits per pixel
    unsigned char  m_ImageDescByte;       	///< Image descriptor flags
} tga_header;
#pragma packpop

/**
 * @struct TGA_Byte
 * @brief RLE packet structure
 * 
 * Used for reading RLE-compressed TGA files.
 * Flag bit indicates whether packet is raw (0) or RLE (1).
 * Count stores number of pixels minus 1.
 */
typedef struct TGA_Byte
{
    unsigned char Count : 7;	///< Pixel count - 1
    unsigned char Flag  : 1;	///< 0 = Raw, 1 = RLE
} TgaByte;

/**
 * @union charToShort
 * @brief Byte-swapping utility union
 * 
 * Allows easy byte-level access to 16-bit values for
 * handling endianness when reading TGA files.
 */
union charToShort
{
    unsigned short s;	///< 2-byte value
    unsigned char  b[2];	///< Individual bytes
};


using namespace std;

/**
 * @class lsTGA
 * @brief TGA image file handler
 * 
 * Loads TGA images from files and provides utilities for
 * converting pixel formats and flipping images.
 * 
 * Supported TGA types:
 * - Uncompressed RGB (type 2)
 * - Uncompressed grayscale (type 3)
 * - RLE-compressed RGB (type 10)
 * 
 * @example
 * @code
 * lsTGA tga;
 * unsigned char* data;
 * unsigned int w, h, bpp;
 * 
 * if (tga.LoadTgaImage("texture.tga", &data, &w, &h, &bpp) == 0) {
 *     // Convert BGR to RGB if needed
 *     tga.BGR_TO_RGB(data, w * h, bpp);
 *     // Flip if needed (TGA origin can vary)
 *     tga.Flip_Image(data, w * (bpp/8), h);
 * }
 * @endcode
 */
class lsTGA
{
public:
	/// Default constructor
    lsTGA();
    
    /// Destructor
    virtual ~lsTGA();

    //=========================================================================
    // Loading
    //=========================================================================
    
    /**
     * @brief Load TGA image from file
     * 
     * @param filename Path to TGA file
     * @param ppData Pointer to store pixel data (caller must delete[])
     * @param w Pointer to store width
     * @param h Pointer to store height
     * @param bpp Pointer to store bits per pixel
     * @return 0 on success, error code on failure
     */
    int LoadTgaImage( const char* filename, unsigned char** ppData, unsigned int* w, unsigned int* h, unsigned int* bpp );

    //=========================================================================
    // Saving
    //=========================================================================
    
    /**
     * @brief Save current frame to TGA
     * 
     * @param filename Output filename
     */
    void frameGrab( lsRender *render, const string& filename );

    //=========================================================================
    // Image Processing
    //=========================================================================
    
    /**
     * @brief Convert BGR to RGB
     * 
     * TGA stores color as BGR, this swaps to RGB.
     * 
     * @param data Pixel data
     * @param num_px Number of pixels
     * @param bpp Bits per pixel
     */
    void BGR_TO_RGB( unsigned char* data, unsigned int num_px, unsigned int bpp );
    
    /**
     * @brief Flip image vertically
     * 
     * TGA can have different origins, this flips to standard.
     * 
     * @param data Pixel data
     * @param rowsize Bytes per row
     * @param height Image height
     */
    void Flip_Image( unsigned char* data, unsigned int rowsize, unsigned int height );

    //=========================================================================
    // Utilities
    //=========================================================================
    
    /**
     * @brief Check system byte order
     * @return 0 for little-endian, 1 for big-endian
     */
    int CheckByteOrder();
    
    /**
     * @brief Swap 2 bytes in place
     * @param ptr Pointer to 2-byte value
     */
    void Swap2Bytes( void *ptr );

    //=========================================================================
    // Memory
    //=========================================================================
    
    /**
     * @brief Set memory used
     * @param val Memory in bytes
     */
    void setMem_used( unsigned int val )
    {
        mem_used = val;
    }

    /**
     * @brief Get memory used
     * @return Memory in bytes
     */
    unsigned int getMem_used() const
    {
        return mem_used;
    }

private:
    /// Memory used by loaded image
    unsigned int mem_used;

};


#endif
