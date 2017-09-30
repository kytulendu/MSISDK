//////////////////////////////////////////////////////////////////////////////
// msitutor.c																					 //
//																									 //
// This sample code demonstrates how the MSI95 API calls are made.			 //
// Please use the 'msi95.doc' document as a guide.								 	 //
//////////////////////////////////////////////////////////////////////////////

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <windows.h>

#include "msi95.h"

//
// Some defines
//
#define MSIDBG_MSG_PREFIX       "[msitutor] "
#define MSIDBG_LOG_PATH         "msitutor.log"

#define TRUE 		1
#define FALSE		0

#define TEXA_SIZE_X	256
#define TEXA_SIZE_Y	256
#define TEXB_SIZE_X	256
#define TEXB_SIZE_Y	256
#define TEXC_SIZE_X	256
#define TEXC_SIZE_Y	64
#define TEXD_SIZE_X	128
#define TEXD_SIZE_Y	128
#define TEXE_SIZE_X	128
#define TEXE_SIZE_Y	128
#define TEXF_SIZE_X	64
#define TEXF_SIZE_Y	64
#define TEXG_SIZE_X	128
#define TEXG_SIZE_Y	128
#define TEXH_SIZE_X	256
#define TEXH_SIZE_Y	256
#define LUT4A_OFFSET	0
#define LUT4B_OFFSET	LUT4A_OFFSET+32
#define TEXA_OFFSET	LUT4B_OFFSET+32
#define TEXB_OFFSET	TEXA_OFFSET+TEXA_SIZE_X*TEXA_SIZE_Y*2
#define TEXC_OFFSET	TEXB_OFFSET+TEXB_SIZE_X*TEXB_SIZE_Y*2
#define TEXD_OFFSET	TEXC_OFFSET+TEXC_SIZE_X*TEXC_SIZE_Y*2
#define TEXE_OFFSET	TEXD_OFFSET+TEXD_SIZE_X*TEXD_SIZE_Y*2
#define TEXF_OFFSET	TEXE_OFFSET+TEXE_SIZE_X*TEXE_SIZE_Y*2
#define TEXG_OFFSET	TEXF_OFFSET+TEXF_SIZE_X*TEXF_SIZE_Y*2
#define TEXH_OFFSET	TEXG_OFFSET+TEXG_SIZE_X*TEXG_SIZE_Y
#define LUT8A_OFFSET	TEXH_OFFSET+TEXH_SIZE_X*TEXH_SIZE_Y*2
#define LUT8B_OFFSET	LUT8A_OFFSET + 512

//
// Globals
//
BYTE pBitmapA[2*TEXA_SIZE_X*TEXA_SIZE_Y];
BYTE pBitmapB[2*TEXB_SIZE_X*TEXB_SIZE_Y];
BYTE pBitmapC[2*TEXC_SIZE_X*TEXC_SIZE_Y];
BYTE pBitmapD[2*TEXD_SIZE_X*TEXD_SIZE_Y];
BYTE pBitmapE[2*TEXE_SIZE_X*TEXE_SIZE_Y];
BYTE pBitmapF[2*TEXF_SIZE_X*TEXF_SIZE_Y];
BYTE pBitmapG[TEXG_SIZE_X*TEXG_SIZE_Y];
BYTE pBitmapH[2*TEXH_SIZE_X*TEXH_SIZE_Y];

T_msiVertex msiVertex[9];
T_msiParameters msiParameters;
T_msiInfo*  pInfo;   // MSI Direct Frame Buffer Access information structure.

LRESULT CALLBACK WindowProc ( HWND, UINT, WPARAM,	LPARAM );

// Used for msiInit
#define Xres 640
#define Yres 480
#define UseHardwareZ 0

//
// Functions
//

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DBG_PRINT																	 //
//																									 //
// DESCRIPTION:	Prints possible errors to a file									 //
//////////////////////////////////////////////////////////////////////////////
static void DBG_PRINT   (  char *pMessage, ...  )
{
   char     buffer[256]; 
   int      len;
   va_list  ap;
   FILE     *fd;

   va_start(ap, pMessage);

   strcpy(buffer, MSIDBG_MSG_PREFIX);
   len = vsprintf(buffer + strlen(MSIDBG_MSG_PREFIX), pMessage, ap);

   buffer[strlen(MSIDBG_MSG_PREFIX) + len]     = '\n';
   buffer[strlen(MSIDBG_MSG_PREFIX) + len + 1] = '\0';

   if ((fd = fopen(MSIDBG_LOG_PATH, "a")) != NULL)
    {
     fprintf(fd, buffer);
     fclose(fd);
    }
   
   va_end(ap);

} // DBG_PRINT

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	SetDefaultParameters													  	 //
//																									 //
// DESCRIPTION:	Initializes default values for calls to msiSetParameters	 //
//////////////////////////////////////////////////////////////////////////////
void SetDefaultParameters(T_msiParameters *msiParameters)
{
   msiParameters->msiTexture.Enable					= TRUE;
   msiParameters->msiTexture.Width					= 0;
   msiParameters->msiTexture.Height					= 0;
   msiParameters->msiTexture.Planes					= 16;
   msiParameters->msiTexture.pMem					= NULL;
   msiParameters->msiTexture.pHeap					= NULL;
   msiParameters->msiTexture.CacheOffset			= 0;          
   msiParameters->msiTexture.Clamp_u				= FALSE;
   msiParameters->msiTexture.Clamp_v				= FALSE;
   msiParameters->msiTexture.Modulate				= FALSE;
   msiParameters->msiTexture.Decal					= FALSE;
   msiParameters->msiTexture.Transparency			= FALSE;
   msiParameters->msiTexture.KeyingColor			= 0x0000;
   msiParameters->msiTexture.KeyingColorMask		= 0xFFFF;
   msiParameters->msiTexture.KeyingAlpha			= 0x0001;
   msiParameters->msiTexture.KeyingAlphaMask		= 0x0000;

   msiParameters->msiTexture.msiLUT.pMem			= NULL;
   msiParameters->msiTexture.msiLUT.pHeap			= NULL;
   msiParameters->msiTexture.msiLUT.CacheOffset	= 0;

   msiParameters->msiDepth.Enable					= FALSE;
   msiParameters->msiDepth.Compare					= msiCMP_LEQUAL;
   msiParameters->msiDepth.Protect					= FALSE;

   msiParameters->msiColor.Dither					= TRUE;
   msiParameters->msiColor.Protect					= FALSE;

} // SetDefaultParameters

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	AddxAddy																		 //
//																									 //
// DESCRIPTION:	Used to modify the vertices of a triangle					 	 //
//////////////////////////////////////////////////////////////////////////////
BOOL AddxAddy(T_msiVertex *pVertex,int from,int to,float xadd,float yadd)
{
   int i=0;

   if (from>to)
      return (FALSE);

   for (i=from;i<=to;i++)
   {
    pVertex[i].x+=xadd;
    pVertex[i].y+=yadd;
   }

   return (TRUE);

} // AddxAddy

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	BMPLoad																		 //
//																									 //
// DESCRIPTION:	Used to load BMPs from disk and into pBitMap					 //
//						The Pitch = 0 Linear													 //
//////////////////////////////////////////////////////////////////////////////
BOOL BMPLoad ( LPCSTR FileName,	// 32 bit ptr. to constant char str.
               BYTE *pBitMap,		// Pointer to where in mem. to load the BMP.
               LONG XSize,			// The Width of the BMP to be Loaded.
               LONG XOffset,		// The Upper Left Corner of BMP.
               LONG YSize,			// The Hight of the BMP to be Loaded.
               LONG YOffset,		// The Upper Left Corner of BMP.
               LONG Pitch			// The Pitch.
             )
{
   BITMAPFILEHEADER  BitMapFileHeader;   // The BitMap File Header.
   BITMAPINFOHEADER  BitMapInfoHeader;   // The BitMap Info Information.

   FILE* pFile;		// File Pointer.
   char  char_read;	// The character read from File.BMP
   LONG  off_start,	// End of actual bitmap data on file.
         xx, yy;		// Loop Variables in x,y directions.
   WORD  Color;	// Used to store the complete 16 Bit RGB Color.
   BOOL  Flag;
   long i;
   
   // File Does not exist in specified path.
   if ((pFile = fopen(FileName, "rb")) == NULL)
   {
    DBG_PRINT("BMPLoad Error: Can't open the source bitmap, File not found.");
    return(FALSE);
   } 
   
   // Loading the BMP File Header.
   if (fread(&BitMapFileHeader,sizeof(BitMapFileHeader),1,pFile)==NULL)
   {
    DBG_PRINT("BMPLoad Error: BMP File Header Corrupted.");
    return(FALSE);
   }

   // Loading the BMP Map Information Header 
   if (fread(&BitMapInfoHeader,sizeof(BitMapInfoHeader),1,pFile)==NULL)
   { 
    DBG_PRINT("BMPLoad Error: BMP Map Information Header Corrupted.");
    return(FALSE);
   }
   
   // Check that this is a Valid BMP File: 1st 6 Bytes must be "BM??00" 
   if ((BitMapFileHeader.bfType!=66+77*256)|(BitMapFileHeader.bfReserved1!=0)|(BitMapFileHeader.bfReserved2!=0))
   {
    DBG_PRINT("BMPLoad Error: Not a Valid BMP File.");
    return(FALSE);
   }
   
   // Check to see if file is 24Bits per Color.
   if ((BitMapInfoHeader.biBitCount!=24) /*&(BitMapInfoHeader.biBitCount!=16)&(BitMapInfoHeader.biBitCount!=8)*/)
   {
    DBG_PRINT("BMLoad Error: File NOT in Bits per Color.");
    DBG_PRINT("   - File is %d Bits per Color....",BitMapInfoHeader.biBitCount);
    return(FALSE);
   }

   // bfOffBits: Offset in Bytes, from the BITMAPFILEHEADER structure 
   // to the bitmap bits.
   fseek(pFile,0,SEEK_END); // Move the file pointer to the end of the file.
   off_start=ftell(pFile);  // Get the current position of the file pointer.

   // Check for invalid XSize.
   i=1;
   Flag=FALSE;
   for (xx=1; xx<17; xx++) 
   {
    if (XSize==i) Flag=TRUE;  
      i*=2;
   }
   if (Flag==FALSE) 
   {
    DBG_PRINT("BMPLoad Error: Invalid XSize Parameter %d (Must be a Power of 2).",XSize);
    return (FALSE);
   }

   // Check for invalid YSize.
   i=1;
   Flag=FALSE;
   for (yy=1; yy<17; yy++) 
   {
    if (XSize==i) Flag=TRUE;  
      i*=2;
   }
   if (Flag==FALSE) 
   {
    DBG_PRINT("BMPLoad Error: Invalid YSize Parameter %d (Must be a Power of 2).", YSize);
    return (FALSE);
   }

   // Check for Invalid XOfsset, XSize Parameters.
   if (XOffset+XSize>BitMapInfoHeader.biWidth)
   {
    DBG_PRINT("BMPLoad Error: Access Beyond BMP Width.");
    DBG_PRINT("XOffset + XSize = %d + %d = %d, Is Greater than BMP Width: %d",XOffset,XSize,XOffset+XSize,BitMapInfoHeader.biWidth);
    return(FALSE);   
   }

   // Check for Invalid YOffset, YSize Parameters.
   if (YOffset+YSize>BitMapInfoHeader.biHeight)
   {
    DBG_PRINT("BMPLoad Error: Access Beyond BMP Height.");
    DBG_PRINT("YOffset + YSize = %d + %d = %d, Is Greater than BMP Height: %d",YOffset,YSize,YOffset+YSize,BitMapInfoHeader.biHeight);
    return(FALSE);   
   }

   // Check if BMP File is RLE compressed.
   if (BitMapInfoHeader.biCompression!=BI_RGB)
   {
    DBG_PRINT("BMPLoad Error: BMP File is in Compressed Format:"); 
    switch(BitMapInfoHeader.biCompression)
    {
     case BI_RLE8: DBG_PRINT("	A run-length encoded (RLE) format for bitmaps with 8 bits per pixel."); break;
     case BI_RLE4: DBG_PRINT("    An RLE format for bitmaps with 4 bits per pixel."); break;
     case BI_BITFIELDS: DBG_PRINT("   Color table consists of three doubleword color masks per RGB Triplet."); break; 
    } // switch
    return(FALSE);
   }

   // If (Pitch==0) Do a Linear BMP Load from bfOffBits. 
   // fseek(pFile,BitMapFileHeader.bfOffBits,SEEK_SET);
   i=0;
/*
   // Check if the File Lengths match.
   // This makes sure that none of the RGB Triplets are missing, i.e. that the file data is complete.
   if ((off_start-((unsigned)BitMapInfoHeader.biWidth
       * (unsigned)BitMapInfoHeader.biHeight * 3))!=BitMapFileHeader.bfOffBits)
   {
    DBG_PRINT("BMPLoad Error: BMP File corrupted.");
    DBG_PRINT("   - BitMapFileHeader.bfOffBits = %d",BitMapFileHeader.bfOffBits);
    DBG_PRINT("   - Off_Start .... = %d",off_start-(BitMapInfoHeader.biWidth
       * BitMapInfoHeader.biHeight * 3));
    DBG_PRINT("Dimensions %d x %d",(unsigned)BitMapInfoHeader.biWidth,(unsigned)BitMapInfoHeader.biHeight);
    return(FALSE);
   }
*/
   // Read the File 
   for (yy=0; yy<YSize; yy++)
   {
    fseek(pFile,((XOffset*3)+BitMapFileHeader.bfOffBits+((BitMapInfoHeader.biHeight-yy-YOffset-1)*BitMapInfoHeader.biWidth*3)),SEEK_SET);
    for (xx=0; xx<XSize; xx++)
    { 
     // read as BGR
     // Initialize color to 0, read the colors (1 byte each), shift the wanted bits to 
     // correct equivalent position in color, mask out unwanted bits with & then add
     // the 3 parts together with or to obtain the final 16 bit color.
     Color=0;
        
     fscanf(pFile,"%c",&char_read);      // BLUE
     Color|=((char_read>>3&0x001F));     // BLUE  // 0000 0000 0001 1111
     fscanf(pFile,"%c",&char_read);      // GREEN
     Color|=((char_read>>2<<5)&0x07E0);  // GREEN // 0000 0111 1110 0000
     fscanf(pFile,"%c",&char_read);      // RED
     Color|=((char_read>>3<<11)&0xF800); // RED   // 1111 1000 0000 0000

     if (Pitch!=0)
		 // BITMAP IS BOTTOM UP
       ((WORD *)pBitMap)[((/*(Ysize-1)-*/yy)*Pitch)+xx] = Color;
     else
		 ((WORD *)pBitMap)[i++] = Color;
    } // for
   } // yy

   return (TRUE);

} // BMPLoad

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	CopyBMPLinear																 //
//																									 //
// DESCRIPTION:	Used to copy data from main memory to a Heap or directly	 //
//						to a valid MSI pInfo pointer (Direct Frame Buffer Access) //
//////////////////////////////////////////////////////////////////////////////
void CopyBMPLinear(LPBYTE pTexture, BYTE *pBitmap, int Width, int Height)
{
	// pTexture:	A pointer to the start of the Video card's memory (Color,
	//					Depth, or Texture Cache) or to a valid MSI Heap

	// pBitmap:		A pointer to the bitmap residing in main memory

	// The bitmap data at pBitmap is moved into the destination - linearly.

	// If copying to a MSI pInfo pointer (Direct Frame Buffer Access), a call
	// to msiSetParameters(NULL) was previously necessary. This BREAKS the
	// BUS-MASTERING which reduces performance, hence this method should only
	// be used when frame rates are not critical!  A non critical case would be
	//	at very the beginning of a game or at the beginning of a game level.

	int xx, yy;

	// Copy the bytes linearly - assumes 16bpp textures
	for (yy=0;yy<Height;yy++)
   	for (xx=0;xx<Width;xx++)
			((WORD *)pTexture)[xx+(yy*Width)] = ((WORD*)pBitmap)[xx+(yy*Width)];

} // CopyBMPLinear

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	MakePic4																		 //
//																									 //
// DESCRIPTION:	Used for the ShowLUT4 test. This function creates a		 //
//						simple PseudoColor image with four different quadrants,	 //
//						each having a different offset in the LUT.					 //
//																									 //
//						********************													 //
//						*        *         *													 //
//						*        *         *													 //
//						*   0    *    1    *													 //
//						*        *         *													 //
//						*        *         *													 //
//						********************													 //
//						*        *         *													 //
//						*        *         *													 //
//						*   2    *    3    *													 //
//						*        *         *													 //
//						*        *         *													 //
//						********************													 //
//////////////////////////////////////////////////////////////////////////////
void MakePic4(LPBYTE pBitmap,LONG xsize,LONG ysize)
{
	int i, j;

	// xsize is divided by two since there are two nibbles (4 bits)
	// in each byte.  
	
	// Top half
	for (i = 0; i < ysize/2; i++)
	{
 	 // Left half
	 for (j = 0; j < xsize/4; j++)
	 	pBitmap[i*xsize/2 + j] = 0x00; // Quadrant 0

 	 // Right half
	 for (j = xsize/4; j < xsize/2; j++)
	 	pBitmap[i*xsize/2 + j] = 0x11; // Quadrant 1
	}

	// Bottom half
	for (i = ysize/2; i < ysize; i++)
	{
	 // Left half
	 for (j = 0; j < xsize/4; j++)
	 	pBitmap[i*xsize/2 + j] = 0x22; // Quadrant 2

	 // Right half
 	 for (j = xsize/4; j < xsize/2; j++)
	 	pBitmap[i*xsize/2 + j] = 0x33; // Quadrant 3
	}

} // MakePic4


//
// Tests
//

#define pt(x,y) ((x&0x0000ffff)|((y&0xffff)<<16))

void DrawLines()
{
	//
   // Demonstrates how to use: msiDrawSingleLine, msiStartFrame, msiEndFrame
	//

	DWORD color;
	DWORD XYStart, XYEnd, LineStyle;

	// Start a new frame and clear the screen to the specified RGB color
	if (!msiStartFrame(1,100.0f,100.0f,100.0f,0,1.0)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

	// Red in 5:6:5 format
	color = 0xf800;
	XYStart = pt(20, 200);
	XYEnd = pt(620, 200);
	LineStyle = 0xffff0000;

	if (!msiDrawSingleLine(color, XYStart, XYEnd, LineStyle))
   {
    DBG_PRINT("msiDrawSingleLine has failed.\n");
    exit(0);
   }

	// Green in 5:6:5 format
	color = 0x07e0;
	XYStart = pt(20, 240);
	XYEnd = pt(620, 240);
	LineStyle = 0xf0f0f0f0;

	if (!msiDrawSingleLine(color, XYStart, XYEnd, LineStyle))
   {
    DBG_PRINT("msiDrawSingleLine has failed.\n");
    exit(0);
   }

	// Blue in 5:6:5 format
	color = 0x001f;
	XYStart = pt(20, 280);
	XYEnd = pt(620, 280);
	LineStyle = 0xffffffff;

	if (!msiDrawSingleLine(color, XYStart, XYEnd, LineStyle))
   {
    DBG_PRINT("msiDrawSingleLine has failed.\n");
    exit(0);
   }

	// Note that the TRUE parameter renders the frame immediately.  For the
	// purpose of this application, we want to force the drawing to be finished
	// immediately so as to view the frame after a keystroke.  In the case of
	// a game, this parameter would be set to FALSE.  The FALSE would allow the
	// BUS-MASTERING to continue until the beginning of the next msiStartFrame.
	if (!msiEndFrame(0, 0, TRUE))
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

} // DrawLines

void CacheTexturesFromHeap()
{
	//
	// Demonstrates how to use:	msiAllocTextureHeap, msiFreeTextureHeap,
	//										msiSetTextureOffset
	// Demonstrates how to: 		Load textures in the Texture Cache via a
	//										Heap method.  This method takes advantage of
	//										BUS-MASTERING.  Also shows how to draw some
	//										simple Texture Mapped triangles.
	//

	LPBYTE pTexture; // This will point to the HEAP memory that will be used
							// to place the textures in the Texture Cache.

	if (!msiStartFrame(1,200.0f,0.0f,0.0f,1,0.5)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

   // Allocate the texture heaps (# of 4K byte pages) to hold the bitmaps.
	// We want enough to hold the two bitmaps for the first two textures
	// Calculation: 2 * 256*256*2bpp/4096 = 64
	pTexture = msiAllocTextureHeap(64);
	if (pTexture==NULL)
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

   // Copy the previously loaded bitmaps to the Heap
   CopyBMPLinear(pTexture,&pBitmapA[0],TEXA_SIZE_X,TEXA_SIZE_Y); 
	// The second bitmap will be placed in the Heap; right after the first one.
   CopyBMPLinear(pTexture+TEXA_SIZE_X*TEXA_SIZE_Y*2,&pBitmapB[0],TEXA_SIZE_X,TEXA_SIZE_Y); 

	// Set up the structure to pass to msiSetParameters 
	SetDefaultParameters(&msiParameters);

	msiParameters.msiTexture.Width         		= TEXA_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXA_SIZE_Y;
	msiParameters.msiTexture.pMem          		= pTexture;
	msiParameters.msiTexture.pHeap         		= pTexture;
	msiParameters.msiTexture.CacheOffset   		= TEXA_OFFSET;

	// This will load the first texture into the Texture Cache
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	msiParameters.msiTexture.pMem          		= pTexture+TEXA_SIZE_X*TEXA_SIZE_Y*2;
	msiParameters.msiTexture.CacheOffset   		= TEXB_OFFSET;
	// This will load the second texture into the Texture Cache
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	// Set the vertices to pass to msiRenderTriangle
	msiVertex[0].x = 40.0f;		msiVertex[0].y = 100.0f;	msiVertex[0].z		= 1.0f;
   msiVertex[0].u = 0.0f;		msiVertex[0].v = 0.0f;		msiVertex[0].invW = 1.0f;
	msiVertex[1].x = 40+256;	msiVertex[1].y = 100.0f;	msiVertex[1].z		= 1.0f;
   msiVertex[1].u = 1.0f;		msiVertex[1].v = 0.0f;		msiVertex[1].invW = 1.0f;
	msiVertex[2].x = 40+256;	msiVertex[2].y = 100+256;	msiVertex[2].z		= 1.0f;
   msiVertex[2].u = 1.0f;		msiVertex[2].v = 1.0f;		msiVertex[2].invW = 1.0f;
	msiVertex[3].x = 40.0f;		msiVertex[3].y = 100+256;	msiVertex[3].z		= 1.0f;
   msiVertex[3].u = 0.0f;		msiVertex[3].v = 1.0f;		msiVertex[3].invW = 1.0f;

   // Set the source texture to the first one
	if (!msiSetTextureOffset(TEXA_OFFSET))
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

   // Set the source texture to the second one
   if (!msiSetTextureOffset(TEXB_OFFSET))
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiEndFrame(0, 0, TRUE))  // TRUE renders the frame immediately.
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

	// Free the allocated texture Heap
	if (!msiFreeTextureHeap(pTexture))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

} // CacheTexturesFromHeap

void CacheTexturesFromMem()
{
	//
	// Demonstrates how to use:	msiSetTextureOffset
	// Demonstrates how to: 		Load textures in the Texture Cache via a
	//										System Memory method.  This method DOES NOT
	//										take advantage of BUS-MASTERING.  Also shows
	//										how to draw some simple Texture Mapped
	//										triangles.

	if (!msiStartFrame(1,0.0f,200.0f,0.0f,1,0.5)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

	// Set up the structure to pass to msiSetParameters 
	SetDefaultParameters(&msiParameters);

	msiParameters.msiTexture.Width         		= TEXA_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXA_SIZE_Y;
	msiParameters.msiTexture.pMem          		= pBitmapA;
	msiParameters.msiTexture.pHeap         		= 0;
	msiParameters.msiTexture.CacheOffset   		= TEXA_OFFSET;

	// This will load the first texture into the Texture Cache
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	msiParameters.msiTexture.pMem          		= pBitmapB;
	msiParameters.msiTexture.CacheOffset   		= TEXB_OFFSET;
	// This will load the second texture into the Texture Cache
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	// Set the vertices to pass to msiRenderTriangle
	msiVertex[0].x	 = 40.0f;	msiVertex[0].y	 = 100.0f;	msiVertex[0].z		= 1.0f;
	msiVertex[0].mr = 1.0f;		msiVertex[0].mg = 1.0f;		msiVertex[0].mb	= 1.0f;
   msiVertex[0].u	 = 0.0f;		msiVertex[0].v	 = 0.0f;		msiVertex[0].invW = 1.0f;
	msiVertex[1].x	 = 40+256;	msiVertex[1].y	 = 100.0f;	msiVertex[1].z		= 1.0f;
	msiVertex[1].mr = 1.0f;		msiVertex[1].mg = 1.0f;		msiVertex[1].mb	= 1.0f;
   msiVertex[1].u	 = 1.0f;		msiVertex[1].v	 = 0.0f;		msiVertex[1].invW = 1.0f;
	msiVertex[2].x	 = 40+256;	msiVertex[2].y	 = 100+256;	msiVertex[2].z		= 1.0f;
	msiVertex[2].mr = 1.0f;		msiVertex[2].mg = 1.0f;		msiVertex[2].mb	= 1.0f;
   msiVertex[2].u	 = 1.0f;		msiVertex[2].v	 = 1.0f;		msiVertex[2].invW = 1.0f;
	msiVertex[3].x	 = 40.0f;	msiVertex[3].y	 = 100+256;	msiVertex[3].z		= 1.0f;
	msiVertex[3].mr = 1.0f;		msiVertex[3].mg = 1.0f;		msiVertex[3].mb	= 1.0f;
   msiVertex[3].u	 = 0.0f;		msiVertex[3].v	 = 1.0f;		msiVertex[3].invW = 1.0f;

   if (!msiSetTextureOffset(TEXA_OFFSET))
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

   if (!msiSetTextureOffset(TEXB_OFFSET))
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiEndFrame(0, 0, TRUE))  // TRUE renders the frame immediately.
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

} // CacheTexturesFromMem

void CacheTexturesDirectly()
{
	//
	// Demonstrates how to use:	msiSetTextureOffset, msiSetTextureEnable
	// Demonstrates how to: 		Load textures in the Texture Cache via a
	//										Direct Frame Buffer Access method.  This
	//										method DOES NOT take advantage of
	//										BUS-MASTERING.  Furthermore, this method
	//										BREAKS BUS-MASTERING if it is used when an
	//										animation sequence is taking place.  Also
	//										shows how to draw some simple Texture Mapped
	//										triangles.

	if (!msiStartFrame(1,0.0f,0.0f,200.0f,1,0.5)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

	// Enter Direct Frame Buffer Access mode
   if (!msiSetParameters(NULL)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }
   // The pInfo->???? parameters are now valid
	// This BREAKS BUS-MASTERING which reduces performance, hence
	// this method should only be used when frame rates are not critical!
	// A non critical case would be at very the beginning of a game or at the
	// beginning of a game level.

   CopyBMPLinear(pInfo->msiTexture.pMem+TEXA_OFFSET,&pBitmapA[0],TEXA_SIZE_X,TEXA_SIZE_Y); 
   CopyBMPLinear(pInfo->msiTexture.pMem+TEXB_OFFSET,&pBitmapB[0],TEXB_SIZE_X,TEXB_SIZE_Y); 

	// Set up the structure to pass to msiSetParameters 
	SetDefaultParameters(&msiParameters);

	msiParameters.msiTexture.Width         		= TEXA_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXA_SIZE_Y;

   // Exit Direct Frame Buffer Access mode and set valid rendering parameters
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	// Set the vertices to pass to msiRenderTriangle
	msiVertex[0].x	 = 40.0f;	msiVertex[0].y	 = 100.0f;	msiVertex[0].z		= 1.0f;
	msiVertex[0].mr = 1.0f;		msiVertex[0].mg = 1.0f;		msiVertex[0].mb	= 1.0f;
   msiVertex[0].u	 = 0.0f;		msiVertex[0].v	 = 0.0f;		msiVertex[0].invW = 1.0f;
	msiVertex[1].x	 = 40+256;	msiVertex[1].y	 = 100.0f;	msiVertex[1].z		= 1.0f;
	msiVertex[1].mr = 1.0f;		msiVertex[1].mg = 1.0f;		msiVertex[1].mb	= 1.0f;
   msiVertex[1].u	 = 1.0f;		msiVertex[1].v	 = 0.0f;		msiVertex[1].invW = 1.0f;
	msiVertex[2].x	 = 40+256;	msiVertex[2].y	 = 100+256;	msiVertex[2].z		= 1.0f;
	msiVertex[2].mr = 1.0f;		msiVertex[2].mg = 1.0f;		msiVertex[2].mb	= 1.0f;
   msiVertex[2].u	 = 1.0f;		msiVertex[2].v	 = 1.0f;		msiVertex[2].invW = 1.0f;
	msiVertex[3].x	 = 40.0f;	msiVertex[3].y	 = 100+256;	msiVertex[3].z		= 1.0f;
	msiVertex[3].mr = 1.0f;		msiVertex[3].mg = 1.0f;		msiVertex[3].mb	= 1.0f;
   msiVertex[3].u	 = 0.0f;		msiVertex[3].v	 = 1.0f;		msiVertex[3].invW = 1.0f;

   if (!msiSetTextureOffset(TEXA_OFFSET))
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

   if (!msiSetTextureOffset(TEXB_OFFSET))
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiEndFrame(0, 0, TRUE))  // TRUE renders the frame immediately.
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

} // CacheTexturesDirectly

void TexturingTypes()
{
	//
   // Demonstrates how to use:	msiSetTextureBlend
   // Demonstrates how to:  		Draw Texture Mapped triangles with Decal,
	//										Transparency and Modulation.  Also draw a
	//										triangle with Opacity.  This screen door
	//										transparency effect is often used in shadows.

	if (!msiStartFrame(1,100.0f,100.0f,100.0f,0,1.0)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

	// Set up the structure to pass to msiSetParameters 
	SetDefaultParameters(&msiParameters);

	msiParameters.msiTexture.Width         		= TEXC_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXC_SIZE_Y;
	// Note that since we are not concerned with frame rate in an application
	// such as this, we will load the texture in the Texture Cache via a
	//	System Memory method.  Otherwise, it is important to use the Heap 
	// method for the best frame rate.
	msiParameters.msiTexture.pMem          		= pBitmapC;
	msiParameters.msiTexture.pHeap         		= 0;
	msiParameters.msiTexture.CacheOffset   		= TEXC_OFFSET;

	// Load the first texture into the Texture Cache 
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	msiParameters.msiTexture.pMem          		= pBitmapD;
	msiParameters.msiTexture.CacheOffset   		= TEXD_OFFSET;
	// Load the second texture into the Texture Cache 
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	// Set the vertices to pass to msiRenderTriangle
	msiVertex[0].x	 = 40.0f;	msiVertex[0].y	 = 5.0f;		msiVertex[0].z		= 1.0f;
	msiVertex[0].r	 = 255.0f;	msiVertex[0].g  = 0.0f;		msiVertex[0].b		= 0.0f;
	msiVertex[0].mr = 255.0f;	msiVertex[0].mg = 255.0f;	msiVertex[0].mb	= 255.0f;
   msiVertex[0].u	 = 0.0f;		msiVertex[0].v	 = 0.0f;		msiVertex[0].invW = 1.0f;
	msiVertex[1].x	 = 40+256;	msiVertex[1].y	 = 5.0f;		msiVertex[1].z		= 1.0f;
	msiVertex[1].mr = 0.0f;		msiVertex[1].mg = 255.0f;	msiVertex[1].mb	= 0.0f;
	msiVertex[1].r  = 0.0f;		msiVertex[1].g  = 255.0f;	msiVertex[1].b		= 0.0f;
   msiVertex[1].u	 = 1.0f;		msiVertex[1].v	 = 0.0f;		msiVertex[1].invW = 1.0f;
	msiVertex[2].x	 = 40+256;	msiVertex[2].y	 = 5+64;		msiVertex[2].z		= 1.0f;
	msiVertex[2].mr = 0.0f;		msiVertex[2].mg = 255.0f;	msiVertex[2].mb	= 0.0f;
	msiVertex[2].r  = 255.0f;	msiVertex[2].g  = 255.0f;	msiVertex[2].b		= 0.0f;
   msiVertex[2].u	 = 1.0f;		msiVertex[2].v	 = 1.0f;		msiVertex[2].invW = 1.0f;
	msiVertex[3].x	 = 40.0f;	msiVertex[3].y	 = 5+64;		msiVertex[3].z		= 1.0f;
	msiVertex[3].mr = 255.0f;	msiVertex[3].mg = 255.0f;	msiVertex[3].mb	= 255.0f;
	msiVertex[3].r  = 0.0f;		msiVertex[3].g  = 255.0f;	msiVertex[3].b	   = 0.0f;
   msiVertex[3].u	 = 0.0f;		msiVertex[3].v	 = 1.0f;		msiVertex[3].invW = 1.0f;

	msiVertex[4].x	 = 100.0f;	msiVertex[4].y	 = 220.0f;	msiVertex[4].z		= 1.0f;
	msiVertex[4].r	 = 255.0f;	msiVertex[4].g  = 0.0f;		msiVertex[4].b		= 0.0f;
	msiVertex[4].mr = 0.0f;		msiVertex[4].mg = 0.0f;		msiVertex[4].mb	= 0.0f;
   msiVertex[4].u	 = 0.0f;		msiVertex[4].v	 = 0.0f;		msiVertex[4].invW = 1.0f;
	msiVertex[5].x	 = 100+128;	msiVertex[5].y	 = 220.0f;	msiVertex[5].z		= 1.0f;
	msiVertex[5].mr = 0.0f;		msiVertex[5].mg = 0.0f;		msiVertex[5].mb	= 0.0f;
	msiVertex[5].r  = 0.0f;		msiVertex[5].g  = 255.0f;	msiVertex[5].b		= 0.0f;
   msiVertex[5].u	 = 1.0f;		msiVertex[5].v	 = 0.0f;		msiVertex[5].invW = 1.0f;
	msiVertex[6].x	 = 100+128;	msiVertex[6].y	 = 220+128;	msiVertex[6].z		= 1.0f;
	msiVertex[6].mr = 255.0f;	msiVertex[6].mg = 255.0f;	msiVertex[6].mb	= 255.0f;
	msiVertex[6].r  = 255.0f;	msiVertex[6].g  = 255.0f;	msiVertex[6].b		= 0.0f;
   msiVertex[6].u	 = 1.0f;		msiVertex[6].v	 = 1.0f;		msiVertex[6].invW = 1.0f;
	msiVertex[7].x	 = 100.0f;	msiVertex[7].y	 = 220+128;	msiVertex[7].z		= 1.0f;
	msiVertex[7].mr = 255.0f;	msiVertex[7].mg = 255.0f;	msiVertex[7].mb	= 255.0f;
	msiVertex[7].r  = 0.0f;		msiVertex[7].g  = 255.0f;	msiVertex[7].b	   = 0.0f;
   msiVertex[7].u	 = 0.0f;		msiVertex[7].v	 = 1.0f;		msiVertex[7].invW = 1.0f;

   if (!msiSetTextureOffset(TEXC_OFFSET)) 
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Now enable transparency, and don't reload the texture since it is
	// already present in the Texture Cache.
	msiParameters.msiTexture.pMem          		= 0;
	msiParameters.msiTexture.CacheOffset   		= TEXC_OFFSET;
	msiParameters.msiTexture.Transparency  		= TRUE;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Now disable transparency and enable decal
	msiParameters.msiTexture.Transparency  		= FALSE;
	msiParameters.msiTexture.Decal			  		= TRUE;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,-300,70);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Now do the texture modulation and turn off decal
   if (!msiSetTextureBlend(MODULATE)) 
   {
    DBG_PRINT("msiSetTextureBlend has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Now enable transparency and modulation
	msiParameters.msiTexture.Transparency  		= TRUE;
	msiParameters.msiTexture.Modulate 				= TRUE;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	// Modify some modulation colors
	msiVertex[0].mr = 255.0f;	msiVertex[0].mg = 0.0f;		msiVertex[0].mb = 0.0f;
   msiVertex[1].mr = 255.0f;	msiVertex[1].mg = 255.0f;	msiVertex[1].mb = 255.0f;
	msiVertex[2].mr = 255.0f;	msiVertex[2].mg = 255.0f;	msiVertex[2].mb = 255.0f;
	msiVertex[3].mr = 255.0f;	msiVertex[3].mg = 0.0f;		msiVertex[3].mb = 0.0f;
	
 	AddxAddy(msiVertex,0,3,-300,70);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Modify some Gouraud Shading colors
	msiVertex[0].r = 0.0f;	msiVertex[0].g = 0.0f;	msiVertex[0].b	= 0.0f;
	msiVertex[1].r = 0.0f;	msiVertex[1].g = 0.0f;	msiVertex[1].b	= 0.0f;
	msiVertex[2].r = 0.0f;	msiVertex[2].g = 0.0f;	msiVertex[2].b = 0.0f;
	msiVertex[3].r = 0.0f;	msiVertex[3].g = 0.0f;	msiVertex[3].b = 0.0f;

	//  Turn off texture mapping
	if (!msiSetTextureEnable(FALSE))
   {
    DBG_PRINT("msiSetTextureEnable has failed.\n");
    exit(0);
   }

   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 50)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 50)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Now disable transparency and enable decal and modulation
	msiParameters.msiTexture.Transparency  		= FALSE;
	msiParameters.msiTexture.Modulate		 		= TRUE;
	msiParameters.msiTexture.Decal			  		= TRUE;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	// Modify some modulation colors
	msiVertex[0].mr = 255.0f;	msiVertex[0].mg = 255.0f;	msiVertex[0].mb = 0.0f;
   msiVertex[1].mr = 255.0f;	msiVertex[1].mg = 0.0f;		msiVertex[1].mb = 255.0f;
	msiVertex[2].mr = 255.0f;	msiVertex[2].mg = 255.0f;	msiVertex[2].mb = 255.0f;
	msiVertex[3].mr = 255.0f;	msiVertex[3].mg = 255.0f;	msiVertex[3].mb = 255.0f;

	// Modify some Gouraud Shading colors
	msiVertex[0].r = 255.0f;msiVertex[0].g = 255.0f;msiVertex[0].b	= 255.0f;
   msiVertex[1].r = 0.0f;	msiVertex[1].g = 0.0f;	msiVertex[1].b	= 0.0f;
	msiVertex[2].r = 255.0f;msiVertex[2].g = 255.0f;msiVertex[2].b	= 255.0f;
	msiVertex[3].r = 0.0f;	msiVertex[3].g = 0.0f;	msiVertex[3].b	= 0.0f;

 	AddxAddy(msiVertex,0,3,300,0);
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	SetDefaultParameters(&msiParameters);

	msiParameters.msiTexture.Width         		= 128;
	msiParameters.msiTexture.Height        		= 128;
	msiParameters.msiTexture.pMem          		= 0;
	msiParameters.msiTexture.CacheOffset   		= TEXD_OFFSET;
	msiParameters.msiTexture.pHeap         		= 0;

   // Setup parameters for the second image
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Now turn on the texture modulation and turn on transparency
	msiParameters.msiTexture.Modulate 				= TRUE;
	msiParameters.msiTexture.Transparency  		= TRUE;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

 	AddxAddy(msiVertex,4,7,145,0);
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Modify some modulation colors
	msiVertex[4].mr = 0.0f;		msiVertex[4].mg = 0.0f;		msiVertex[4].mb = 255.0f;
	msiVertex[5].mr = 0.0f;		msiVertex[5].mg = 0.0f;		msiVertex[5].mb = 255.0f;

 	AddxAddy(msiVertex,4,7,145,0);
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 25)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 25)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Modify some modulation colors
	msiVertex[4].mr = 255.0f;	msiVertex[4].mg = 255.0f;	msiVertex[4].mb = 255.0f;
	msiVertex[5].mr = 255.0f;	msiVertex[5].mg = 255.0f;	msiVertex[5].mb = 255.0f;
	msiVertex[6].mr = 255.0f;	msiVertex[6].mg = 0.0f;		msiVertex[6].mb = 0.0f;
	msiVertex[7].mr = 255.0f;	msiVertex[7].mg = 0.0f;		msiVertex[7].mb = 0.0f;

   // Add the decal and remove transparency
	msiParameters.msiTexture.Transparency  		= FALSE;
	msiParameters.msiTexture.Decal			  		= TRUE;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

 	AddxAddy(msiVertex,4,7,-290,130);
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Change the Gouraud Shaded colors that will show up with the decal
	msiVertex[4].r = 255.0f;	msiVertex[4].g = 255.0f;	msiVertex[4].b	= 255.0f;
	msiVertex[5].r = 255.0f;	msiVertex[5].g = 255.0f;	msiVertex[5].b	= 255.0f;
	msiVertex[6].r = 255.0f;	msiVertex[6].g = 0.0f;		msiVertex[6].b	= 0.0f;
	msiVertex[7].r = 255.0f;	msiVertex[7].g = 0.0f;		msiVertex[7].b	= 0.0f;

   // Set decal only and no modulation
	if (!msiSetTextureBlend(DECAL)) 
   {
    DBG_PRINT("msiSetTextureBlend has failed.\n");
    exit(0);
   }

 	AddxAddy(msiVertex,4,7,145,0);
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Modify some modulation colors
	msiVertex[4].mr = 0.0f;		msiVertex[4].mg = 255.0f;	msiVertex[4].mb = 0.0f;
	msiVertex[5].mr = 0.0f;		msiVertex[5].mg = 0.0f;		msiVertex[5].mb = 255.0f;
	msiVertex[6].mr = 0.0f;		msiVertex[6].mg = 0.0f;		msiVertex[6].mb = 255.0f;
	msiVertex[7].mr = 0.0f;		msiVertex[7].mg = 255.0f;	msiVertex[7].mb = 0.0f;

   // Do modulation only
	msiParameters.msiTexture.Transparency  		= FALSE;
	msiParameters.msiTexture.Decal			  		= FALSE;
	msiParameters.msiTexture.Modulate		  		= TRUE;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

 	AddxAddy(msiVertex,4,7,145,0);
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
   if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiEndFrame(0, 0, TRUE))  // TRUE renders the frame immediately.
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

} // TexturingTypes

void ShowLUT4()
{
	//
   // Demonstrates how to use:	msiSetTextureCLUT4
   // Demonstrates how to:			Use CLUT4 textures, and load LUT4 tables into
	//										the Texture Cache.
	//

	// Part A shows how a LUT can be loaded into the Texture Cache
	// The result of Part A is shown on the screen in Row 1
	//
	// Part B shows how a LUT that is already in the Texture Cache can be
	// selected and assigned an index value.
	// The result of Part B is shown on the screen in Row 2
	//
	// Part C shows how the index value can be used to easily select which LUT
	// is to be loaded inside the drawing engine and to then be used.  
	// The result of Part C is shown on the screen in Row 3
	//
	// LUTs can be used for textures that are identical, except for their color.

	LPBYTE pTexture; // used to load the image
	LPBYTE pLUT1; // used to create LUT1
	LPBYTE pLUT2; // used to create LUT2

	if (!msiStartFrame(1,100.0f,100.0f,100.0f,0,0.0)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

	// *** Set the vertices for the triangles to be rendered ***
	msiVertex[0].x = 125.0f;msiVertex[0].y = 20.0f;	msiVertex[0].z    = 0.4f;
	msiVertex[0].u = 0.0f;	msiVertex[0].v = 0.0f;	msiVertex[0].invW = 1.0f;
	msiVertex[1].x = 125+64;msiVertex[1].y = 20.0f;	msiVertex[1].z    = 0.4f;
	msiVertex[1].u = 1.0f;	msiVertex[1].v = 0.0f;	msiVertex[1].invW = 1.0f;
	msiVertex[2].x = 125+64;msiVertex[2].y = 20+64;	msiVertex[2].z    = 0.4f;
	msiVertex[2].u = 1.0f;	msiVertex[2].v = 1.0f;	msiVertex[2].invW = 1.0f;
	msiVertex[3].x = 125.0f;msiVertex[3].y = 20+64;	msiVertex[3].z    = 0.4f;
	msiVertex[3].u = 0.0f;	msiVertex[3].v = 1.0f;	msiVertex[3].invW = 1.0f;

	// *** Allocate the texture Heaps needed ***
	if (!(pTexture = msiAllocTextureHeap(1))) 
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

	if (!(pLUT1 = msiAllocTextureHeap(1))) 
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

	if (!(pLUT2 = msiAllocTextureHeap(1))) 
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

	// *** Make the LUTs ***
	// LUT 1
	((WORD *)pLUT1)[0] = 0xffff; // white
	((WORD *)pLUT1)[1] = 0x0000; // black
	((WORD *)pLUT1)[2] = 0x0000;	// black
	((WORD *)pLUT1)[3] = 0xffff;	// white

	// LUT 2
	((WORD *)pLUT2)[0] = 0xf800; // red
	((WORD *)pLUT2)[1] = 0x07e0; // green
	((WORD *)pLUT2)[2] = 0x001f; // blue
	((WORD *)pLUT2)[3] = 0xffff; // white

	// *** Make the PseudoColor texture ***
	MakePic4(pTexture, 32, 32);

	// PART A -- Loading of the LUTs
   // Each LUT is an array of 16 colors (32 bytes total for a LUT)

	SetDefaultParameters(&msiParameters);
	msiParameters.msiTexture.Width         		= 32;
	msiParameters.msiTexture.Height        		= 32;
	msiParameters.msiTexture.Planes       			= 4;
	msiParameters.msiTexture.pMem         			= pTexture;
	msiParameters.msiTexture.pHeap        			= pTexture;
	msiParameters.msiTexture.CacheOffset   		= TEXD_OFFSET;

	msiParameters.msiTexture.msiLUT.pMem		  = pLUT1;
	msiParameters.msiTexture.msiLUT.pHeap		  = pLUT1;
	msiParameters.msiTexture.msiLUT.CacheOffset = LUT4A_OFFSET;

	// ***   PART A -- Loading of the LUTs   ***
	// Here, the LUTs are loaded into the texture cache and the images are
	// drawn.

	// Load in the first LUT and at the same time, place the texture in the 
	//	Texture Cache.  This LUT will then be used for rendering textured 
	//	triangles with a msiTexture.Planes = 4.
	if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Load in the second LUT and set this as the current LUT.
	// Don't bother reloading the texture since it is already in the Texture
	//	Cache.
	msiParameters.msiTexture.pMem         			= 0;
	msiParameters.msiTexture.pHeap        			= 0;
	msiParameters.msiTexture.msiLUT.pMem			= pLUT2;
	msiParameters.msiTexture.msiLUT.pHeap			= pLUT2;
	msiParameters.msiTexture.msiLUT.CacheOffset	= LUT4B_OFFSET;
	if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// ***   PART B -- Assigning the LUT to an Index   ***
	// The LUTs are already in the Texture Cache, so only their offset in the
	// texture cache needs to be specified.  At the same time, we assign
	// an Index value to the LUT so that it can be easily referred to in a
	// future call to msiSetTextureCLUT4.

	// Use LUT at offset 0, and assign index 0 to the LUT
	if (!msiSetTextureCLUT4(LUT4A_OFFSET, 0))
   {
    DBG_PRINT("msiSetTextureCLUT4 has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,-300,160);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// use LUT at offset 32, and assign index 1 to the LUT
	if (!msiSetTextureCLUT4(LUT4B_OFFSET, 1))
   {
    DBG_PRINT("msiSetTextureCLUT4 has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// ***   PART C -- Selecting a LUT by its Index   ***
	// Previously, we associated an Index value with each LUT.  Now, to
	// select that LUT we just have to specify it's Index value.
	//
	// Passing an offset value higher than 0x80000000 tells the graphics
	// engine that it must use the cached LUT at the specified Index.

	if (!msiSetTextureCLUT4(0xFFFFFFFF, 1))
   {
    DBG_PRINT("msiSetTextureCLUT4 has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,-300,160);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiSetTextureCLUT4(0xFFFFFFFF, 0))
   {
    DBG_PRINT("msiSetTextureCLUT4 has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,300,0);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

 	if (!msiEndFrame(0, 0, TRUE))  // TRUE renders the frame immediately.
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

	if (!msiFreeTextureHeap(pTexture))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

	if (!msiFreeTextureHeap(pLUT1))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

	if (!msiFreeTextureHeap(pLUT2))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

} // ShowLUT4


#define RGB565_SCALE(base_col,scale) (	(unsigned int)(((base_col&0xF800)>>11)*scale)<<11 | \
										(unsigned int)(((base_col&0x07E0)>>5)*scale)<<5 | \
										(unsigned int)((base_col&0x001F)*scale)	)

void ShowLUT8()
{
	//
   // Demonstrates how to use:	msiSetTextureCLUT8
   // Demonstrates how to:			Use CLUT8 textures, and load LUT8 tables into
	//										the Texture Cache.
	//

	int i;
	LPBYTE pTexture; 
	LPBYTE pLUT1; // used to create LUT1
	LPBYTE pLUT2; // used to create LUT2

	if (!msiStartFrame(1,100.0f,100.0f,100.0f,0,0.0)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

	// *** Set the vertices for the triangles to be rendered ***
	msiVertex[0].x = 250.0f;msiVertex[0].y = 20.0f;	msiVertex[0].z    = 0.4f;
	msiVertex[0].u = 0.0f;	msiVertex[0].v = 0.0f;	msiVertex[0].invW = 1.0f;
	msiVertex[1].x = 250+128;msiVertex[1].y = 20.0f;msiVertex[1].z    = 0.4f;
	msiVertex[1].u = 1.0f;	msiVertex[1].v = 0.0f;	msiVertex[1].invW = 1.0f;
	msiVertex[2].x = 250+128;msiVertex[2].y = 20+128;	msiVertex[2].z    = 0.4f;
	msiVertex[2].u = 1.0f;	msiVertex[2].v = 1.0f;	msiVertex[2].invW = 1.0f;
	msiVertex[3].x = 250.0f;msiVertex[3].y = 20+128;msiVertex[3].z    = 0.4f;
	msiVertex[3].u = 0.0f;	msiVertex[3].v = 1.0f;	msiVertex[3].invW = 1.0f;

   // Allocate the texture heap (# of 4K byte pages) to hold the LUT8 bitmap.
	// Calculation: (128*128)/4096 = 4
	if (!(pTexture = msiAllocTextureHeap(4))) 
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

	if (!(pLUT1 = msiAllocTextureHeap(1))) 
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

	if (!(pLUT2 = msiAllocTextureHeap(1))) 
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

   // Place the bitmap in the Heap memory.  Remember that this is a 8 bit per
	// pixel bitmap.
   for (i=0;i<TEXG_SIZE_X*TEXG_SIZE_Y;i++)
		pTexture[i]=pBitmapG[i];

	// *** Make the LUTs ***
	// LUT 1
   ((WORD *)pLUT1)[0]  = 0x0000; // Black
   ((WORD *)pLUT1)[1]  = RGB565_SCALE(0xF800,1.0); // Red - Top row
   ((WORD *)pLUT1)[2]  = RGB565_SCALE(0xF800,0.9);
   ((WORD *)pLUT1)[3]  = RGB565_SCALE(0xF800,0.8);
   ((WORD *)pLUT1)[4]  = RGB565_SCALE(0xF800,0.7);
   ((WORD *)pLUT1)[5]  = RGB565_SCALE(0xF800,0.6);
   ((WORD *)pLUT1)[6]  = RGB565_SCALE(0x07E0,1.0); // Green - Top row
   ((WORD *)pLUT1)[7]  = RGB565_SCALE(0x07E0,0.9);
   ((WORD *)pLUT1)[8]  = RGB565_SCALE(0x07E0,0.8);
   ((WORD *)pLUT1)[9]  = RGB565_SCALE(0x07E0,0.7);
   ((WORD *)pLUT1)[10] = RGB565_SCALE(0x07E0,0.6);
   ((WORD *)pLUT1)[11] = RGB565_SCALE(0x001F,1.0); // Blue - Top row
   ((WORD *)pLUT1)[12] = RGB565_SCALE(0x001F,0.9);
   ((WORD *)pLUT1)[13] = RGB565_SCALE(0x001F,0.8);
   ((WORD *)pLUT1)[14] = RGB565_SCALE(0x001F,0.7);
   ((WORD *)pLUT1)[15] = RGB565_SCALE(0x001F,0.6);
   ((WORD *)pLUT1)[16] = RGB565_SCALE(0xFFFF,1.0); // Grey - Middle row
   ((WORD *)pLUT1)[17] = RGB565_SCALE(0xFFFF,0.9);
   ((WORD *)pLUT1)[18] = RGB565_SCALE(0xFFFF,0.8);
   ((WORD *)pLUT1)[19] = RGB565_SCALE(0xFFFF,0.7);
   ((WORD *)pLUT1)[20] = RGB565_SCALE(0xFFFF,0.6);
   ((WORD *)pLUT1)[21] = RGB565_SCALE(0xFFE0,1.0); // Yellow - Middle row
   ((WORD *)pLUT1)[22] = RGB565_SCALE(0xFFE0,0.9);
   ((WORD *)pLUT1)[23] = RGB565_SCALE(0xFFE0,0.8);
   ((WORD *)pLUT1)[24] = RGB565_SCALE(0xFFE0,0.7);
   ((WORD *)pLUT1)[25] = RGB565_SCALE(0xFFE0,0.6);
   ((WORD *)pLUT1)[26] = RGB565_SCALE(0xF81F,1.0); // Purple - Bottom row
   ((WORD *)pLUT1)[27] = RGB565_SCALE(0xF81F,0.9);
   ((WORD *)pLUT1)[28] = RGB565_SCALE(0xF81F,0.8);
   ((WORD *)pLUT1)[29] = RGB565_SCALE(0xF81F,0.7);
   ((WORD *)pLUT1)[30] = RGB565_SCALE(0xF81F,0.6);
	// We don't care about the other ...-255 entries for this example

	// LUT 2
   ((WORD *)pLUT2)[0]  = 0x0000; // Black
   ((WORD *)pLUT2)[1]  = RGB565_SCALE(0xA605,1.0); // Random - Top row
   ((WORD *)pLUT2)[2]  = RGB565_SCALE(0xA605,0.9);
   ((WORD *)pLUT2)[3]  = RGB565_SCALE(0xA605,0.8);
   ((WORD *)pLUT2)[4]  = RGB565_SCALE(0xA605,0.7);
   ((WORD *)pLUT2)[5]  = RGB565_SCALE(0xA605,0.6);
   ((WORD *)pLUT2)[6]  = RGB565_SCALE(0x9294,1.0); // Random - Top row
   ((WORD *)pLUT2)[7]  = RGB565_SCALE(0x9294,0.9);
   ((WORD *)pLUT2)[8]  = RGB565_SCALE(0x9294,0.8);
   ((WORD *)pLUT2)[9]  = RGB565_SCALE(0x9294,0.7);
   ((WORD *)pLUT2)[10] = RGB565_SCALE(0x9294,0.6);
   ((WORD *)pLUT2)[11] = RGB565_SCALE(0xD00A,1.0); // Random - Top row
   ((WORD *)pLUT2)[12] = RGB565_SCALE(0xD00A,0.9);
   ((WORD *)pLUT2)[13] = RGB565_SCALE(0xD00A,0.8);
   ((WORD *)pLUT2)[14] = RGB565_SCALE(0xD00A,0.7);
   ((WORD *)pLUT2)[15] = RGB565_SCALE(0xD00A,0.6);
   ((WORD *)pLUT2)[16] = RGB565_SCALE(0x07FF,1.0); // Cyan - Middle row
   ((WORD *)pLUT2)[17] = RGB565_SCALE(0x07FF,0.9);
   ((WORD *)pLUT2)[18] = RGB565_SCALE(0x07FF,0.8);
   ((WORD *)pLUT2)[19] = RGB565_SCALE(0x07FF,0.7);
   ((WORD *)pLUT2)[20] = RGB565_SCALE(0x07FF,0.6);
   ((WORD *)pLUT2)[21] = RGB565_SCALE(0x01AF,1.0); // Random - Middle row
   ((WORD *)pLUT2)[22] = RGB565_SCALE(0x01AF,0.9);
   ((WORD *)pLUT2)[23] = RGB565_SCALE(0x01AF,0.8);
   ((WORD *)pLUT2)[24] = RGB565_SCALE(0x01AF,0.7);
   ((WORD *)pLUT2)[25] = RGB565_SCALE(0x01AF,0.6);
   ((WORD *)pLUT2)[26] = RGB565_SCALE(0x2D4E,1.0); // Random - Bottom row
   ((WORD *)pLUT2)[27] = RGB565_SCALE(0x2D4E,0.9);
   ((WORD *)pLUT2)[28] = RGB565_SCALE(0x2D4E,0.8);
   ((WORD *)pLUT2)[29] = RGB565_SCALE(0x2D4E,0.7);
   ((WORD *)pLUT2)[30] = RGB565_SCALE(0x2D4E,0.6);
	// We t care about the other ...-255 entries for this example

	SetDefaultParameters(&msiParameters);

	msiParameters.msiTexture.Width         		= TEXG_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXG_SIZE_Y;
	msiParameters.msiTexture.Planes       			= 8;
	msiParameters.msiTexture.pMem         			= pTexture;
	msiParameters.msiTexture.pHeap        			= pTexture;
	msiParameters.msiTexture.CacheOffset   		= TEXG_OFFSET;
	msiParameters.msiTexture.Transparency 			= TRUE;
	msiParameters.msiTexture.KeyingColor			= 0x0000;
	msiParameters.msiTexture.KeyingColorMask		= 0xffff;

	msiParameters.msiTexture.msiLUT.pMem		  = pLUT1;
	msiParameters.msiTexture.msiLUT.pHeap		  = pLUT1;
	msiParameters.msiTexture.msiLUT.CacheOffset = LUT8A_OFFSET;

	// Load in the first LUT and at the same time, place the texture in the 
	//	Texture Cache.  This LUT will then be used for rendering textured 
	//	triangles with a msiTexture.Planes = 8.
	if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	msiParameters.msiTexture.pMem         			= 0;
	msiParameters.msiTexture.pHeap        			= 0;
	msiParameters.msiTexture.msiLUT.pMem			= pLUT2;
	msiParameters.msiTexture.msiLUT.pHeap			= pLUT2;
	msiParameters.msiTexture.msiLUT.CacheOffset	= LUT8B_OFFSET;
	// Load in the second LUT and set this as the current LUT.
	if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,0,160);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	// Select the first LUT again
	if (!msiSetTextureCLUT8(LUT8A_OFFSET))
   {
    DBG_PRINT("msiSetTextureCLUT8 has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,0,160);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

 	if (!msiEndFrame(0, 0, TRUE))  // TRUE renders the frame immediately.
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

	if (!msiFreeTextureHeap(pTexture))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

	if (!msiFreeTextureHeap(pLUT1))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

	if (!msiFreeTextureHeap(pLUT2))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

} // ShowLUT8

void UV_CLAMPTest()
{
	//
   // Demonstrates how to use:	msiSetTextureSize, msiSetTextureWrap
	//

	LPBYTE pTexture;

   // Allocate the texture heaps (# of 4K byte pages) to hold the bitmaps.
	// Calculation: (128*128*2bpp+64*64*2bpp)/4096 = 10
	pTexture = msiAllocTextureHeap(10);
	if (pTexture==NULL)
   {
    DBG_PRINT("msiAllocTextureHeap has failed.\n");
    exit(0);
   }

   CopyBMPLinear(pTexture,&pBitmapE[0],TEXE_SIZE_X,TEXE_SIZE_Y); 
   CopyBMPLinear(pTexture+TEXE_SIZE_X*TEXE_SIZE_Y*2,&pBitmapF[0],TEXF_SIZE_X,TEXF_SIZE_Y); 

	if (!msiStartFrame(1,100.0f,100.0f,100.0f,0,0.0)) 
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

	// Set the vertices to pass to msiRenderTriangle
	msiVertex[0].x	 = 10.0f;	msiVertex[0].y	 = 10.0f;	msiVertex[0].z		= 1.0f;
	msiVertex[0].mr = 255.0f;	msiVertex[0].mg = 255.0f;	msiVertex[0].mb	= 255.0f;
   msiVertex[0].u	 = 0.0f;		msiVertex[0].v	 = 0.0f;		msiVertex[0].invW = 1.0f;
	msiVertex[1].x	 = 10+64;	msiVertex[1].y	 = 10.0f;	msiVertex[1].z		= 1.0f;
	msiVertex[1].mr = 255.0f;	msiVertex[1].mg = 255.0f;	msiVertex[1].mb	= 255.0f;
   msiVertex[1].u	 = 1.0f;		msiVertex[1].v	 = 0.0f;		msiVertex[1].invW = 1.0f;
	msiVertex[2].x	 = 10+64;	msiVertex[2].y	 = 10+64;	msiVertex[2].z		= 1.0f;
	msiVertex[2].mr = 255.0f;	msiVertex[2].mg = 255.0f;	msiVertex[2].mb	= 255.0f;
   msiVertex[2].u	 = 1.0f;		msiVertex[2].v	 = 1.0f;		msiVertex[2].invW = 1.0f;
	msiVertex[3].x	 = 10.0f;	msiVertex[3].y	 = 10+64;	msiVertex[3].z		= 1.0f;
	msiVertex[3].mr = 255.0f;	msiVertex[3].mg = 255.0f;	msiVertex[3].mb	= 255.0f;
   msiVertex[3].u	 = 0.0f;		msiVertex[3].v	 = 1.0f;		msiVertex[3].invW = 1.0f;

	msiVertex[4].x	 = 200.0f;	msiVertex[4].y	 = 30.0f;	msiVertex[4].z		= 1.0f;
	msiVertex[4].mr = 255.0f;	msiVertex[4].mg = 255.0f;	msiVertex[4].mb	= 255.0f;
   msiVertex[4].u	 = 0.0f;		msiVertex[4].v	 = 0.0f;		msiVertex[4].invW = 1.0f;
	msiVertex[5].x	 = 200+128;	msiVertex[5].y	 = 30.0f;	msiVertex[5].z		= 1.0f;
	msiVertex[5].mr = 255.0f;	msiVertex[5].mg = 255.0f;	msiVertex[5].mb	= 255.0f;
   msiVertex[5].u	 = 1.0f;		msiVertex[5].v	 = 0.0f;		msiVertex[5].invW = 1.0f;
	msiVertex[6].x	 = 200+128;	msiVertex[6].y	 = 30+128;	msiVertex[6].z		= 1.0f;
	msiVertex[6].mr = 255.0f;	msiVertex[6].mg = 255.0f;	msiVertex[6].mb	= 255.0f;
   msiVertex[6].u	 = 1.0f;		msiVertex[6].v	 = 1.0f;		msiVertex[6].invW = 1.0f;
	msiVertex[7].x	 = 200.0f;	msiVertex[7].y	 = 30+128;	msiVertex[7].z		= 1.0f;
	msiVertex[7].mr = 255.0f;	msiVertex[7].mg = 255.0f;	msiVertex[7].mb	= 255.0f;
   msiVertex[7].u	 = 0.0f;		msiVertex[7].v	 = 1.0f;		msiVertex[7].invW = 1.0f;


	SetDefaultParameters(&msiParameters);

	msiParameters.msiTexture.Width         		= TEXE_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXE_SIZE_Y;
	msiParameters.msiTexture.pMem         			= pTexture;
	msiParameters.msiTexture.pHeap        			= pTexture;
	msiParameters.msiTexture.CacheOffset   		= TEXE_OFFSET;
	if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	msiParameters.msiTexture.Width         		= TEXF_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXF_SIZE_Y;
	msiParameters.msiTexture.pMem         			= pTexture+TEXE_SIZE_X*TEXE_SIZE_Y*2;
	msiParameters.msiTexture.CacheOffset   		= TEXF_OFFSET;
	if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

   msiVertex[1].u	 = 1.0f;		msiVertex[1].v	 = 0.0f;		msiVertex[1].invW = 1.0f;
   msiVertex[2].u	 = 1.0f;		msiVertex[2].v	 = 2.0f;		msiVertex[2].invW = 1.0f;
   msiVertex[3].u	 = 0.0f;		msiVertex[3].v	 = 2.0f;		msiVertex[3].invW = 1.0f;

	AddxAddy(msiVertex,0,3,0,75);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

   msiVertex[1].u	 = 2.0f;		msiVertex[1].v	 = 0.0f;		msiVertex[1].invW = 1.0f;
   msiVertex[2].u	 = 2.0f;		msiVertex[2].v	 = 2.0f;		msiVertex[2].invW = 1.0f;
   msiVertex[3].u	 = 0.0f;		msiVertex[3].v	 = 2.0f;		msiVertex[3].invW = 1.0f;

	AddxAddy(msiVertex,0,3,0,75);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiSetTextureWrap(0,1))
   {
    DBG_PRINT("msiSetTextureWrap has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,0,75);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiSetTextureWrap(1,0))
   {
    DBG_PRINT("msiSetTextureWrap has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,0,75);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

	if (!msiSetTextureWrap(1,1))
   {
    DBG_PRINT("msiSetTextureWrap has failed.\n");
    exit(0);
   }

	AddxAddy(msiVertex,0,3,0,75);
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[1], &msiVertex[2], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[0], &msiVertex[2], &msiVertex[3], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

 	// Now work with another image size
	if (!msiSetTextureSize(TEXE_SIZE_X,TEXE_SIZE_Y))
   {
    DBG_PRINT("msiSetTextureSize has failed.\n");
    exit(0);
   }

   if (!msiSetTextureOffset(TEXE_OFFSET))
   {
    DBG_PRINT("msiSetTextureOffset has failed.\n");
    exit(0);
   }

	if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

 	msiParameters.msiTexture.Transparency  		= TRUE;
	msiParameters.msiTexture.Width         		= TEXE_SIZE_X;
	msiParameters.msiTexture.Height        		= TEXE_SIZE_Y;
	msiParameters.msiTexture.pMem         			= 0;
	msiParameters.msiTexture.pHeap        			= 0;
	msiParameters.msiTexture.CacheOffset   		= TEXE_OFFSET;
   if (!msiSetParameters(&msiParameters)) 
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   }

   msiVertex[5].u	 = 1.0f;		msiVertex[5].v	 = 0.0f;		msiVertex[5].invW = 1.0f;
   msiVertex[6].u	 = 1.0f;		msiVertex[6].v	 = 2.0f;		msiVertex[6].invW = 1.0f;
   msiVertex[7].u	 = 0.0f;		msiVertex[7].v	 = 2.0f;		msiVertex[7].invW = 1.0f;

	AddxAddy(msiVertex,4,7,0,150);
	if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

   msiVertex[5].u	 = 0.5f;		msiVertex[5].v	 = 0.0f;		msiVertex[5].invW = 1.0f;
   msiVertex[6].u	 = 0.5f;		msiVertex[6].v	 = 1.0f;		msiVertex[6].invW = 1.0f;
   msiVertex[7].u	 = 0.0f;		msiVertex[7].v	 = 1.0f;		msiVertex[7].invW = 1.0f;

 	AddxAddy(msiVertex,4,7,0,150);
	if (!msiRenderTriangle(&msiVertex[4], &msiVertex[5], &msiVertex[6], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }
	if (!msiRenderTriangle(&msiVertex[4], &msiVertex[6], &msiVertex[7], 100)) 
   {
    DBG_PRINT("msiRenderTriangle has failed.\n");
    exit(0);
   }

 	if (!msiEndFrame(0, 0, TRUE))  // TRUE renders the frame immediately.
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    exit(0);
   }

	if (!msiFreeTextureHeap(pTexture))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	WinMain																		 //
//								 																	 //
// DESCRIPTION:	Win95 entry point											 			 //
//									 										 						 //
//////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain   (  HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

   MSG msg;
   FILE *fp;

   // Load the Bitmaps that will be used in the examples below
   if (!BMPLoad("mys_1.bmp" ,pBitmapA,TEXA_SIZE_X,0,TEXA_SIZE_Y,0,0))
     exit(0);
   if (!BMPLoad("mys_3.bmp" ,pBitmapB,TEXB_SIZE_X,0,TEXB_SIZE_Y,0,0))
     exit(0);
   if (!BMPLoad("matrox.bmp",pBitmapC,TEXC_SIZE_X,0,TEXC_SIZE_Y,0,0))
     exit(0);
   if (!BMPLoad("mys_3d.bmp",pBitmapD,TEXD_SIZE_X,0,TEXD_SIZE_Y,0,0))
     exit(0);
   if (!BMPLoad("mys_2.bmp" ,pBitmapE,TEXE_SIZE_X,0,TEXE_SIZE_Y,0,0))
     exit(0);
   if (!BMPLoad("bkbl16.bmp",pBitmapF,TEXF_SIZE_X,0,TEXF_SIZE_Y,0,0))
     exit(0);
   if (!BMPLoad("logo.bmp",pBitmapH,TEXH_SIZE_X,0,TEXH_SIZE_Y,0,0))
     exit(0);
   
   fp=fopen("lut8.raw","rb");
   if (!fp)
   {
    DBG_PRINT("Can't read lut8.raw\n");
    exit(0);
   }
   fread(pBitmapG,TEXG_SIZE_X*TEXG_SIZE_Y,1,fp);
   fclose(fp);

  pInfo = msiInit(Xres, Yres, 16, UseHardwareZ, msiDBG_DumpToFile, WindowProc);
  if (pInfo == NULL)
   {
    MessageBox(NULL, "MsiInit failed, exit ...", NULL, MB_OK);
    return(0);
   }
 
	ShowCursor(FALSE);

  	// Display the Matrox Splash Screen at the beginning.
	// This could only be done at a specific point and it will overwrite the 
	// contents of the Texture Cache.
   if (!msiSplashScreen()) 
   {
    DBG_PRINT("msiSplashScreen has failed.\n");
    return(0);
   }

   while (GetMessage(&msg, NULL, 0, 0))
      {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      }

   // Close the MSI library and return to Windows
   msiExit();

   ShowCursor(TRUE);

   return(msg.wParam);
} // WinMain

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	WindowProc														 		  	 //
//								 											 						 //
// DESCRIPTION:	Window procedure that handles the windows message loop	 //
//									 										 						 //
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProc(  HWND hWnd, UINT uMsg, WPARAM wParam,	LPARAM lParam)	
{
   switch(uMsg)
   {
     case WM_SIZE:
       switch (wParam)
       {
         case SIZE_MAXIMIZED:   break;            
         case SIZE_MINIMIZED:   break;
         default:               break;
       }
       break;

     case WM_KEYDOWN:
       switch (wParam)
       {
      case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0); 
			break;
		case '1':
			CacheTexturesFromHeap();
			break;
		case '2':
			CacheTexturesFromMem();
			break;
		case '3':
			CacheTexturesDirectly();
			break;
		case '4':
			ShowLUT4();
			break;
		case '8':
			ShowLUT8();
			break;
 		case 'l':
		case 'L':
			DrawLines();
			break;
	  	case 't':
		case 'T':
			TexturingTypes();
			break;
		case 'u':
		case 'U':
			UV_CLAMPTest();
			break;
		default:
			break;
       } // Switch (wParam)
       break;

    case WM_CLOSE:
     case WM_DESTROY:
       PostQuitMessage(0);
	    break;

   } // Switch (uMsg)

	// Default processing for any window messages that an application does not
	// process. 
	return DefWindowProc(hWnd, uMsg, wParam, lParam);

} // WindowProc
