#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <Windows.h>

using namespace std;
#pragma pack( push, 2 )

struct BitmapFileHeader
{
	unsigned short	BmpTag;				//header field - BM
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

struct BGRA
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;

	BGRA Lerp(BGRA B, float t)
	{
		BGRA Result;
		Result.Blue = Blue + (B.Blue - Blue) * t;
		Result.Green = Green + (B.Green - Green) * t;
		Result.Red = Red + (B.Red - Red) * t;
		Result.Alpha = Alpha + (B.Alpha - Alpha) * t;
		return Result;
	}

	void Scale(float t)
	{
		Blue = __min(Blue * t, 0xFF);
		Green = __min(Green * t, 0xFF);
		Red = __min(Red * t, 0xFF);
		Alpha = __min(Alpha * t, 0xFF);
	}
};

struct BMPColorHeader32
{
	uint32_t RedMask;			// Bit mask for the red channel
	uint32_t GreenMask;			// Bit mask for the green channel
	uint32_t BlueMask;			// Bit mask for the blue channel
	uint32_t AlphaMask;			// Bit mask for the alpha channel
	uint32_t ColorSpaceType;	// Default "sRGB" (0x73524742)
	uint32_t Unused[16];		// Unused data for sRGB color space

	BMPColorHeader32() :
		RedMask(0x00FF0000),
		GreenMask(0x0000FF00),
		BlueMask(0x000000FF),
		AlphaMask(0xFF000000),
		ColorSpaceType(0x73524742)
	{
		memset(Unused, 0, 16 * sizeof(uint32_t));
	}
};

#pragma pack( pop )

namespace
{
	BGRA Gray75 = { 0xC0, 0xC0, 0xC0 };
	BGRA Gray50 = { 0x80, 0x80, 0x80 };
	BGRA Blue = { 0xFF, 0x00, 0x00 };
	BGRA Green = { 0x00, 0xFF, 0x00 };
	BGRA Red = { 0x00, 0x00, 0xFF };
	BGRA White = { 0xFF, 0xFF, 0xFF };
}

class BMPImporter
{
public:
	BMPImporter();
	BMPImporter(const char* fileName);
	~BMPImporter();

	void			LoadBuffer(uint8_t* PixelData, int Width = 256, int Height = 256, int Bpp = 24);
	void			WriteBmpFromBuffer(const char* fileName, uint8_t* PixelData, int Width = 256, int Height = 256, int Bpp = 24);
	void			Write(const char* fileName);
	bool			WriteBmp8Bit(const char* FileName);
	bool			WriteBmp8Bit(uint8_t* Buffer, const char* FileName, int Width, int Height);
	void			Resize(int Width, int Height);
	int				GetWidth();
	int				GetHeight();
	int				GetBitPerPixel();
	BGRA			GetPixel(int x, int y);
	void			SetPixel(int x, int y, BGRA rgb);
	uint8_t*		GetPixelData();
	vector<uint8_t>	GetPixelData32();
	DIBHeader*		GetBitmapHeader();

private:
	bool ReadBMP(const char* fileName);
	BGRA BilinearInterp(float Rx, float Ry, int w, int h);

	BitmapFileHeader	m_BMPHeader;
	DIBHeader			m_DibHeader;
	BMPColorHeader32	m_BGRAHeader;
	size_t				m_PixelRowSize;
	size_t				m_Channels;
	size_t				m_PixelDataSize = 0;
	bool				m_TempBuffer;
	uint8_t*			m_PixelData;

};

