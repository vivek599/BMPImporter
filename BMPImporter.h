#pragma once

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct RGB
{
	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
};

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

class BMPImporter
{

public:
	BMPImporter( const char* fileName );
	~BMPImporter();

	uint32_t make_stride_aligned(uint32_t align_stride, uint32_t row_stride)
	{
		uint32_t new_stride = row_stride;
		while (new_stride % align_stride != 0) 
		{
			new_stride++;
		}

		return new_stride;
	}

private:
	BitmapFileHeader	BMPHeader;
	DIBHeader			DibHeader;

	vector<uint8_t>		PixelData;
	RGB**				RGBData;
	
};

