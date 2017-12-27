/*
*  msimisc.c
*
*  This sample code demonstrates how some other, less used, MSIDOS API
*  calls are made.
*  Please use the 'msidos.doc' document as a guide.
*/

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "..\msidos.h"
#include "bitmap.h" /* Needed to read the .bmp files under DOS */

/* Some defines */
#define LPCSTR const char far*
#define MSIDBG_MSG_PREFIX       "[msimisc] "
#define MSIDBG_LOG_PATH         "msimisc.log"

#define VK_ESCAPE   27

#define TRUE        1
#define FALSE       0

#define TEXA_SIZE_X 256
#define TEXA_SIZE_Y 256
#define TEXA_OFFSET 0

/* Global Variables */
static T_msiInfo*  pInfo;   /* MSI Direct Frame Buffer Access information structure. */

T_msiVertex msiVertex[9];
T_msiParameters msiParameters;

BYTE pBitmapA[2 * TEXA_SIZE_X * TEXA_SIZE_Y];

/* Used for msiInit */
#define Xres 640
#define Yres 480
#define UseHardwareZ 1

/* Functions */

/*
*  FUNCTION:    DBG_PRINT
*
*  DESCRIPTION: Prints possible errors to a file
*/
static void DBG_PRINT( char *pMessage, ... ) {
    char     buffer[256];
    int      len;
    va_list  ap;
    FILE     *fd;

    va_start( ap, pMessage );

    strcpy( buffer, MSIDBG_MSG_PREFIX );
    len = vsprintf( buffer + strlen( MSIDBG_MSG_PREFIX ), pMessage, ap );

    buffer[strlen( MSIDBG_MSG_PREFIX ) + len] = '\n';
    buffer[strlen( MSIDBG_MSG_PREFIX ) + len + 1] = '\0';

    if ( ( fd = fopen( MSIDBG_LOG_PATH, "a" ) ) != NULL ) {
        fprintf( fd, buffer );
        fclose( fd );
    }

    va_end( ap );

}

/*
*  FUNCTION:      SetDefaultParameters
*
*  DESCRIPTION:   Initializes default values for calls to msiSetParameters
*/
void SetDefaultParameters( T_msiParameters *msiParameters ) {
    msiParameters->msiTexture.Enable = TRUE;
    msiParameters->msiTexture.Width = 0;
    msiParameters->msiTexture.Height = 0;
    msiParameters->msiTexture.Planes = 16;
    msiParameters->msiTexture.pMem = NULL;
    msiParameters->msiTexture.pHeap = NULL;
    msiParameters->msiTexture.CacheOffset = 0;
    msiParameters->msiTexture.Clamp_u = FALSE;
    msiParameters->msiTexture.Clamp_v = FALSE;
    msiParameters->msiTexture.Modulate = FALSE;
    msiParameters->msiTexture.Decal = FALSE;
    msiParameters->msiTexture.Transparency = FALSE;
    msiParameters->msiTexture.KeyingColor = 0x0000;
    msiParameters->msiTexture.KeyingColorMask = 0xFFFF;
    msiParameters->msiTexture.KeyingAlpha = 0x0001;
    msiParameters->msiTexture.KeyingAlphaMask = 0x0000;

    msiParameters->msiTexture.msiLUT.pMem = NULL;
    msiParameters->msiTexture.msiLUT.pHeap = NULL;
    msiParameters->msiTexture.msiLUT.CacheOffset = 0;

    msiParameters->msiDepth.Enable = FALSE;
    msiParameters->msiDepth.Compare = msiCMP_LEQUAL;
    msiParameters->msiDepth.Protect = FALSE;

    msiParameters->msiColor.Dither = TRUE;
    msiParameters->msiColor.Protect = FALSE;

}

/*
*  FUNCTION:    AddxAddy
*
*  DESCRIPTION: Used to modify the vertices of a triangle
*/
BOOL AddxAddy( T_msiVertex *pVertex, int from, int to, float xadd, float yadd ) {
    int i = 0;

    if ( from > to ) {
        return ( FALSE );
    }

    for ( i = from; i <= to; i++ ) {
        pVertex[i].x += xadd;
        pVertex[i].y += yadd;
    }

    return ( TRUE );

}

/*
*  FUNCTION:    BMPLoad
*
*  DESCRIPTION: Used to load BMPs from disk and into pBitMap
*               The Pitch = 0 Linear
*/
BOOL BMPLoad( LPCSTR FileName,  /* 32 bit ptr. to constant char str. */
    BYTE *pBitMap,              /* Pointer to where in mem. to load the BMP. */
    LONG XSize,                 /* The Width of the BMP to be Loaded. */
    LONG XOffset,               /* The Upper Left Corner of BMP. */
    LONG YSize,                 /* The Hight of the BMP to be Loaded. */
    LONG YOffset,               /* The Upper Left Corner of BMP. */
    LONG Pitch                  /* The Pitch. */
) {
    BITMAPFILEHEADER  BitMapFileHeader;   /* The BitMap File Header. */
    BITMAPINFOHEADER  BitMapInfoHeader;   /* The BitMap Info Information. */

    FILE* pFile;                /* File Pointer. */
    char  char_read;            /* The character read from File.BMP */
    LONG  off_start;            /* End of actual bitmap data on file. */
    LONG  xx, yy;               /* Loop Variables in x,y directions. */
    WORD  Color;                /* Used to store the complete 16 Bit RGB Color. */
    BOOL  Flag;
    long i;

    /* File Does not exist in specified path. */
    if ( ( pFile = fopen( FileName, "rb" ) ) == NULL ) {
        DBG_PRINT( "BMPLoad Error: Can't open the source bitmap, File not found." );
        return( FALSE );
    }

    /* Loading the BMP File Header. */
    if ( fread( &BitMapFileHeader, sizeof( BitMapFileHeader ), 1, pFile ) == NULL ) {
        DBG_PRINT( "BMPLoad Error: BMP File Header Corrupted." );
        return( FALSE );
    }

    /* Loading the BMP Map Information Header  */
    if ( fread( &BitMapInfoHeader, sizeof( BitMapInfoHeader ), 1, pFile ) == NULL ) {
        DBG_PRINT( "BMPLoad Error: BMP Map Information Header Corrupted." );
        return( FALSE );
    }

    /* Check that this is a Valid BMP File: 1st 6 Bytes must be "BM??00" */
    if ( ( BitMapFileHeader.bfType != 66 + 77 * 256 ) | ( BitMapFileHeader.bfReserved1 != 0 ) | ( BitMapFileHeader.bfReserved2 != 0 ) ) {
        DBG_PRINT( "BMPLoad Error: Not a Valid BMP File." );
        return( FALSE );
    }

    /* Check to see if file is 24Bits per Color. */
    if ( ( BitMapInfoHeader.biBitCount != 24 ) /*& ( BitMapInfoHeader.biBitCount != 16 ) & ( BitMapInfoHeader.biBitCount != 8 )*/ ) {
        DBG_PRINT( "BMLoad Error: File NOT in Bits per Color." );
        DBG_PRINT( "   - File is %d Bits per Color....", BitMapInfoHeader.biBitCount );
        return( FALSE );
    }

    /* bfOffBits: Offset in Bytes, from the BITMAPFILEHEADER structure
    *  to the bitmap bits. */
    fseek( pFile, 0, SEEK_END ); /* Move the file pointer to the end of the file.*/
    off_start = ftell( pFile );  /* Get the current position of the file pointer.*/

                                 /* Check for invalid XSize.*/
    i = 1;
    Flag = FALSE;
    for ( xx = 1; xx < 17; xx++ ) {
        if ( XSize == i ) { Flag = TRUE; }
        i *= 2;
    }
    if ( Flag == FALSE ) {
        DBG_PRINT( "BMPLoad Error: Invalid XSize Parameter %d (Must be a Power of 2).", XSize );
        return ( FALSE );
    }

    /* Check for invalid YSize. */
    i = 1;
    Flag = FALSE;
    for ( yy = 1; yy < 17; yy++ ) {
        if ( XSize == i ) { Flag = TRUE; }
        i *= 2;
    }
    if ( Flag == FALSE ) {
        DBG_PRINT( "BMPLoad Error: Invalid YSize Parameter %d (Must be a Power of 2).", YSize );
        return ( FALSE );
    }

    /* Check for Invalid XOfsset, XSize Parameters. */
    if ( XOffset + XSize>BitMapInfoHeader.biWidth ) {
        DBG_PRINT( "BMPLoad Error: Access Beyond BMP Width." );
        DBG_PRINT( "XOffset + XSize = %d + %d = %d, Is Greater than BMP Width: %d", XOffset, XSize, XOffset + XSize, BitMapInfoHeader.biWidth );
        return( FALSE );
    }

    /* Check for Invalid YOffset, YSize Parameters. */
    if ( YOffset + YSize>BitMapInfoHeader.biHeight ) {
        DBG_PRINT( "BMPLoad Error: Access Beyond BMP Height." );
        DBG_PRINT( "YOffset + YSize = %d + %d = %d, Is Greater than BMP Height: %d", YOffset, YSize, YOffset + YSize, BitMapInfoHeader.biHeight );
        return( FALSE );
    }

    /* Check if BMP File is RLE compressed. */
    if ( BitMapInfoHeader.biCompression != BI_RGB ) {
        DBG_PRINT( "BMPLoad Error: BMP File is in Compressed Format:" );
        switch ( BitMapInfoHeader.biCompression ) {
        case BI_RLE8: DBG_PRINT( "  A run-length encoded (RLE) format for bitmaps with 8 bits per pixel." ); break;
        case BI_RLE4: DBG_PRINT( "    An RLE format for bitmaps with 4 bits per pixel." ); break;
        case BI_BITFIELDS: DBG_PRINT( "   Color table consists of three doubleword color masks per RGB Triplet." ); break;
        }
        return( FALSE );
    }

    /* If ( Pitch == 0 ) Do a Linear BMP Load from bfOffBits. */
    /*fseek( pFile, BitMapFileHeader.bfOffBits, SEEK_SET );*/
    i = 0;
    /*
    / * Check if the File Lengths match.
    * This makes sure that none of the RGB Triplets are missing, i.e. that the file data is complete. * /
    if ( ( off_start - ( ( unsigned ) BitMapInfoHeader.biWidth
        * ( unsigned ) BitMapInfoHeader.biHeight * 3 ) ) != BitMapFileHeader.bfOffBits ) {
        DBG_PRINT( "BMPLoad Error: BMP File corrupted." );
        DBG_PRINT( "   - BitMapFileHeader.bfOffBits = %d", BitMapFileHeader.bfOffBits );
        DBG_PRINT( "   - Off_Start .... = %d", off_start - ( BitMapInfoHeader.biWidth
            * BitMapInfoHeader.biHeight * 3 ) );
        DBG_PRINT( "Dimensions %d x %d", ( unsigned ) BitMapInfoHeader.biWidth, ( unsigned ) BitMapInfoHeader.biHeight );
        return( FALSE );
    }
    */
    /* Read the File */
    for ( yy = 0; yy < YSize; yy++ ) {
        fseek( pFile, ( ( XOffset * 3 ) + BitMapFileHeader.bfOffBits + ( ( BitMapInfoHeader.biHeight - yy - YOffset - 1 )*BitMapInfoHeader.biWidth * 3 ) ), SEEK_SET );
        for ( xx = 0; xx < XSize; xx++ ) {
            /* read as BGR
            *  Initialize color to 0, read the colors (1 byte each), shift the wanted bits to
            *  correct equivalent position in color, mask out unwanted bits with & then add
            *  the 3 parts together with or to obtain the final 16 bit color. */
            Color = 0;

            fscanf( pFile, "%c", &char_read );              /* BLUE */
            Color |= ( ( char_read >> 3 & 0x001F ) );       /* BLUE     0000 0000 0001 1111 */
            fscanf( pFile, "%c", &char_read );              /* GREEN */
            Color |= ( ( char_read >> 2 << 5 ) & 0x07E0 );  /* GREEN    0000 0111 1110 0000 */
            fscanf( pFile, "%c", &char_read );              /* RED */
            Color |= ( ( char_read >> 3 << 11 ) & 0xF800 ); /* RED      1111 1000 0000 0000 */

            if ( Pitch != 0 ) {
                /* BITMAP IS BOTTOM UP */
                ( ( WORD * ) pBitMap )[( (/*( Ysize - 1 ) -*/ yy ) *Pitch ) + xx] = Color;
            } else {
                ( ( WORD * ) pBitMap )[i++] = Color;
            }
        }
    }

    return ( TRUE );

}

/*
*  FUNCTION:    CopyBMPLinear
*
*  DESCRIPTION: Used to copy data from main memory to a Heap or directly
*               to a valid MSI pInfo pointer (Direct Frame Buffer Access)
*/
void CopyBMPLinear( LPBYTE pTexture, BYTE *pBitmap, int Width, int Height ) {
    /* pTexture:    A pointer to the start of the Video card's memory (Color,
    *                 Depth, or Texture Cache) or to a valid MSI Heap
    *
    *  pBitmap:     A pointer to the bitmap residing in main memory
    *
    *  The bitmap data at pBitmap is moved into the destination - linearly.
    *
    *  If copying to a MSI pInfo pointer (Direct Frame Buffer Access), a call
    *  to msiSetParameters(NULL) was previously necessary. This BREAKS the
    *  BUS-MASTERING which reduces performance, hence this method should only
    *  be used when frame rates are not critical!  A non critical case would be
    *  at very the beginning of a game or at the beginning of a game level. */

    int xx, yy;

    /* Copy the bytes linearly - assumes 16bpp textures */
    for ( yy = 0; yy < Height; yy++ ) {
        for ( xx = 0; xx < Width; xx++ ) {
            ( ( WORD far* ) pTexture )[xx + ( yy * Width )] = ( ( WORD* ) pBitmap )[xx + ( yy * Width )];
        }
    }

}

/* Tests */

void GeneralStateChange( ) {
    /*
    *  Demonstrates how to use: msiSetDepthCompare, msiSetDitherEnable,
    *                           msiSetParameters, msiRenderTriangle
    *  Demonstrates how to:     Draw Gouraud Shaded triangles.
    */

    if ( !msiStartFrame( 1, 100.0f, 100.0f, 100.0f, 1, 0.5 ) ) {
        DBG_PRINT( "msiStartFrame has failed.\n" );
        exit( 0 );
    }

    /* Set up the structure to pass to msiSetParameters */
    SetDefaultParameters( &msiParameters );

    msiParameters.msiTexture.Enable = FALSE;
    /* Since the above is set to FALSE, the msiTexture and msiTexture.msiLUT
    *  parameters will not actually be processed by msiSetParameters. */

    msiParameters.msiDepth.Enable = TRUE;
    msiParameters.msiDepth.Compare = msiCMP_ALWAYS;

    if ( !msiSetParameters( &msiParameters ) ) {
        DBG_PRINT( "msiSetParameters has failed.\n" );
        exit( 0 );
    }

    /* Set the vertices to pass to msiRenderTriangle */

    /* The Top triangles are for demonstrating msiSetDepthCompare
    *  Note that since the texturing is not enabled in these tests, the
    *  following vertex values need not be defined as they are not processed:
    *  msiVertex[].mr, msiVertex[].mg, msiVertex[].mb,
    *  msiVertex[].u, msiVertex[].v, msiVertex[].invW */
    msiVertex[0].x = 20.0f;     msiVertex[0].y = 50.0f;     msiVertex[0].z = 0.4f;
    msiVertex[0].r = 255.0f;    msiVertex[0].g = 0.0f;      msiVertex[0].b = 0.0f;
    msiVertex[1].x = 140.0f;    msiVertex[1].y = 50.0f;     msiVertex[1].z = 0.4f;
    msiVertex[1].r = 0.0f;      msiVertex[1].g = 255.0f;    msiVertex[1].b = 0.0f;
    msiVertex[2].x = 80.0f;     msiVertex[2].y = 150.0f;    msiVertex[2].z = 0.4f;
    msiVertex[2].r = 0.0f;      msiVertex[2].g = 0.0f;      msiVertex[2].b = 255.0f;
    msiVertex[3].x = 20.0f;     msiVertex[3].y = 150.0f;    msiVertex[3].z = 0.6f;
    msiVertex[3].r = 255.0f;    msiVertex[3].g = 255.0f;    msiVertex[3].b = 255.0f;
    msiVertex[4].x = 80.0f;     msiVertex[4].y = 50.0f;     msiVertex[4].z = 0.6f;
    msiVertex[4].r = 255.0f;    msiVertex[4].g = 255.0f;    msiVertex[4].b = 255.0f;
    msiVertex[5].x = 140.0f;    msiVertex[5].y = 150.0f;    msiVertex[5].z = 0.6f;
    msiVertex[5].r = 255.0f;    msiVertex[5].g = 255.0f;    msiVertex[5].b = 255.0f;

    /* The Bottom triangles are for demonstrating msiSetDitherEnable */
    msiVertex[6].x = 10.0f;     msiVertex[6].y = 200.0f;    msiVertex[6].z = 0.8f;
    msiVertex[6].r = 255.0f;    msiVertex[6].g = 255.0f;    msiVertex[6].b = 255.0f;
    msiVertex[7].x = 310.0f;    msiVertex[7].y = 200.0f;    msiVertex[7].z = 0.8f;
    msiVertex[7].r = 255.0f;    msiVertex[7].g = 0.0f;      msiVertex[7].b = 0.0f;
    msiVertex[8].x = 160.0f;    msiVertex[8].y = 460.0f;    msiVertex[8].z = 0.8f;
    msiVertex[8].r = 0.0f;      msiVertex[8].g = 0.0f;      msiVertex[8].b = 255.0f;

    /* Will draw both triangles due to the Depth Compare value */
    if ( !msiRenderTriangle( &msiVertex[3], &msiVertex[4], &msiVertex[5], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }
    if ( !msiRenderTriangle( &msiVertex[0], &msiVertex[1], &msiVertex[2], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }

    /* Move the triangle to another location on the screen */
    AddxAddy( msiVertex, 0, 5, 250, 0 );
    if ( !msiSetDepthCompare( msiCMP_LEQUAL ) ) {
        DBG_PRINT( "msiSetDepthCompare has failed.\n" );
        exit( 0 );
    }
    /* Will draw the first triangle only */
    if ( !msiRenderTriangle( &msiVertex[0], &msiVertex[1], &msiVertex[2], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }
    if ( !msiRenderTriangle( &msiVertex[3], &msiVertex[4], &msiVertex[5], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }

    AddxAddy( msiVertex, 0, 5, 250, 0 );
    if ( !msiSetDepthCompare( msiCMP_GEQUAL ) ) {
        DBG_PRINT( "msiSetDepthCompare has failed.\n" );
        exit( 0 );
    }
    /* Will draw the second triangle only */
    if ( !msiRenderTriangle( &msiVertex[0], &msiVertex[1], &msiVertex[2], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }
    if ( !msiRenderTriangle( &msiVertex[3], &msiVertex[4], &msiVertex[5], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }

    /* Now draw the Bottom triangles for showing color Dithering */
    if ( !msiRenderTriangle( &msiVertex[6], &msiVertex[7], &msiVertex[8], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }

    AddxAddy( msiVertex, 6, 8, 305, 0 );
    if ( !msiSetDitherEnable( FALSE ) ) {
        DBG_PRINT( "msiSetDitherEnable has failed.\n" );
        exit( 0 );
    }
    if ( !msiRenderTriangle( &msiVertex[6], &msiVertex[7], &msiVertex[8], 100 ) ) {
        DBG_PRINT( "msiRenderTriangle has failed.\n" );
        exit( 0 );
    }

    /* TRUE renders the frame immediately. */
    if ( !msiEndFrame( 0, 0, TRUE ) ) {
        DBG_PRINT( "msiEndFrame has failed.\n" );
        exit( 0 );
    }

}

void RenderZPlaneTest_Z( ) {
    /*
    *  Demonstrates how to use: msiRenderZPlane, msiSetTextureTransparency
    */

    LPBYTE pTexture;
    T_msiZPlane       ZPlane;

    /* Allocate the texture heaps (# of 4K byte pages) to hold the bitmap.
    *  Calculation: (256*256*2bpp)/4096 = 32 */
    pTexture = msiAllocTextureHeap( 32 );
    if ( pTexture == NULL ) {
        DBG_PRINT( "msiAllocTextureHeap has failed.\n" );
        exit( 0 );
    }

    CopyBMPLinear( pTexture, &pBitmapA[0], TEXA_SIZE_X, TEXA_SIZE_Y );

    /* Start a new frame and clear both the RGB and Z buffers.
    *  The new frame is cleared to white and Z-Buffer to 1 (being furthest). */
    if ( !msiStartFrame( 1, 100.0f, 100.0f, 100.0f, 1, 1.0 ) ) {
        DBG_PRINT( "msiStartFrame has failed.\n" );
        exit( 0 );
    }

    SetDefaultParameters( &msiParameters );

    msiParameters.msiTexture.Width = TEXA_SIZE_X;
    msiParameters.msiTexture.Height = TEXA_SIZE_Y;
    msiParameters.msiTexture.pMem = pTexture;
    msiParameters.msiTexture.pHeap = pTexture;
    msiParameters.msiTexture.CacheOffset = TEXA_OFFSET;
    msiParameters.msiTexture.Clamp_u = TRUE;
    msiParameters.msiTexture.Clamp_v = TRUE;
    msiParameters.msiDepth.Enable = TRUE;
    if ( !msiSetParameters( &msiParameters ) ) {
        DBG_PRINT( "msiSetParameters has failed.\n" );
        exit( 0 );
    }

    /* Render a Zplane image */
    ZPlane.x = 10;
    ZPlane.y = 10;
    ZPlane.xWidth = 256;
    ZPlane.yHeight = 256;

    ZPlane.u = 0;
    ZPlane.uWidth = 256 - 1;
    ZPlane.v = 0;
    ZPlane.vHeight = 256 - 1;

    ZPlane.z = 0.5f;

    ZPlane.r = 255.0f;
    ZPlane.g = 255.0f;
    ZPlane.b = 255.0f;
    ZPlane.mr = 255.0f;
    ZPlane.mg = 255.0f;
    ZPlane.mb = 255.0f;

    msiRenderZPlane( &ZPlane, 100 );

    ZPlane.x = 255;
    ZPlane.y = 45;
    ZPlane.xWidth = 360;
    ZPlane.yHeight = 125;

    ZPlane.u = 0;
    ZPlane.uWidth = 256 - 1;
    ZPlane.v = 0;
    ZPlane.vHeight = 256 - 1;

    ZPlane.z = 0.7f;

    ZPlane.r = 255.0f;
    ZPlane.g = 255.0f;
    ZPlane.b = 255.0f;
    ZPlane.mr = 255.0f;
    ZPlane.mg = 0.0f;
    ZPlane.mb = 0.0f;

    if ( !msiSetTextureBlend( MODULATE ) ) {
        DBG_PRINT( "msiSetTextureBlend has failed.\n" );
        exit( 0 );
    }

    msiRenderZPlane( &ZPlane, 100 );

    /* Render a Zplane image */
    ZPlane.x = 200;
    ZPlane.y = 145;
    ZPlane.xWidth = 220;
    ZPlane.yHeight = 260;

    ZPlane.u = 0;
    ZPlane.uWidth = 256 - 1;
    ZPlane.v = 25;
    ZPlane.vHeight = 225;

    ZPlane.z = 0.6f;

    ZPlane.r = 255.0f;
    ZPlane.g = 255.0f;
    ZPlane.b = 0.0f;
    ZPlane.mr = 255.0f;
    ZPlane.mg = 255.0f;
    ZPlane.mb = 255.0f;

    if ( !msiSetTextureBlend( DECAL ) ) {
        DBG_PRINT( "msiSetTextureBlend has failed.\n" );
        exit( 0 );
    }

    /* Make 'white' the keying color for decal */
    if ( !msiSetTextureTransparency( 0xFFFFFFFF ) ) {
        DBG_PRINT( "msiSetTextureTransparency has failed.\n" );
        exit( 0 );
    }

    msiRenderZPlane( &ZPlane, 100 );

    /* TRUE renders the frame immediately. */
    if ( !msiEndFrame( 0, 0, TRUE ) ) {
        DBG_PRINT( "msiEndFrame has failed.\n" );
        exit( 0 );
    }

    if ( !msiFreeTextureHeap( pTexture ) ) {
        DBG_PRINT( "msiFreeTextureHeap has failed.\n" );
        exit( 0 );
    }

}

void RenderZPlaneTest_noZ( ) {
    /*
    *  Demonstrates how to use: msiRenderZPlane, msiSetTextureTransparency
    *
    *  Note that in this case, the msiRenderZPlanes are still done but with no
    *  Z comparisons.
    */

    LPBYTE pTexture;
    T_msiZPlane       ZPlane;

    /* Allocate the texture heaps (# of 4K byte pages) to hold the bitmap.
    *  Calculation: (256*256*2bpp)/4096 = 32 */
    pTexture = msiAllocTextureHeap( 32 );
    if ( pTexture == NULL ) {
        DBG_PRINT( "msiAllocTextureHeap has failed.\n" );
        exit( 0 );
    }

    CopyBMPLinear( pTexture, &pBitmapA[0], TEXA_SIZE_X, TEXA_SIZE_Y );

    /* Start a new frame and clear both the RGB and Z buffers.
    *  The new frame is cleared to white and Z-Buffer to 1 (being furthest). */
    if ( !msiStartFrame( 1, 100.0f, 100.0f, 100.0f, 1, 1.0 ) ) {
        DBG_PRINT( "msiStartFrame has failed.\n" );
        exit( 0 );
    }

    SetDefaultParameters( &msiParameters );

    msiParameters.msiTexture.Width = TEXA_SIZE_X;
    msiParameters.msiTexture.Height = TEXA_SIZE_Y;
    msiParameters.msiTexture.pMem = pTexture;
    msiParameters.msiTexture.pHeap = pTexture;
    msiParameters.msiTexture.CacheOffset = TEXA_OFFSET;
    msiParameters.msiTexture.Clamp_u = TRUE;
    msiParameters.msiTexture.Clamp_v = TRUE;
    if ( !msiSetParameters( &msiParameters ) ) {
        DBG_PRINT( "msiSetParameters has failed.\n" );
        exit( 0 );
    }

    /* Render a Zplane image */
    ZPlane.x = 10;
    ZPlane.y = 10;
    ZPlane.xWidth = 256;
    ZPlane.yHeight = 256;

    ZPlane.u = 0;
    ZPlane.uWidth = 256 - 1;
    ZPlane.v = 0;
    ZPlane.vHeight = 256 - 1;

    ZPlane.z = 0.5f;

    ZPlane.r = 255.0f;
    ZPlane.g = 255.0f;
    ZPlane.b = 255.0f;
    ZPlane.mr = 255.0f;
    ZPlane.mg = 255.0f;
    ZPlane.mb = 255.0f;

    msiRenderZPlane( &ZPlane, 100 );

    ZPlane.x = 255;
    ZPlane.y = 45;
    ZPlane.xWidth = 360;
    ZPlane.yHeight = 125;

    ZPlane.u = 0;
    ZPlane.uWidth = 256 - 1;
    ZPlane.v = 0;
    ZPlane.vHeight = 256 - 1;

    ZPlane.z = 0.7f;

    ZPlane.r = 255.0f;
    ZPlane.g = 255.0f;
    ZPlane.b = 255.0f;
    ZPlane.mr = 255.0f;
    ZPlane.mg = 0.0f;
    ZPlane.mb = 0.0f;

    if ( !msiSetTextureBlend( MODULATE ) ) {
        DBG_PRINT( "msiSetTextureBlend has failed.\n" );
        exit( 0 );
    }

    msiRenderZPlane( &ZPlane, 100 );

    /* Render a Zplane image */
    ZPlane.x = 200;
    ZPlane.y = 145;
    ZPlane.xWidth = 220;
    ZPlane.yHeight = 260;

    ZPlane.u = 0;
    ZPlane.uWidth = 256 - 1;
    ZPlane.v = 25;
    ZPlane.vHeight = 225;

    ZPlane.z = 0.6f;

    ZPlane.r = 255.0f;
    ZPlane.g = 255.0f;
    ZPlane.b = 0.0f;
    ZPlane.mr = 255.0f;
    ZPlane.mg = 255.0f;
    ZPlane.mb = 255.0f;

    if ( !msiSetTextureBlend( DECAL ) ) {
        DBG_PRINT( "msiSetTextureBlend has failed.\n" );
        exit( 0 );
    }

    /* Make 'white' the keying color for decal */
    if ( !msiSetTextureTransparency( 0xFFFFFFFF ) ) {
        DBG_PRINT( "msiSetTextureTransparency has failed.\n" );
        exit( 0 );
    }

    msiRenderZPlane( &ZPlane, 100 );

    /* TRUE renders the frame immediately. */
    if ( !msiEndFrame( 0, 0, TRUE ) ) {
        DBG_PRINT( "msiEndFrame has failed.\n" );
        exit( 0 );
    }

    if ( !msiFreeTextureHeap( pTexture ) ) {
        DBG_PRINT( "msiFreeTextureHeap has failed.\n" );
        exit( 0 );
    }

}

void BitBltTest( ) {
    /*
    *  Demonstrates how to use: msiBlitRect
    */

    int xx, yy;
    LPBYTE pTexture;
    T_msiMemoryStatus pMemStatus;

    /* Allocate the texture heaps (# of 4K byte pages) to hold the bitmap.
    *  The pitch of the Heap has to match that of the screen, hence 640.
    *  Calculation: 640*256*2bpp/4096 = 80 */
    pTexture = msiAllocTextureHeap( 80 );
    if ( pTexture == NULL ) {
        DBG_PRINT( "msiAllocTextureHeap has failed.\n" );
        exit( 0 );
    }

    /* Copy the previously loaded bitmaps to the Heap for use in a BlitRect */
    yy = 0;
    while ( yy < 256 * 2 ) {
        for ( xx = 0; xx < 256 * 2; xx++ ) {
            *( LPBYTE ) ( pTexture + 640 * yy + xx ) = pBitmapA[256 * yy + xx];
            yy++;
        }
    }

    if ( !msiStartFrame( 1, 100.0f, 100.0f, 100.0f, 0, 1.0 ) ) {
        DBG_PRINT( "msiStartFrame has failed.\n" );
        exit( 0 );
    }

    /* Place a bitblt at x=25,y=25, hence DestinationOff = (640*y+x)*2 = 32050 */
    msiBlitRect( pTexture, pTexture, &pMemStatus, 640 * 2, 16, TEXA_SIZE_X, TEXA_SIZE_Y, 32050, 0xFFFF, 0x0000 );
    while ( msiIsMemoryBusy( &pMemStatus ) ); /* Wait until the Blit has finished executing.
                                              *  Now just change the color keying on black
                                              *  Place a bitblt at x=360,y=25, hence DestinationOff = (640*y+x)*2=32720  */
    msiBlitRect( pTexture, pTexture, &pMemStatus, 640 * 2, 16, TEXA_SIZE_X, TEXA_SIZE_Y, 32720, 0x0, 0xFFFF );
    while ( msiIsMemoryBusy( &pMemStatus ) ); /* Wait until the Blit has finished executing. */

    /* TRUE renders the frame immediately. */
    if ( !msiEndFrame( 0, 0, TRUE ) ) {
        DBG_PRINT( "msiEndFrame has failed.\n" );
        exit( 0 );
    }

    if ( !msiFreeTextureHeap( pTexture ) ) {
        DBG_PRINT( "msiFreeTextureHeap has failed.\n" );
        exit( 0 );
    }

}

void main( ) {
    int c;

    pInfo = msiInit( Xres, Yres, 16, UseHardwareZ, msiDBG_DumpToFile );
    if ( !pInfo ) {
        printf( "msiInit has failed.\n" );
        exit( 0 );
    }

    /* Display the Matrox Splash Screen at the beginning.
    *  This could only be done at a specific point and it will overwrite the
    *  contents of the Texture Cache. */
    if ( !msiSplashScreen( ) ) {
        DBG_PRINT( "msiSplashScreen has failed.\n" );
        exit( 0 );
    }

    /* Load the Bitmaps that will be used in this application */
    if ( !BMPLoad( "logo.bmp", pBitmapA, TEXA_SIZE_X, 0, TEXA_SIZE_Y, 0, 0 ) ) {
        exit( 0 );
    }

    while ( ( c = getch( ) ) != VK_ESCAPE ) {
        switch ( c ) {
        case 'z':
        case 'Z':
            RenderZPlaneTest_Z( );
            break;
        case 'x':
        case 'X':
            RenderZPlaneTest_noZ( );
            break;
        case 'g':
        case 'G':
            GeneralStateChange( );
            break;
        case 'b':
        case 'B':
            BitBltTest( );
            break;
        default:
            break;
        }
    }

    msiExit( );

}
