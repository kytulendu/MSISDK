//
// Header file containing some BMP file information structures and defines 
//

/* constants for the biCompression field */
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

#define BI_BITFIELDS  3L

typedef struct tagBITMAPINFOHEADER
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER
{
//    UINT    bfType;
    WORD    bfType;
    DWORD   bfSize;
//    UINT    bfReserved1;
    WORD    bfReserved1;
//    UINT    bfReserved2;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;
