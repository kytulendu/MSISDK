//////////////////////////////////////////////////////////////////////////////
// msicube.c																					 //
//																									 //
// This sample code is designed to demonstrate the use of the MSIDOS API.	 //
//	It is mainly intended for showing the frame rate comparison of the cube	 //
//	when setting BUS-MASTERING On versus Off.											 //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <i86.h>

#include "..\msidos.h"
#include "bitmap.h" /* Needed to read the .bmp files under DOS */
	  
//
// Some defines
//
#define LPCSTR const char far*
#define MSIDBG_MSG_PREFIX       "[msicube] "
#define MSIDBG_LOG_PATH         "msicube.log"

#define KEY_TAB 		9
#define KEY_SPACE		32
#define KEY_ENTER		13
#define VK_UP			72
#define VK_DOWN 		80
#define VK_DELETE 	83
#define VK_INSERT		82
#define VK_END 		79
#define VK_HOME 		71
#define KEY_ESCAPE	27
#define VK_F1			59
#define VK_F2			60
#define VK_F3			61
#define VK_F4			62
#define VK_F5			63
#define VK_F6			64
#define VK_F7			65
#define VK_F8			66
#define VK_F9			67
	
#define TRUE 		1
#define FALSE		0

//
// Global Variables 
//
static T_msiInfo*  pInfo;   // MSI Direct Frame Buffer Access information structure.

//
// Variables relating to the Cube vertices.
//
T_msiParameters    msiParameters;
T_msiVertex        Vertices[12];			// Vertices for msiRenderTriangle (Frames)
float              CubeVerticesX[24],	// Vertices for the Cube.
                   CubeVerticesY[24],     
                   CubeVerticesZ[24],     
                   CubeVerticesOX[24],	// These are the Original X,Y,Z Values.
                   CubeVerticesOY[24],    
                   CubeVerticesOZ[24];
T_msiVertex        CubeVertices[24];	// Cube Verticies (SCR Coordinates to Msi) 

LPBYTE pTextures;	// Pointer to a Texture Heap to hold the cube textures
LPBYTE pNumbers;		// Pointer to a Texture Heap to hold the number texture
LPBYTE pLUT;			// Pointer to a Texture Heap to hold the LUT

#define FACES_SIZE_X 256
#define FACES_SIZE_Y 256
#define NUMBERS_SIZE_X 1024
#define NUMBERS_SIZE_Y 32
#define LUT8_OFFSET 0
#define NUMBERS_OFFSET LUT8_OFFSET+512
#define FACES_OFFSET NUMBERS_OFFSET+NUMBERS_SIZE_X*NUMBERS_SIZE_Y

// Will hold the read in .bmp files
BYTE pBitmapFace[2*FACES_SIZE_X*FACES_SIZE_Y];
BYTE pBitmapNum[NUMBERS_SIZE_X*NUMBERS_SIZE_Y];

//
// Frame Counting Variables
//
int                TotFrames = 0;       // The Total Number of Frames so far.
int                AvgFrames = 0;       // The Average No. of Frames per sec.

//
// Cube Control Variables
//
int                XRotation = 0, // Current State of Rotation in X-Axis.
                   YRotation = 0, // Current state of Rotation in Y-Axis.
                   ZRotation = 0; // Current State of Rotation in Z-Axis.
//
// Math Variables
//
int                AngleX = 0, AngleY = 0, AngleZ = 0;
float              Dist = 2500.0f;	// The Viewing Distance - reduce to obtain prespective.
float              CosTable[360], SinTable[360];
float              OX, OY;				// Old X,Y Values

//
// Time Variables
//
clock_t		temps;
float       ftemps, Result, TotResult = 0.0f;                

//
// Demo Specific Variables  
//
int Again;	// The Calculations involved here take very neglegible time
				// as compared to the drawing time. Thus to show the full
				// advantage of BUS-MASTERING, we keep the CPU busy by doing the
				// calculations over and over for so many times.

float              CubeSize = 100.0f; 
int                Repetitions=50; // The Number of times to repeat the calculations.
int                Spins, Loop;

BOOL               WAIT = FALSE;	// No WAIT, i.e. Bus Mastering On
BOOL               GOURAUD = FALSE;   

//
// Functions
//
void GenerateLookUpTables();
BOOL InitNumbers();
void CopyBMPLinear(LPBYTE pTexture, BYTE *pBitmap, int Width, int Height);
void SetDefaultParameters(T_msiParameters *msiParameters);
BOOL InitTextures();
void InitCubeVertices(void);
BOOL AddxAddy(T_msiVertex *pVertex,int from,int to,float xadd,float yadd);
float VertexProductZ(  T_msiVertex *Vertex1,T_msiVertex *Vertex2,T_msiVertex *Vertex3);
void DisplayNumber(float X, float Y, int Number);
void DisplayCharXYZ(float X, float Y, int Value, int State);
void DisplayBus(float X, float Y, BOOL State);
BOOL BMPLoad ( LPCSTR FileName,	// 32 bit ptr. to constant char str.
               BYTE *pBitMap,		// Pointer to where in mem. to load the BMP.
               LONG XSize,			// The Width of the BMP to be Loaded.
               LONG XOffset,		// The Upper Left Corner of BMP.
               LONG YSize,			// The Hight of the BMP to be Loaded.
               LONG YOffset,		// The Upper Left Corner of BMP.
               LONG Pitch			// The Pitch.
             );
static void DBG_PRINT   (  char *pMessage, ...  );

main() 
{
   int c;

   // Initialize the MSI Library to 640x480 with 16 bits per pixel.
   // Also enable Dumping of Debugging Information (Errors Only).
   pInfo = msiInit(640, 480, 16, FALSE, msiDBG_DumpToFile);
   if (pInfo == NULL)
   {
    printf("msiInit has failed.\n");
    exit(0);
   }

	// Display the Matrox Splash Screen at the beginning.
	// This could only be done at a specific point and it will overwrite the 
	// contents of the Texture Cache.
   if (!msiSplashScreen()) 
   {
    DBG_PRINT("msiSplashScreen has failed.\n");
    exit(0);
   }

   GenerateLookUpTables();
   
	// We want enough to hold the number texture
	// Calculation: 1024*32/4096 = 8
   pNumbers = msiAllocTextureHeap(8); 
   if (!pNumbers)
	{
	 DBG_PRINT("msiAllocTextureHeap has failed.\n");
	 exit(0);
	}

   pLUT = msiAllocTextureHeap(1); 
   if (!pLUT)
	{
	 DBG_PRINT("msiAllocTextureHeap has failed.\n");
	 exit(0);
	}

   if (!InitNumbers())
	  exit(0);

	// We want enough to hold the 6 textures for the cube faces
	// Calculation: 6 * 256*256*2bpp/4096 = 192
   pTextures = msiAllocTextureHeap(192); 
   if (!pTextures)
	{
	 DBG_PRINT("msiAllocTextureHeap has failed.\n");
	 exit(0);
	}

   if (!InitTextures())
	  exit(0);
   
   InitCubeVertices();
   
   // This will use the system timer for displaying the FPS.
   ReStartBench();

   while (1)
   {
    if (kbhit())
	 {   
     c = getch();
     if (c == 0) c = getch();
     switch (c)
     {
      // Motion Control keys 
      case VK_F1: XRotation = -1; ReStartBench(); break;
      case VK_F2: XRotation =  0; ReStartBench(); break;
      case VK_F3: XRotation =  1; ReStartBench(); break;
      case VK_F4: YRotation = -1; ReStartBench(); break;
      case VK_F5: YRotation =  0; ReStartBench(); break;
      case VK_F6: YRotation =  1; ReStartBench(); break;
      case VK_F7: ZRotation = -1; ReStartBench(); break;
      case VK_F8: ZRotation =  0; ReStartBench(); break;
      case VK_F9: ZRotation =  1; ReStartBench(); break;

      // Run the Demo.
      case VK_HOME:
			ReStartBench();
     		Repetitions = 125;
         for (Spins=0; Spins<2; Spins ++)
            for (Loop=0; Loop < 360; Loop++)
               DrawCube(Loop, Loop, Loop);
         AngleX = AngleY = AngleZ = 0;
     		Repetitions = 50;
		   ReStartBench();
         break;
      
      case VK_END:
	      ReStartBench();
			Repetitions = 1;
         for (Spins=0; Spins<2; Spins ++)
            for (Loop=0; Loop < 360; Loop++)
               DrawCube(Loop, Loop, Loop);
         AngleX = AngleY = AngleZ = 0;
         Repetitions = 50;
	   	ReStartBench();
   	   break;
      
      case VK_INSERT: 
		   ReStartBench();
      	GOURAUD = TRUE;
         break;

      case VK_DELETE:
		   ReStartBench();
         GOURAUD = FALSE;
         break;
           
      case KEY_SPACE:
	      // Pause a Frame.
		   ReStartBench();
		   // Restart rotation.
         XRotation = YRotation = ZRotation = 0;
         break;
 
      case KEY_ENTER:
		   ReStartBench();
         if (WAIT == TRUE)
           WAIT = FALSE;
		 	else
			  WAIT = TRUE;
         break;

      case VK_DOWN: 
			// Shrink the Cube by 25 units squared.
			ReStartBench();
      	CubeSize -= 25.0f;
         if (CubeSize<50.0f)
			  CubeSize=50.0f;
         InitCubeVertices();
         break;

      case VK_UP:
      	// Enlarge the Cube.
		   ReStartBench();
      	CubeSize += 25.0f;
         if (CubeSize>150.0f)
			  CubeSize=150.0f; 
         InitCubeVertices();
         break;

      case KEY_TAB:
		   ReStartBench();
      	AngleX=AngleY=AngleZ=0;
         break;

      case KEY_ESCAPE:
         break;
		default:
		   break;
     } // Switch (c)

     if (c == KEY_ESCAPE)
       break;
    } // if kbhit()

    AngleX += XRotation;
    if (AngleX>359)
	   AngleX = 0;
	 else if (AngleX<0)
		AngleX = 359;

    AngleY += YRotation;
    if (AngleY>359)
	   AngleY = 0;
	 else if (AngleY<0)
	 AngleY = 359;

    AngleZ += ZRotation;
    if (AngleZ>359)
	 	AngleZ = 0;
	 else if (AngleZ<0)
	 AngleZ = 359;

	 DrawCube(AngleX, AngleY, AngleZ);
   } // end while(1)


	// Free the previously allocated Heaps
	if (!msiFreeTextureHeap(pTextures))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }
	if (!msiFreeTextureHeap(pNumbers))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }
	if (!msiFreeTextureHeap(pLUT))
   {
    DBG_PRINT("msiFreeTextureHeap has failed.\n");
    exit(0);
   }

   // Close the MSI Library
   msiExit();

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	GenerateLookUpTables														 //
//																									 //
// DESCRIPTION:	Generates Sin and Cos Look up Tables for faster math.		 //
//////////////////////////////////////////////////////////////////////////////
void GenerateLookUpTables(void)
{
   int                Angle;
   float              Rad;
   for (Angle = 0; Angle <360; Angle ++)
   {
    // The Tables must be declared as Global as they are used by many
	 // procedures.
    // Convert to rad
    Rad = (float)(3.141592654*(float)Angle/(float)180);
    CosTable[Angle] = (float) cos(Rad);
    SinTable[Angle] = (float) sin(Rad);
   } // angle
} // GenerateLookUpTables

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	ReStartBench														 		 //
//																									 //
// DESCRIPTION:	Used to restart the benchmarking and reinit its variables //
//////////////////////////////////////////////////////////////////////////////
void ReStartBench(void)
{
   // BenchMark the Demo
   TotFrames = 0;
   Result = TotResult = 0.0f;
} // ReStartBench

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
			((WORD far*)pTexture)[xx+(yy*Width)] = ((WORD*)pBitmap)[xx+(yy*Width)];

} // CopyBMPLinear

void PutNumbersInCache()
{
    // Start a new frame and clear the RGB
    if (!msiStartFrame(TRUE, 0.0f, 0.0f, 0.0f, FALSE, 1.0f))
    {
     DBG_PRINT("msiStartFrame has failed.\n");
     exit(0);
    }
   
    // Load the Number Bitmaps and LUT into the Texture Cache
    SetDefaultParameters(&msiParameters);

    msiParameters.msiTexture.pMem					= pNumbers; 
    msiParameters.msiTexture.pHeap					= pNumbers;    
    msiParameters.msiTexture.CacheOffset			= NUMBERS_OFFSET;          
    msiParameters.msiTexture.Width					= NUMBERS_SIZE_X; 
    msiParameters.msiTexture.Height					= NUMBERS_SIZE_Y;
    msiParameters.msiTexture.Planes					= 8;
    msiParameters.msiTexture.msiLUT.pMem			= pLUT;
    msiParameters.msiTexture.msiLUT.pHeap			= pLUT;
    msiParameters.msiTexture.msiLUT.CacheOffset	= LUT8_OFFSET;

    if (!msiSetParameters(&msiParameters))
    {
     DBG_PRINT("msiSetParameters has failed.\n");
     exit(0);
    } 

    if (msiEndFrame(FALSE, 0, WAIT) == FALSE) 
    {
     DBG_PRINT("msiEndFrame has failed.\n");
     exit(0);
    }

} // PutNumbersInCache

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	InitNumbers																	 //
//																									 //
// DESCRIPTION:	Loads the numbers texture into the Texture Cache			 //
//						Used in:																	 //
//						DisplayNumber();                           					 //
//						DisplayCharXYZ();                          					 //
//						DisplayBus();															 //
//////////////////////////////////////////////////////////////////////////////
BOOL InitNumbers()
{
   FILE *fp;
	int i;

	fp = fopen("numbers.raw","rb");
	if (!fp)
    {
     DBG_PRINT("Can't read numbers.raw\n");
     return(0);
    }
   fread(pBitmapNum,NUMBERS_SIZE_X*NUMBERS_SIZE_Y,1,fp);
   fclose(fp);

   // Place texture into the Heap - later to be loaded into the Texture Cache
   for (i=0;i<NUMBERS_SIZE_X*NUMBERS_SIZE_Y;i++)
		pNumbers[i]=pBitmapNum[i];

	// *** Make the LUT ***
	// We only care about the first two indexes
	((WORD far *)pLUT)[0] = 0x0000; // black
   ((WORD far *)pLUT)[1] = 0xF800; // red

   // Define the vertices for displaying the characters
   Vertices[0].x   =   0.0f;   Vertices[0].y   =   0.000f; Vertices[0].z    =   0.0f;
   Vertices[0].u   =   0.0f;   Vertices[0].v   =   0.0f;   Vertices[0].invW =   1.0f;
   Vertices[1].x   =   0.000f; Vertices[1].y   =  32.000f; Vertices[1].z    =   0.0f;
   Vertices[1].u   =   0.0f;   Vertices[1].v   =   1.0f;   Vertices[1].invW =   1.0f;
   Vertices[2].x   =  32.000f; Vertices[2].y   =   0.000f; Vertices[2].z    =   0.0f;
   Vertices[2].u   =   1.0f;   Vertices[2].v   =   0.0f;   Vertices[2].invW =   1.0f;
   Vertices[3].x   =  32.0f;   Vertices[3].y   =  32.000f; Vertices[3].z    =   0.0f;
   Vertices[3].u   =   1.0f;   Vertices[3].v   =   1.0f;   Vertices[3].invW =   1.0f;
   Vertices[4].x   =  32.000f; Vertices[4].y   =   0.000f; Vertices[4].z    =   0.0f;
   Vertices[4].u   =   1.0f;   Vertices[4].v   =   0.0f;   Vertices[4].invW =   1.0f;
   Vertices[5].x   =  32.000f; Vertices[5].y   =  32.000f; Vertices[5].z    =   0.0f;
   Vertices[5].u   =   1.0f;   Vertices[5].v   =   1.0f;   Vertices[5].invW =   1.0f;
   Vertices[6].x   =  64.0f;   Vertices[6].y   =   0.000f; Vertices[6].z    =   0.0f;
   Vertices[6].u   =   2.0f;   Vertices[6].v   =   0.0f;   Vertices[6].invW =   1.0f;
   Vertices[7].x   =  64.000f; Vertices[7].y   =  32.000f; Vertices[7].z    =   0.0f;
   Vertices[7].u   =   2.0f;   Vertices[7].v   =   1.0f;   Vertices[7].invW =   1.0f;
   Vertices[8].x   =  64.0f;   Vertices[8].y   =   0.000f; Vertices[8].z    =   0.0f;
   Vertices[8].u   =   0.0f;   Vertices[8].v   =   0.0f;   Vertices[8].invW =   1.0f;
   Vertices[9].x   =  64.000f; Vertices[9].y   =  32.000f; Vertices[9].z    =   0.0f;
   Vertices[9].u   =   0.0f;   Vertices[9].v   =   1.0f;   Vertices[9].invW =   1.0f;
   Vertices[10].x  =  96.000f; Vertices[10].y  =   0.000f; Vertices[10].z    =   0.0f;
   Vertices[10].u  =   1.0f;   Vertices[10].v  =   0.0f;   Vertices[10].invW =   1.0f;
   Vertices[11].x  =  96.0f;   Vertices[11].y  =  32.000f; Vertices[11].z    =   0.0f;
   Vertices[11].u  =   1.0f;   Vertices[11].v  =   1.0f;   Vertices[11].invW =   1.0f;
  
   PutNumbersInCache();
 
   return(1);

} // InitNumbers

void PutTexturesInCache()
{
   if (!msiStartFrame(TRUE, 0.0f, 0.0f, 0.0f, FALSE, 1.0f))
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

    // Load the cube Bitmaps into the Texture Cache
    SetDefaultParameters(&msiParameters);

    msiParameters.msiTexture.pMem          = pTextures;
    msiParameters.msiTexture.pHeap         = pTextures;
    msiParameters.msiTexture.CacheOffset   = FACES_OFFSET;
    msiParameters.msiTexture.Width         = FACES_SIZE_X;
    msiParameters.msiTexture.Height        = FACES_SIZE_Y;
   
    if (!msiSetParameters(&msiParameters))
    {
     DBG_PRINT("msiSetParameters has failed.\n");
     exit(0);
    } 

    msiParameters.msiTexture.pMem          = pTextures+1*2*FACES_SIZE_X*FACES_SIZE_Y;
    msiParameters.msiTexture.CacheOffset   = FACES_OFFSET+1*2*FACES_SIZE_X*FACES_SIZE_Y;
    if (!msiSetParameters(&msiParameters))
    {
     DBG_PRINT("msiSetParameters has failed.\n");
     exit(0);
    } 
    msiParameters.msiTexture.pMem          = pTextures+2*2*FACES_SIZE_X*FACES_SIZE_Y;
    msiParameters.msiTexture.CacheOffset   = FACES_OFFSET+2*2*FACES_SIZE_X*FACES_SIZE_Y;
    if (!msiSetParameters(&msiParameters))
    {
     DBG_PRINT("msiSetParameters has failed.\n");
     exit(0);
    } 
    msiParameters.msiTexture.pMem          = pTextures+3*2*FACES_SIZE_X*FACES_SIZE_Y;
    msiParameters.msiTexture.CacheOffset   = FACES_OFFSET+3*2*FACES_SIZE_X*FACES_SIZE_Y;
    if (!msiSetParameters(&msiParameters))
    {
     DBG_PRINT("msiSetParameters has failed.\n");
     exit(0);
    } 
    msiParameters.msiTexture.pMem          = pTextures+4*2*FACES_SIZE_X*FACES_SIZE_Y;
    msiParameters.msiTexture.CacheOffset   = FACES_OFFSET+4*2*FACES_SIZE_X*FACES_SIZE_Y;
    if (!msiSetParameters(&msiParameters))
    {
     DBG_PRINT("msiSetParameters has failed.\n");
     exit(0);
    } 
    msiParameters.msiTexture.pMem          = pTextures+5*2*FACES_SIZE_X*FACES_SIZE_Y;
    msiParameters.msiTexture.CacheOffset   = FACES_OFFSET+5*2*FACES_SIZE_X*FACES_SIZE_Y;
    if (!msiSetParameters(&msiParameters))
    {
     DBG_PRINT("msiSetParameters has failed.\n");
     exit(0);
    } 

    if (msiEndFrame(FALSE, 0, WAIT) == FALSE) 
    {
     DBG_PRINT("msiEndFrame has failed.\n");
     exit(0);
    }

} // PutTexturesInCache

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	InitTextures																 //
//																									 //
// DESCRIPTION:	Loads the cube textures into the Texture Cache			 	 //
//////////////////////////////////////////////////////////////////////////////
BOOL InitTextures()
{
	if (!BMPLoad("mys_1.bmp",pBitmapFace,FACES_SIZE_X,0,FACES_SIZE_Y,0,0))
	  return(0);
	CopyBMPLinear(pTextures,&pBitmapFace[0],FACES_SIZE_X,FACES_SIZE_Y); 
	if (!BMPLoad("mys_2.bmp",pBitmapFace,FACES_SIZE_X,0,FACES_SIZE_Y,0,0))
	  return(0);
	CopyBMPLinear(pTextures+1*2*FACES_SIZE_X*FACES_SIZE_Y,&pBitmapFace[0],FACES_SIZE_X,FACES_SIZE_Y); 
	if (!BMPLoad("mys_3.bmp",pBitmapFace,FACES_SIZE_X,0,FACES_SIZE_Y,0,0))
	  return(0);
	CopyBMPLinear(pTextures+2*2*FACES_SIZE_X*FACES_SIZE_Y,&pBitmapFace[0],FACES_SIZE_X,FACES_SIZE_Y); 
	if (!BMPLoad("mys_5.bmp",pBitmapFace,FACES_SIZE_X,0,FACES_SIZE_Y,0,0))
	  return(0);
	CopyBMPLinear(pTextures+3*2*FACES_SIZE_X*FACES_SIZE_Y,&pBitmapFace[0],FACES_SIZE_X,FACES_SIZE_Y); 
	if (!BMPLoad("mys_6.bmp",pBitmapFace,FACES_SIZE_X,0,FACES_SIZE_Y,0,0))
	  return(0);
	CopyBMPLinear(pTextures+4*2*FACES_SIZE_X*FACES_SIZE_Y,&pBitmapFace[0],FACES_SIZE_X,FACES_SIZE_Y); 
	if (!BMPLoad("logo.bmp",pBitmapFace,FACES_SIZE_X,0,FACES_SIZE_Y,0,0))
	  return(0);
	CopyBMPLinear(pTextures+5*2*FACES_SIZE_X*FACES_SIZE_Y,&pBitmapFace[0],FACES_SIZE_X,FACES_SIZE_Y); 

   PutTexturesInCache();

   return(1);

} // InitTextures

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	InitCubeVertices															 //
//																									 //
// DESCRIPTION:	Initializes the Cube according to CubeSize					 //
//////////////////////////////////////////////////////////////////////////////
void InitCubeVertices(void)
{
   int Counti;
   // Define the vertices for displaying the Cube    
   
   CubeVerticesX[0]=  -CubeSize ;CubeVerticesY[0]=   CubeSize ;CubeVerticesZ[0]=   CubeSize;
   CubeVerticesX[1]=   CubeSize ;CubeVerticesY[1]=   CubeSize ;CubeVerticesZ[1]=   CubeSize;
   CubeVerticesX[2]=  -CubeSize ;CubeVerticesY[2]=   CubeSize ;CubeVerticesZ[2]=  -CubeSize;
   CubeVerticesX[3]=   CubeSize ;CubeVerticesY[3]=   CubeSize ;CubeVerticesZ[3]=  -CubeSize;
   
   CubeVerticesX[4]=   CubeSize ;CubeVerticesY[4]=   CubeSize ;CubeVerticesZ[4]=   CubeSize;
   CubeVerticesX[5]=   CubeSize ;CubeVerticesY[5]=  -CubeSize ;CubeVerticesZ[5]=   CubeSize;
   CubeVerticesX[6]=   CubeSize ;CubeVerticesY[6]=   CubeSize ;CubeVerticesZ[6]=  -CubeSize;
   CubeVerticesX[7]=   CubeSize ;CubeVerticesY[7]=  -CubeSize ;CubeVerticesZ[7]=  -CubeSize;
   
   CubeVerticesX[8]=   CubeSize ;CubeVerticesY[8]=  -CubeSize ;CubeVerticesZ[8]=   CubeSize;
   CubeVerticesX[9]=  -CubeSize ;CubeVerticesY[9]=  -CubeSize ;CubeVerticesZ[9]=   CubeSize;
   CubeVerticesX[10]=  CubeSize ;CubeVerticesY[10]= -CubeSize ;CubeVerticesZ[10]= -CubeSize;
   CubeVerticesX[11]= -CubeSize ;CubeVerticesY[11]= -CubeSize ;CubeVerticesZ[11]= -CubeSize;
   
   CubeVerticesX[12]= -CubeSize ;CubeVerticesY[12]= -CubeSize ;CubeVerticesZ[12]=  CubeSize;
   CubeVerticesX[13]= -CubeSize ;CubeVerticesY[13]=  CubeSize ;CubeVerticesZ[13]=  CubeSize;
   CubeVerticesX[14]= -CubeSize ;CubeVerticesY[14]= -CubeSize ;CubeVerticesZ[14]= -CubeSize;
   CubeVerticesX[15]= -CubeSize ;CubeVerticesY[15]=  CubeSize ;CubeVerticesZ[15]= -CubeSize;
   
   CubeVerticesX[16]= -CubeSize ;CubeVerticesY[16]= -CubeSize ;CubeVerticesZ[16]=  CubeSize;
   CubeVerticesX[17]=  CubeSize ;CubeVerticesY[17]= -CubeSize ;CubeVerticesZ[17]=  CubeSize;
   CubeVerticesX[18]= -CubeSize ;CubeVerticesY[18]=  CubeSize ;CubeVerticesZ[18]=  CubeSize;
   CubeVerticesX[19]=  CubeSize ;CubeVerticesY[19]=  CubeSize ;CubeVerticesZ[19]=  CubeSize;
   
   CubeVerticesX[20]= -CubeSize ;CubeVerticesY[20]=  CubeSize ;CubeVerticesZ[20]= -CubeSize;
   CubeVerticesX[21]=  CubeSize ;CubeVerticesY[21]=  CubeSize ;CubeVerticesZ[21]= -CubeSize;
   CubeVerticesX[22]= -CubeSize ;CubeVerticesY[22]= -CubeSize ;CubeVerticesZ[22]= -CubeSize;
   CubeVerticesX[23]=  CubeSize ;CubeVerticesY[23]= -CubeSize ;CubeVerticesZ[23]= -CubeSize;

   // Define other characteristics of the vertices
   Counti=0;
   while (Counti<24)
   {
    CubeVertices[Counti].r = 255.0f;
	 CubeVertices[Counti].g = 0.0f;
	 CubeVertices[Counti].b = 0.0f;
    CubeVertices[Counti+1].r = 0.0f;
	 CubeVertices[Counti+1].g = 0.0f;
	 CubeVertices[Counti+1].b = 255.0f;
    CubeVertices[Counti+2].r = 0.0f;
	 CubeVertices[Counti+2].g = 255.0f;
	 CubeVertices[Counti+2].b = 0.0f;
    Counti+=3;
   } 
 
   CubeVertices[0].u  = 0.0f ; CubeVertices[0].v  = 1.0f ;
   CubeVertices[1].u  = 1.0f ; CubeVertices[1].v  = 1.0f ;
   CubeVertices[2].u  = 0.0f ; CubeVertices[2].v  = 0.0f ;
   CubeVertices[3].u  = 1.0f ; CubeVertices[3].v  = 0.0f ;

   CubeVertices[4].u  = 1.0f ; CubeVertices[4].v  = 1.0f ;
   CubeVertices[5].u  = 1.0f ; CubeVertices[5].v  = 0.0f ;
   CubeVertices[6].u  = 0.0f ; CubeVertices[6].v  = 1.0f ;
   CubeVertices[7].u  = 0.0f ; CubeVertices[7].v  = 0.0f ;

   CubeVertices[8].u  = 1.0f ; CubeVertices[8].v  = 0.0f ;
   CubeVertices[9].u  = 0.0f ; CubeVertices[9].v  = 0.0f ;
   CubeVertices[10].u = 1.0f ; CubeVertices[10].v = 1.0f ;
   CubeVertices[11].u = 0.0f ; CubeVertices[11].v = 1.0f ;

   CubeVertices[12].u = 0.0f ; CubeVertices[12].v = 0.0f ;
   CubeVertices[13].u = 0.0f ; CubeVertices[13].v = 1.0f ;
   CubeVertices[14].u = 1.0f ; CubeVertices[14].v = 0.0f ;
   CubeVertices[15].u = 1.0f ; CubeVertices[15].v = 1.0f ;

   CubeVertices[16].u = 1.0f ; CubeVertices[16].v = 0.0f ;
   CubeVertices[17].u = 0.0f ; CubeVertices[17].v = 0.0f ;
   CubeVertices[18].u = 1.0f ; CubeVertices[18].v = 1.0f ;
   CubeVertices[19].u = 0.0f ; CubeVertices[19].v = 1.0f ;

   CubeVertices[20].u = 0.0f ; CubeVertices[20].v = 1.0f ;
   CubeVertices[21].u = 1.0f ; CubeVertices[21].v = 1.0f ;
   CubeVertices[22].u = 0.0f ; CubeVertices[22].v = 0.0f ;
   CubeVertices[23].u = 1.0f ; CubeVertices[23].v = 0.0f ;

   for (Counti=0; Counti<24; Counti++)
   {
    CubeVerticesOX[Counti] = CubeVerticesX[Counti];
    CubeVerticesOY[Counti] = CubeVerticesY[Counti];
    CubeVerticesOZ[Counti] = CubeVerticesZ[Counti];
   } // Preserve Original Values

} // InitCubeVertices

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DisplayNumber																 //
//																									 //
// DESCRIPTION:	This routine will display any number from 0 to 999 at any //
//						location on the screen.												 //
//////////////////////////////////////////////////////////////////////////////
void DisplayNumber(float X, float Y, int Number)
{
   // 1/32 = 0.03125f
   Vertices[0].u = Vertices[1].u = ((Number % 1000 ) / 100) * 0.03125f;
   Vertices[2].u = Vertices[3].u = Vertices[0].u + 0.03125f;
   
   Vertices[4].u = Vertices[5].u = ((Number % 100) / 10 ) * 0.03125f;
   Vertices[6].u = Vertices[7].u = Vertices[4].u + 0.03125f;
   
   Vertices[8].u = Vertices[9].u = (Number %10 )* 0.03125f;
   Vertices[10].u = Vertices[11].u = Vertices[8].u + 0.03125f;
   
   AddxAddy(Vertices,0,11,X,Y);
   //Display the 1st Number.
   msiRenderTriangle(&Vertices[0] , &Vertices[1] , &Vertices[2] ,  100);
   msiRenderTriangle(&Vertices[1] , &Vertices[2] , &Vertices[3] ,  100);
   // Display the 2nd Number.
   msiRenderTriangle(&Vertices[4] , &Vertices[5] , &Vertices[6] ,  100);
   msiRenderTriangle(&Vertices[5] , &Vertices[6] , &Vertices[7] ,  100);
   // Display the Third Number.
   msiRenderTriangle(&Vertices[8] , &Vertices[9] , &Vertices[10] ,  100);
   msiRenderTriangle(&Vertices[9] , &Vertices[10] , &Vertices[11] , 100);
   AddxAddy(Vertices,0,11,-X,-Y); 

} // DisplayNumber

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DisplayCharXYZ																 //
//																									 //
// DESCRIPTION:	This routine will display X, Y, or Z and their Status at	 //
//						any location on the screen. It takes the following			 //
//						Parameters:																 //
//						X, Y -->	The location on the screen where the data is to	 //
//									be displayed.												 //
//						Value --> 11 -> X, 12 -> Y & 13 -> Z								 //
//						You may display any Number or character in the numbers.raw//
//						by specifying its number in the Value Parameter. X is 11	 //
//						because it's the 11th Character in that file, etc.			 //
//						State --> -1 is Anticlockwise represented by char # 15 in //
//						the file, see below.  0 -> No rotation in the Axis, and 1 //
//						for Clockwise rotation.												 //
//////////////////////////////////////////////////////////////////////////////
void DisplayCharXYZ(float X, float Y, int Value, int State)
{
   switch (State)
   {
    case 0 : State = 10; break;
    case -1: State = 15; break;
    case 1 : State = 14; break;
   }

   // There are 32 characters of width 32 pixels. Thus 1/32 = 0.03125f.
   Vertices[0].u = Vertices[1].u = Value * 0.03125f; 
   Vertices[2].u = Vertices[3].u = Vertices[0].u + 0.03125f;
   
   Vertices[4].u = Vertices[5].u = State * 0.03125f;
   Vertices[6].u = Vertices[7].u = Vertices[4].u + 0.03125f;
   
   AddxAddy(Vertices,0,11,X,Y);
   //Display the Character.
   msiRenderTriangle(&Vertices[0] , &Vertices[1] , &Vertices[2] ,  100);
   msiRenderTriangle(&Vertices[1] , &Vertices[2] , &Vertices[3] ,  100);
   // Display its State.
   msiRenderTriangle(&Vertices[4] , &Vertices[5] , &Vertices[6] ,  100);
   msiRenderTriangle(&Vertices[5] , &Vertices[6] , &Vertices[7] ,  100);
   AddxAddy(Vertices,0,11,-X,-Y); 

} // DisplayCharXYZ

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DisplayBus																	 //
//																									 //
// DESCRIPTION:	This routine will display the Bus Mastering Status at a	 //
//						location on the screen.												 //
//////////////////////////////////////////////////////////////////////////////
void DisplayBus(float X, float Y, BOOL State)
{
   switch (State)
   {
    case TRUE:
	 	State = 17;
		break; // WAIT is TRUE i.e. BUS MASTERING is OFF
    case FALSE:
	 	State = 16;
		break; // WAIT is FALSE
    default:
		State = 10;
		break;
   }

   // There are 32 characters of width 32 pixels. Thus 1/32 = 0.03125f.
   Vertices[4].u = Vertices[5].u = State * 0.03125f;
   Vertices[6].u = Vertices[7].u = Vertices[4].u + 0.03125f;
   
   AddxAddy(Vertices,0,11,X,Y);
   // Display Bus State.
   msiRenderTriangle(&Vertices[4] , &Vertices[5] , &Vertices[6] ,  100);
   msiRenderTriangle(&Vertices[5] , &Vertices[6] , &Vertices[7] ,  100);
   AddxAddy(Vertices,0,11,-X,-Y); 

} // DisplayBus

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DrawCube																		 //
//																									 //
// DESCRIPTION:	This routine takes 3 parameters representing the amout of //
//						rotation done on the cube in each of the 3 axis.			 //
//////////////////////////////////////////////////////////////////////////////
void DrawCube (int AngleX, int AngleY, int AngleZ)
{
   int Counti, Again;

   for (Again = 0; Again <Repetitions; Again ++)
   {
    // Calculate the new Vertices
    for (Counti=0; Counti<24; Counti++)
    {
     // Using the preserved Values of the Coordinates protects the cube from
     // deforming after a few spins.
     // Spin around the z-axis
     CubeVerticesX[Counti]= CubeVerticesOX[Counti] * CosTable[AngleZ]
										+ CubeVerticesOY[Counti]*SinTable[AngleZ];
     CubeVerticesY[Counti]= CubeVerticesOY[Counti] * CosTable[AngleZ]
										- CubeVerticesOX[Counti]*SinTable[AngleZ];
     // Spin around the y-axis
     OX = CubeVerticesX[Counti];
     CubeVerticesX[Counti]=	CubeVerticesX[Counti] * CosTable[AngleY]
									- CubeVerticesOZ[Counti]*SinTable[AngleY];
     CubeVerticesZ[Counti]= OX * SinTable[AngleY]
									+ CubeVerticesOZ[Counti]*CosTable[AngleY];
     // Spin around the x-axis
     OY = CubeVerticesY[Counti];
     CubeVerticesY[Counti]=CubeVerticesY[Counti] * CosTable[AngleX]
           + CubeVerticesZ[Counti]*SinTable[AngleX];
     CubeVerticesZ[Counti]=CubeVerticesZ[Counti] * CosTable[AngleX]
           - OY*SinTable[AngleX];
     } // Calculating New Vertices
    // Translate New Vertices
    for (Counti=0; Counti < 24; Counti++)
    {
     CubeVertices[Counti].x = ((CubeVerticesX[Counti]+320) / (CubeVerticesZ[Counti]+Dist)) * Dist;
     CubeVertices[Counti].y = ((CubeVerticesY[Counti]+240) / (CubeVerticesZ[Counti]+Dist)) * Dist;
     // We are using a textbook formula for 'w' --> w = z/Dist + 1
	  // In the case of MSI, we need 1/w or invW.
	  // Sometimes, invW could be defined as 'some factor'/z, where z is from
	  // 0.0 to 1.0.  The calculation of invW all depends on the
	  // transformation and viewing model of the game/application.	  
	  CubeVertices[Counti].invW = 1.0/(CubeVerticesZ[Counti]/Dist+1);
    } // Translating New Vertices
   } // for Again.

   // Calculate the number of Frames per Sec.
   if (TotFrames>0)  
   {
    ftemps = (float)(clock() - temps);
    Result = ftemps/(float)CLOCKS_PER_SEC; 
   }

   TotFrames +=1;
	TotResult += Result;

   AvgFrames=(int)(((float)TotFrames/TotResult)+0.5);
   // Start new bench
   temps = clock();


   if (!msiStartFrame(TRUE, 50.0f, 50.0f, 50.0f, FALSE, 1.0f))
   {
    DBG_PRINT("msiStartFrame has failed.\n");
    exit(0);
   }

   // Setup the engine for the number textures
	SetDefaultParameters(&msiParameters);

   msiParameters.msiTexture.CacheOffset   = NUMBERS_OFFSET; 
   msiParameters.msiTexture.Width         = NUMBERS_SIZE_X;
   msiParameters.msiTexture.Height        = NUMBERS_SIZE_Y;
   msiParameters.msiTexture.Planes        = 8;
   msiParameters.msiTexture.Transparency  = TRUE;
   if (!msiSetParameters(&msiParameters))
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   } 

   // FPS
   DisplayNumber(504.0f,20.0f,AvgFrames);

   DisplayCharXYZ(20.0f,384.0f,11, XRotation);
   DisplayCharXYZ(20.0f,416.0f,12, YRotation); 
   DisplayCharXYZ(20.0f,448.0f,13, ZRotation);

   DisplayBus(504.0f, 448.0f, WAIT);
   
   // Setup the engine for the cube textures
   SetDefaultParameters(&msiParameters);

   msiParameters.msiTexture.CacheOffset   = FACES_OFFSET;
   msiParameters.msiTexture.Width         = FACES_SIZE_X;
   msiParameters.msiTexture.Height        = FACES_SIZE_Y;
   msiParameters.msiTexture.Planes        = 16;

   if (GOURAUD == TRUE)
     msiParameters.msiTexture.Enable		= FALSE;

   if (!msiSetParameters(&msiParameters))
   {
    DBG_PRINT("msiSetParameters has failed.\n");
    exit(0);
   } 

   if (VertexProductZ(&CubeVertices[0], &CubeVertices[2],&CubeVertices[3])<0.0f)
   {
    msiRenderTriangle(&CubeVertices[0], &CubeVertices[1],&CubeVertices[2], 100);
    msiRenderTriangle(&CubeVertices[1], &CubeVertices[2],&CubeVertices[3], 100);
   }

	if (GOURAUD == FALSE)
 	  if (!msiSetTextureOffset(FACES_OFFSET+1*2*FACES_SIZE_X*FACES_SIZE_Y))
     {
      DBG_PRINT("msiSetTextureOffset has failed.\n");
      exit(0);
     }
   if (VertexProductZ(&CubeVertices[4], &CubeVertices[6],&CubeVertices[7])<0.0f)
   {
    msiRenderTriangle(&CubeVertices[4], &CubeVertices[5],&CubeVertices[6], 100);
    msiRenderTriangle(&CubeVertices[5], &CubeVertices[6],&CubeVertices[7], 100);
   }

	if (GOURAUD == FALSE)
     if (!msiSetTextureOffset(FACES_OFFSET+2*2*FACES_SIZE_X*FACES_SIZE_Y))
     {
      DBG_PRINT("msiSetTextureOffset has failed.\n");
      exit(0);
     }
   if (VertexProductZ(&CubeVertices[8], &CubeVertices[10],&CubeVertices[11])<0.0f)
   {
    msiRenderTriangle(&CubeVertices[8], &CubeVertices[9],&CubeVertices[10], 100);
    msiRenderTriangle(&CubeVertices[9], &CubeVertices[10],&CubeVertices[11], 100);
   }

	if (GOURAUD == FALSE)
 	  if (!msiSetTextureOffset(FACES_OFFSET+3*2*FACES_SIZE_X*FACES_SIZE_Y))
     {
      DBG_PRINT("msiSetTextureOffset has failed.\n");
      exit(0);
     }
   if (VertexProductZ(&CubeVertices[12],&CubeVertices[14],&CubeVertices[15])<0.0f)
   {
    msiRenderTriangle(&CubeVertices[12], &CubeVertices[13],&CubeVertices[14], 100);
    msiRenderTriangle(&CubeVertices[13], &CubeVertices[14],&CubeVertices[15], 100);
   }

	if (GOURAUD == FALSE)
  	  if (!msiSetTextureOffset(FACES_OFFSET+4*2*FACES_SIZE_X*FACES_SIZE_Y))
     {
      DBG_PRINT("msiSetTextureOffset has failed.\n");
      exit(0);
     }
   if (VertexProductZ(&CubeVertices[16], &CubeVertices[18],&CubeVertices[19])<0.0f)
   {
    msiRenderTriangle(&CubeVertices[16], &CubeVertices[17],&CubeVertices[18], 100);
    msiRenderTriangle(&CubeVertices[17], &CubeVertices[18],&CubeVertices[19], 100);
   }
 
	if (GOURAUD == FALSE)
  	  if (!msiSetTextureOffset(FACES_OFFSET+5*2*FACES_SIZE_X*FACES_SIZE_Y))
     {
      DBG_PRINT("msiSetTextureOffset has failed.\n");
      exit(0);
     }
   if (VertexProductZ(&CubeVertices[20], &CubeVertices[22],&CubeVertices[23])<0.0f)
   {
    msiRenderTriangle(&CubeVertices[20], &CubeVertices[21],&CubeVertices[22], 100);
    msiRenderTriangle(&CubeVertices[21], &CubeVertices[22],&CubeVertices[23], 100);
   }
    
   if (!msiEndFrame(FALSE, 0, WAIT)) 
   {
    DBG_PRINT("msiEndFrame has failed.\n");
    return;
   }

} // DrawCube


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
// FUNCTION:	VertexProductZ																 //
//																									 //
// DESCRIPTION:	Calculates the vertex product by the right hand rule.		 //
//						It is used for Back Culling Purposes. If the vertex		 //
//						is > 0 then display the face.  By Right Hand Rule, we		 //
//						mean:																		 //
//	               01                                                        //
//						23																			 //
//////////////////////////////////////////////////////////////////////////////
float VertexProductZ(  T_msiVertex *Vertex1,T_msiVertex *Vertex2,T_msiVertex *Vertex3)
{
   float VPZ, DeltaBCx, DeltaBCy, DeltaBAx, DeltaBAy;

    DeltaBCx = Vertex2[0].x - Vertex1[0].x;
    DeltaBCy = Vertex2[0].y - Vertex1[0].y;
    DeltaBAx = Vertex2[0].x - Vertex3[0].x;
    DeltaBAy = Vertex2[0].y - Vertex3[0].y;

    VPZ = (DeltaBCx * DeltaBAy)-(DeltaBCy * DeltaBAx);

    return VPZ;

} // VertexProductZ

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
   msiParameters->msiTexture.KeyingAlpha			= 0x0;
   msiParameters->msiTexture.KeyingAlphaMask		= 0x0;

   msiParameters->msiTexture.msiLUT.pMem			= NULL;
   msiParameters->msiTexture.msiLUT.pHeap			= NULL;
   msiParameters->msiTexture.msiLUT.CacheOffset	= 0;

   msiParameters->msiDepth.Enable					= FALSE;
   msiParameters->msiDepth.Compare					= msiCMP_LEQUAL;
   msiParameters->msiDepth.Protect					= FALSE;

   msiParameters->msiColor.Dither					= TRUE;
   msiParameters->msiColor.Protect					= FALSE;

} // SetDefaultParameters


