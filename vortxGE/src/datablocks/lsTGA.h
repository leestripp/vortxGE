/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
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

// OpenGL
#include "../opengl/lsOpenGL.h"

#pragma pack(push,1)
/**
 The TGA file header
 */
typedef struct TGA_Header
{
    unsigned char  m_ImageIdent;			/* size of image identification field */
    unsigned char  m_ColourMapType;         /* 0 - No colour map, 1 - has colour map */
    unsigned char  m_ImageType;             /* 0 - No Image, 1 - 8bit, 2 - uncompress RGB ..... */
    unsigned short m_ColourMapOrigin;		/* Index of first colour map entry */
    unsigned short m_ColourMapSize;         /* Number of colour map entries */
    unsigned char  m_ColourMapESize;		/* Number of bits for each entry (24) */
    unsigned short m_Xorigin;				/* x origin of image */
    unsigned short m_Yorigin;				/* y origin of image */
    unsigned short m_Width;                 /* width of image in pixels */
    unsigned short m_Height;				/* height of image in pixels */
    unsigned char  m_PixelIndexSize;		/* number of bits stored in each pixel index */
    unsigned char  m_ImageDescByte;         /* should always be zero.... */
} tga_header;

/**
 *	This structure is used purely for reading the RLE files.
 */
typedef struct TGA_Byte
{
    unsigned char Count : 7;
    unsigned char Flag  : 1;
} TgaByte;
#pragma pack(pop)


/**
 *	This union is a total of 2 bytes in size. s and b both share the
 *	same memory so the 2byte variable can be easily split into its constituant bytes.
 *	By assigning 1 to the 2byte variable, we can expect the array b to hold
 *	b[] = { 1, 0 } on a PC (LSB) and b[] = { 0, 1 } on an o2 (MSB). It also makes
 *	it nice and easy to swap the variables bytes.
 */
union charToShort
{
    /* 1 x 2 byte variable */
    unsigned short s;
    /* 2 x 1 byte variables */
    unsigned char  b[2];
};


using namespace std;

/*!
 * \brief TGA image
 *
 * Handle loading and saving of TGA image data.
 *
 */
class lsTGA
{
public:
    lsTGA();
    virtual ~lsTGA();

    int LoadTgaImage( const char* filename, unsigned char** ppData, unsigned int* w, unsigned int* h, unsigned int* bpp );

    // Save the current frame
    void frameGrab( const string& filename );

    void BGR_TO_RGB( unsigned char* data, unsigned int num_px, unsigned int bpp );
    void Flip_Image( unsigned char* data, unsigned int rowsize,unsigned int height );

    // utils
    int CheckByteOrder();
    void Swap2Bytes( void *ptr );

    // Memory
    void setMem_used( unsigned int val )
    {
        mem_used = val;
    }

    unsigned int getMem_used() const
    {
        return mem_used;
    }

private:
    unsigned int mem_used;

};


#endif
