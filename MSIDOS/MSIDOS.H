/****************************************************************************
*          name: msidos.h
*
*   description: Matrox Simple Interface for DOS global include file.
*
******************************************************************************/

#ifndef _MSIDOS_H_
#define _MSIDOS_H_

#pragma pack (1)

// Debug flags
//
#define msiDBG_Release       0
#define msiDBG_DumpToFile    0x00000001     /* Dump information to 'msidebug.log' */
#define msiDBG_DumpToFront   0x00000002     /* Render in front buffer, no flip */

// Compare codes used to specify depth compare function and other ...
//
#define msiCMP_NEVER    1
#define msiCMP_LESS     2
#define msiCMP_EQUAL    3
#define msiCMP_LEQUAL   4
#define msiCMP_GREATER  5
#define msiCMP_NOTEQUAL 6
#define msiCMP_GEQUAL   7
#define msiCMP_ALWAYS   8

#define DECAL           0x00000001
#define MODULATE        0x00000002

typedef int             BOOL;        /* boolean      */
typedef unsigned char   BYTE;        /* 8-bit datum  */
typedef unsigned short  WORD;        /* 16-bit datum */
typedef short           SHORT;       /* 16-bit datum */
typedef unsigned long   DWORD;       /* 32-bit datum */
typedef long            LONG;        /* 32-bit datum */
typedef unsigned long   ULONG;       /* 32-bit datum */

typedef unsigned char   _far*       LPBYTE;
typedef          DWORD  _far*       LPDWORD;

// Frame buffer format. At init time, the client receives a pointer to this
// structure to know about the actual frame buffer format. The Color section
// applies to the back buffer and the Depth section to the Z buffer.
//
typedef struct _T_msiInfo
   {
   SHORT  VersionMajor;    
   SHORT  VersionMinor;    

   struct //_msiColor
      {
      LONG                 BytesPerPixel;
      LONG                 rDepth;
      LONG                 gDepth;
      LONG                 bDepth;
      LONG                 aDepth;
      LONG                 rBitShift;
      LONG                 gBitShift;
      LONG                 bBitShift;
      LONG                 aBitShift;
      LONG                 Pitch;            // Value expressed in bytes
      volatile LPBYTE      pMem;
      LONG                 MemSize;          // Value expressed in bytes

      } msiColor;

   struct //_msiDepth
      { 
      LONG                 BytesPerPixel;
      LONG                 zDepth;
      LONG                 zBitShift;
      LONG                 Pitch;            // Value expressed in bytes
      volatile LPBYTE      pMem;
      LONG                 MemSize;          // Value expressed in bytes

      } msiDepth;

   struct // msiTexture
      {
      volatile LPBYTE      pMem;             // Texture Cache memory
      LONG                 MemSize;

      } msiTexture;

   } T_msiInfo;

// Vertex format. This vertex format supports 2D, 3D, gouraud shading and
// texture mapping with lighting, perspective, decal, etc ...
//
typedef struct _T_msiVertex
   { 
   float x, y, z, invW, r, g, b, mr, mg, mb, u, v;

   } T_msiVertex;

// Rendering parameters. This structure is used to control the rendering
// process specifying the different attributes that control texture mapping,
// depth compare and color processing.
//
typedef struct _T_msiParameters
   {
   struct //_msiTexture
      {
      BOOL     Enable;
      long     Width, Height;
      long     Planes;
      LPBYTE   pMem;
      LPBYTE   pHeap;
      long     CacheOffset;

      struct //_msiLUT
         {
         LPBYTE   pMem;
         LPBYTE   pHeap;
         long     CacheOffset;
         } msiLUT;

      BOOL     Clamp_u, Clamp_v;
      BOOL     Modulate;
      BOOL     Decal;
      BOOL     Transparency;
      WORD     KeyingColor,  KeyingColorMask;
      WORD     KeyingAlpha,  KeyingAlphaMask;    // !!! 1 bit value !!!

      } msiTexture;

   struct //_msiDepth
      {
      BOOL     Enable;
      DWORD    Compare;
      BOOL     Protect;

      } msiDepth;

   struct //_msiColor
      {
      BOOL    Dither;
      BOOL    Protect;

      } msiColor;

   } T_msiParameters;

// Z plane parameters. This structure describe the coordinates and color of
// a plane when using "msiRenderZPlane" function call.
//
typedef struct _T_msiZPlane 
   {
   long  x, y,             // Plane top-left coordinate
         u, v,             // Texture top-left coordinate
         xWidth, yHeight,  // Plane dimension
         uWidth, vHeight;  // Texture dimension
   float z,                // Plane's depth
         r, g, b,          // Plane's color
         mr, mg, mb;       // Plane's texture modulation
   } T_msiZPlane;

// Memory status block. This structure is used to know if a drawing
// operation is completed with function IsMemoryBusy()
//
typedef struct _T_msiMemoryStatus
   {
   DWORD reserved[4];
   } T_msiMemoryStatus;

// Function prototypes
//   
#ifdef __cplusplus

// Overall functions

   extern "C" LPBYTE			   msiAllocTextureHeap(LONG Pages);
   extern "C" BOOL			   msiExit();
   extern "C" BOOL			   msiIsMemoryBusy( T_msiMemoryStatus *pMemStatus );   
   extern "C" BOOL			   msiFreeTextureHeap(LPBYTE pTextureHeap);
   extern "C" T_msiInfo*	   msiInit( LONG Width, LONG Height, LONG Planes, BOOL ZBuffer, BOOL Debug);

// Frame functions

   extern "C" BOOL			   msiEndFrame(BOOL Dump, LONG Frame, BOOL Wait);
   extern "C" BOOL			   msiSetParameters(T_msiParameters* pParameters);
   extern "C" BOOL			   msiStartFrame(BOOL ClearRGB, float r, float g, float b, BOOL ClearZ, float z);

// Context Functions          
 
   extern "C" BOOL			   msiSetDepthCompare(DWORD Cmp);
   extern "C" BOOL			   msiSetDitherEnable(BOOL Dither);
   extern "C" BOOL			   msiSetTextureBlend(ULONG msiTexBlend);
   extern "C" BOOL			   msiSetTextureCLUT4(DWORD Offset, DWORD Index );
   extern "C" BOOL			   msiSetTextureCLUT8(DWORD Offset );
   extern "C" BOOL			   msiSetTextureEnable(BOOL Enable);
   extern "C" BOOL			   msiSetTextureOffset(DWORD Offset );
   extern "C" BOOL			   msiSetTexturePlanes(LONG Planes);
   extern "C" BOOL			   msiSetTextureSize(LONG Width, LONG Height);
   extern "C" BOOL			   msiSetTextureTransparency(DWORD MaskKey );
   extern "C" BOOL			   msiSetTextureWrap(BOOL ClampU, BOOL ClampV);
 
// Rendering functions

   extern "C" BOOL			   msiBlitRect (  LPBYTE Pheap, 
                                             LPBYTE Pmemory, 
									                  T_msiMemoryStatus *pMemStatus,
									                  WORD SourcePitch, WORD SourcePlanes, 
									                  WORD BlitWidth, WORD BlitHeight, 
									                  DWORD DestinationOff, 
									                  DWORD KeyingColor, DWORD KeyingMask );
   extern "C" BOOL			   msiDrawSingleLine( DWORD color, DWORD XYStart, DWORD XYEnd, DWORD LineStyle );
   extern "C" BOOL			   msiRenderTriangle(T_msiVertex* pV0, T_msiVertex* pV1, T_msiVertex* pV2, BYTE Opacity);
   extern "C" BOOL			   msiRenderZPlane(T_msiZPlane* pZPlane, BYTE Opacity);
	extern "C" BOOL 				msiSplashScreen();
#else

// Overall functions

   LPBYTE		   msiAllocTextureHeap(LONG Pages);
   BOOL			   msiExit();
   BOOL			   msiIsMemoryBusy( T_msiMemoryStatus *pMemStatus );   
   BOOL			   msiFreeTextureHeap(LPBYTE pTextureHeap);
   T_msiInfo*	   msiInit( LONG Width, LONG Height, LONG Planes, BOOL ZBuffer, BOOL Debug);

// Frame functions

   BOOL			   msiEndFrame(BOOL Dump, LONG Frame, BOOL Wait);
   BOOL			   msiSetParameters(T_msiParameters* pParameters);
   BOOL			   msiStartFrame(BOOL ClearRGB, float r, float g, float b, BOOL ClearZ, float z);

// Context Functions          
 
   BOOL			   msiSetDepthCompare(DWORD Cmp);
   BOOL			   msiSetDitherEnable(BOOL Dither);
   BOOL			   msiSetTextureBlend(ULONG msiTexBlend);
   BOOL			   msiSetTextureCLUT4(DWORD Offset, DWORD Index );
   BOOL			   msiSetTextureCLUT8(DWORD Offset );
   BOOL			   msiSetTextureEnable(BOOL Enable);
   BOOL			   msiSetTextureOffset(DWORD Offset );
   BOOL			   msiSetTexturePlanes(LONG Planes);
   BOOL			   msiSetTextureSize(LONG Width, LONG Height);
   BOOL			   msiSetTextureTransparency(DWORD MaskKey );
   BOOL			   msiSetTextureWrap(BOOL ClampU, BOOL ClampV);
 
// Rendering functions

   BOOL			   msiBlitRect (  LPBYTE Pheap, 
                                 LPBYTE Pmemory, 
	 				                  T_msiMemoryStatus *pMemStatus,
	 				                  WORD SourcePitch, WORD SourcePlanes, 
	 				                  WORD BlitWidth, WORD BlitHeight, 
	 				                  DWORD DestinationOff, 
	 				                  DWORD KeyingColor, DWORD KeyingMask );
   BOOL			   msiDrawSingleLine( DWORD color, DWORD XYStart, DWORD XYEnd, DWORD LineStyle );
   BOOL			   msiRenderTriangle(T_msiVertex* pV0, T_msiVertex* pV1, T_msiVertex* pV2, BYTE Opacity);
   BOOL			   msiRenderZPlane(T_msiZPlane* pZPlane, BYTE Opacity);
	BOOL 				msiSplashScreen();
#endif
  
#pragma pack ()

#endif // _MSIDOS_H_


