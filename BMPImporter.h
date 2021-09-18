#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>

using namespace std;

#pragma pack( push, 1 )

struct BitmapFileHeader
{
	char			Header[2];			//header field - BM
	unsigned int	Size;				//The size of the BMP file in bytes
	unsigned int	Reserved;			//Reserved
	unsigned int	Offset;				//starting address, of the byte where the bitmap image data (pixel array) can be found
};

struct DIBHeader
{
	unsigned int		DIBHeaderSize;			//the size of this header, in bytes (40)
	int					Width;					//bitmap width in pixels (signed integer)
	int					Height;					//bitmap height in pixels (signed integer)
	unsigned short		ColorPlanes;			//the number of color planes (must be 1)
	unsigned short		BitsPerPixel;			//the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32
	unsigned int		Compression;			//compression 
	unsigned int		ImageSize;				//image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps
	int					HorizontalResolution;	//horizontal resolution of the image. (pixel per metre, signed integer)
	int					VerticleResolution;		//vertical resolution of the image. (pixel per metre, signed integer)
	unsigned int		NumColorInPalette;		//number of colors in the color palette, or 0 to default to 2 pow n
	unsigned int		ImportantColorsUsed;	//number of important colors used, or 0 when every color is important; generally ignored
}; 

struct RGB
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
};

struct BMPColorHeader32 
{
	uint32_t RedMask;			// Bit mask for the red channel
	uint32_t GreenMask;		// Bit mask for the green channel
	uint32_t BlueMask;			// Bit mask for the blue channel
	uint32_t AlphaMask;		// Bit mask for the alpha channel
	uint32_t ColorSpaceType;	// Default "sRGB" (0x73524742)
	uint32_t Unused[16];		// Unused data for sRGB color space
	
	BMPColorHeader32() :
		RedMask(0x00ff0000),
		GreenMask(0x0000ff00),
		BlueMask(0x000000ff),
		AlphaMask(0xff000000),
		ColorSpaceType(0x73524742)
	{
		memset(Unused, 0, 16*sizeof(uint32_t));
	}
};

#pragma pack( pop )

class BMPImporter
{

public:
	BMPImporter( const char* fileName );
	~BMPImporter();

	void Write(const char* fileName);

	int GetWidth();
	int GetHeight();
	int GetBitPerPixel();
	RGB GetPixel(int x, int y);
	void SetPixel(int x, int y, RGB rgb);

private:
	bool ReadBMP(const char* fileName);

	BitmapFileHeader	m_BMPHeader;
	DIBHeader			m_DibHeader;
	BMPColorHeader32	m_BGRAHeader;
	size_t				m_PixelRowSize;

	vector<uint8_t>		m_PixelData;
	
};

