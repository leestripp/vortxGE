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

#include <iostream>
// debug
#include "../vortx_debug.h"

// filesystem
#include "../filesystem/lsDir.h"

#include "lsTGA.h"

lsTGA::lsTGA()
{
    mem_used = 0;

}

lsTGA::~lsTGA()
{
}


/*
 *	This function is used to determine if the system has LSB or MSB byte ordering.
 *	By using a union, both the 2byte variable(short) and the array of 2 individual
 *	bytes (chars) will be stored in the same memory location. ie, the charToShort
 *	union data type is only two bytes in size and allows you to split apart a 2byte
 *	variable to check the ordering.
 *
 *	The two byte variables in the tga header will have to have their byte orderings
 *	switched on irix and mac, hence the use of this (sneaky) function to see if we need
 *	to swap the data bytes.
 */
int lsTGA::CheckByteOrder()
{
    /* Create a 2byte check variable */
    union charToShort cs;

    /* assign 1 to the 2byte variable */
    cs.s = 1;

    /* check the first byte */
    if(cs.b[0] == 0)
        return 1;

    return 0;
}


/*
 *	This function switches the byte orderings of two byte variables. It accepts a
 *	pointer to the memory location you wish to swap.
 */
void lsTGA::Swap2Bytes( void *ptr )
{
    /* assign a pointer to the 2byte variable */
    unsigned char *pcs = (unsigned char*)ptr;

    /* swap the bytes around */
    unsigned char temp = pcs[0];
    pcs[0] = pcs[1];
    pcs[1] = temp;
}


/*
 *	\brief	This function flips pixel colours from RGB (or RGBA) to BGR/BGRA
 *	\param	data	-	the pixel data to flip
 *	\param	num_px	-	the number of pixels to flip
 *	\param	bpp		-	the bytes per pixel, ie 1,3,4
 */
void lsTGA::BGR_TO_RGB(unsigned char* data,unsigned int num_px,unsigned int bpp)
{
    /* get pointer to data end */
    unsigned char *end = data + (bpp * num_px);

    /* loop till end of data, move on by number of bytes in pixel each time */
    for( ; data != end ; data += bpp )
    {
        unsigned char temp = *data;
        *data = data[2];
        data[2] = temp;
    }
}



/*
 *	\brief	This function flips an image upside down...
 *	\param	data	-	the pixel data to flip
 *	\param	rowsize	-	the data size of each scanline
 *	\param	height 	-	the number of scanlines in the image
 */
void lsTGA::Flip_Image( unsigned char* data, unsigned int rowsize,unsigned int height)
{
    unsigned int mid = height/2,i,j;
    unsigned char *temprow = (unsigned char*) malloc( rowsize );
    assert( temprow );

    for( i=0, j=(height-1); i<mid; ++i,--j )
    {
        /* copy low row to temp */
        memcpy( temprow           , &data[ i*rowsize ], rowsize );

        /* copy high row to low row */
        memcpy( &data[ i*rowsize ], &data[ j*rowsize ], rowsize );

        /* copy temp to high row */
        memcpy( &data[ j*rowsize ], temprow           , rowsize );
    }

    /* delete allocated data */
    free( temprow );
}


int lsTGA::LoadTgaImage( const char* filename, unsigned char** ppData, unsigned int* w, unsigned int* h, unsigned int* bpp )
{
#ifdef VORTX_DEBUG_TGA
    cout << "lsTGA::LoadTgaImage - Open TGA image: " << filename << endl;
#endif

    // find our base dir
    string basepath;
    string file;
    lsDir dir;

    // built path
    basepath = dir.getexepath();
    // basepath += "data/";
    file = basepath + filename;

    tga_header header;

    FILE *fp = fopen( file.c_str(),"rb" ) ;
    if(fp)
    {
        unsigned int i,j,k;

        /* Read the file header  */
        fread(&header,1,sizeof(tga_header),fp);
        if(CheckByteOrder())
        {
            Swap2Bytes( (void*)&header.m_ColourMapOrigin );
            Swap2Bytes( (void*)&header.m_ColourMapSize   );
            Swap2Bytes( (void*)&header.m_Xorigin );
            Swap2Bytes( (void*)&header.m_Yorigin );
            Swap2Bytes( (void*)&header.m_Width  );
            Swap2Bytes( (void*)&header.m_Height );
        }

        *w = header.m_Width;
        *h = header.m_Height;
        setMem_used( sizeof(tga_header) );

        /* skip the image identification field if present */
        for( i=0; i!=header.m_ImageIdent; ++i )
        {
            fgetc(fp);
        }

        /* store bytes per pixel */
        *bpp = header.m_PixelIndexSize/8;

        /*
         *	Determine the size (in bytes) of the pixel data size contained
         *	in the file. width * height * 3 bytes per colour (RGB).
         *
         *	Use malloc to allocate the required memory size
         */
        *ppData = (unsigned char*)malloc( (*bpp) * header.m_Width * header.m_Height );
        assert( *ppData != NULL);
        setMem_used( getMem_used() + ((*bpp) * header.m_Width * header.m_Height) );

        /* an 8bit colour mapped image */
        if( header.m_ImageType  == 1 )
        {
            /* This pointer holds reference to the actual 8bit data */
            unsigned char* tempptr = *ppData,*pd=0,*pData=0;

            /* number of bytes per row */
            unsigned int rowsize = (header.m_ColourMapESize/8) * header.m_Width;

            /* allocate and then read in the colour map.... */
            unsigned char* colourMap = (unsigned char*) malloc ( header.m_ColourMapSize * (header.m_ColourMapESize/8) );

            /* check allocation */
            assert( colourMap );

            /* store the new bytes per pixel (as given by the colour map entry size) */
            *bpp = ( header.m_ColourMapESize / 8 );

            /* read colour map data */
            fread( colourMap, header.m_ColourMapSize *(*bpp), 1, fp );

            /* flip colour map from BGR to RGB */
            BGR_TO_RGB( colourMap, header.m_ColourMapSize, *bpp );

            /* read the data in one go, we'll flip it upside down when expanding the colour map data.... */
            fread( tempptr, 1, header.m_Width * header.m_Height, fp );

            /* allocate memory for the new expanded image data... */
            *ppData = (unsigned char*) malloc( header.m_Height * rowsize );
            assert( *ppData );

            /* this pointer is used as an iterator  */
            pData = (*ppData);


            /* now going to run backwards through 8bit image and write it upside down when expanding the colour map */
            for( i=0; i != header.m_Height; ++i )
            {
                /* get a pointer to the data row */
                pd = tempptr + ( (header.m_Height-i-1) * header.m_Width );

                /* loop till the end of the row */
                for( j=0; j != rowsize; ++pd )
                {
                    /* get pointer to actual colour in colour map */
                    unsigned char *col =  &colourMap[ 3*(*pd) ];

                    /* get end of colour */
                    unsigned char *colend = col + *bpp;

                    /* copy the colour over (3 or 4bytes probably...) */
                    for( ; col != colend; ++col, ++j, ++pData )
                    {
                        /* copy colour from colour map into correct place in expanded image */
                        *pData = *col;
                    }
                }
            }

            /* free up any data... */
            free( tempptr );
            free( colourMap );


            /* uncompressed grey/RGB/RGBA data */
        } else if( header.m_ImageType  == 2 )
        {
            /* number of bytes per row */
            unsigned int rowsize = (*bpp) * header.m_Width;

            /* pointer to last row in image data */
            unsigned char* pd = *ppData + (*bpp) * (header.m_Height-1) * header.m_Width;

            /*
             *	read the pixel data from the file in rows cos tga files are upside down...
             */
            for( i = 0; i < header.m_Height; ++i, pd -= rowsize )
            {
                fread( pd, 1, rowsize, fp );
            }

            /* don't bother flipping bytes if greyscale image */
            if( *bpp != 1 )
            {
                /*
                 *	the tga file stores pixel data as bgr values. OpenGL however requires
                 *	the data in RGB. Therefore we have to traverse the array and swap the
                 *	pixel data around abit.
                 */
                BGR_TO_RGB( (*ppData), header.m_Height * header.m_Width, *bpp );
            }

            /* RLE colour mapped images */
        } else if( header.m_ImageType  == 9 )
        {
            /* number of bytes per row */
            unsigned int rowsize = (header.m_ColourMapESize/8) * header.m_Width;

            /* allocate and then read in the colour map.... */
            unsigned char* colourMap = (unsigned char*) malloc ( header.m_ColourMapSize * (header.m_ColourMapESize/8) );

            /* temporary iterator pointers */
            unsigned char *pde,*pData;

            /* check allocation */
            assert( colourMap );

            /* store the new bytes per pixel (as given by the colour map entry size) */
            *bpp = ( header.m_ColourMapESize / 8 );

            /* read colour map data */
            fread( colourMap, header.m_ColourMapSize * (*bpp), 1, fp );

            /* flip colour map from BGR to RGB */
            BGR_TO_RGB( colourMap, header.m_ColourMapSize, *bpp );

            /* allocate memory for the uncompressed image... */
            *ppData = (unsigned char*) malloc ( header.m_Height * header.m_Width * (*bpp)  );

            /* get ptr to first element */
            pData = *ppData;

            /* get end pointer marker */
            pde = pData + header.m_Height * header.m_Width * (*bpp);

            /* now the reading process begins!! */
            while( pde != pData )
            {
                unsigned char byte;

                fread( &byte, 1, 1, fp );

                /*
                 *	if the flag is zero, then the count indicates a run of raw data
                 */
                if( (byte & 0x80) == 0 )
                {
                    byte++;

                    for(j=0;j<byte; ++j)
                    {
                        unsigned char c = fgetc(fp);
                        for( k = 0; k != *bpp; ++k, ++pData )
                        {
                            *pData = colourMap[ (*bpp)*c + k ];
                        }
                    }
                }

                /*
                 *	If the flag is one, then the count represents the amount of times that the next byte
                 *	is repeated. This is the RLE bit....
                 */
                else
                {
                    unsigned char c = fgetc(fp);
                    byte -= 0x7F;

                    for(j=0;j<byte; ++j)
                    {
                        for( k = 0; k != *bpp; ++k, ++pData )
                        {
                            *pData = colourMap[ (*bpp)*c + k ];
                        }
                    }

                }
            }

            /* turn the image upside down .... */
            Flip_Image( *ppData, rowsize, header.m_Height );

        } else if( header.m_ImageType  == 10 )
        {
            /* RLE compressed grey/RGB/RGBA images */

            /* This pointer holds reference to the actual 8bit data */
            // unsigned char* tempptr = *ppData;

            /* number of bytes per row */
            // unsigned int rowsize = (*bpp) * header.m_Width;

            /* temporary iterator pointers */
            unsigned char* pde,*pData;

            /* get ptr to first element */
            pData = *ppData;

            /* get end pointer marker */
            pde = pData + header.m_Height * header.m_Width * (*bpp);

            /* time to read the data */
            while( pData < pde )
            {
                unsigned char colour[10] = {0,0,0,0};
                unsigned char byte;

                fread( &byte, 1, 1, fp );

                /*
                 *	if the flag is zero, then the count indicates a run of raw data
                 */
                if( (byte & 0x80) == 0 )
                {
                    byte++;

                    for(j=0;j<byte; ++j)
                    {
                        fread( colour, 1, *bpp, fp );

                        for( k = 0; k != *bpp; ++k, ++pData )
                        {
                            *pData = colour[ k ];
                        }
                    }
                }

                /*
                 *	If the flag is one, then the count represents the amount of times that the next byte
                 *	is repeated. This is the RLE bit....
                 */
                else
                {
                    byte -= 0x7F;

                    fread( colour, 1, *bpp, fp );

                    for(j=0;j<byte; ++j)
                    {
                        for( k = 0; k != *bpp; ++k, ++pData )
                        {
                            *pData = colour[ k ];
                        }
                    }

                }
            }

            /* flip colour map from BGR to RGB */
            BGR_TO_RGB( *ppData, header.m_Width*header.m_Height, *bpp );

            /* turn the image upside down .... */
            // Flip_Image( *ppData, rowsize, header.m_Height );

        } else
        {
            /* unsupported image type... */
            switch( header.m_ImageType )
            {
            case 0:
                printf("[ERROR] The file %s contains no image data\n",filename);
                break;
            case 3:
                printf("[ERROR] The file %s contains black & white data which is unsupported\n",filename);
                break;
            case 11:
                printf("[ERROR] The file %s contains compressed black & white data which is unsupported\n",filename);
                break;
            case 32:
                printf("[ERROR] The file %s contains compressed color map data which is unsupported\n",filename);
                break;
            case 33:
                printf("[ERROR] The file %s contains compressed color map data which is un-supported (4pass quad tree process)\n",filename);
                break;
            default:
                printf("[ERROR] The file %s does not appear to be valid\n",filename);
                break;

            }// switch( header.m_ImageType )
        }

        /*
         *	close the file
         */
        fclose(fp);

        return 1;

    } else
    {
        // ERROR
        cout << "ERROR: Could not open TGA file: " << filename << endl;
    } // if(fp)

    return 0;
}

/*!
 * \brief lsTGA::frameGrab
 *
 * Grab the current frame buffer and save it to a TGA file.\n
 * \n
 *
 * \param filename
 * \param x
 * \param y
 */
void lsTGA::frameGrab( const string& filename )
{
    // Get current viewport size_t
    GLint dims[4] = {0};

    glGetIntegerv( GL_VIEWPORT, dims );
    int x = dims[2];
    int y = dims[3];

    // get the image data
    long imageSize = x * y * 3;
    const char *data = new char[imageSize];
    glReadPixels( 0, 0, x, y, GL_BGR, GL_UNSIGNED_BYTE, (void*)data );

    // split x and y sizes into bytes
    int xa = x % 256;
    int xb = (x-xa)/256;
    int ya = y % 256;
    int yb = (y-ya)/256;

    //assemble the header
    const char header[18] = {0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,24,0};

    // write header and data to file
    fstream File( filename.c_str(), ios::out | ios::binary );
    if( File.is_open() )
    {
        File.write( header, sizeof(char)*18 );
        File.write( data, sizeof(char)*imageSize );
        File.close();
    } else
    {
        cout << "ERROR : (lsTGA) Cant open file for output : " << filename << endl;
    }

    // cleanup
    delete[] data;
    data = NULL;
}

