#include "BMPImporter.h"

BMPImporter::BMPImporter(const char* fileName) : m_BMPHeader{}, m_DibHeader{}
{
	assert( ReadBMP(fileName) );

}

bool BMPImporter::ReadBMP(const char* fileName)
{
	ifstream infile;

	infile.open(fileName, ios_base::binary);

	if (!infile.is_open())
	{
		cout << "Unable to open file : " << fileName << endl;
		return false;
	}

	infile.read(reinterpret_cast<char*>(&m_BMPHeader.Header), 2);
	infile.read(reinterpret_cast<char*>(&m_BMPHeader.Size), 3 * sizeof(int));

	if (m_BMPHeader.Header[0] != 'B' || m_BMPHeader.Header[1] != 'M')
	{
		cout << "Incorrect Header" << m_BMPHeader.Header[0] << m_BMPHeader.Header[1] << endl;
		return false;
	}

	cout << "Reading BitmapFileHeader" << endl;
	cout << "Header\t\t: " << m_BMPHeader.Header[0] << m_BMPHeader.Header[1] << endl;
	cout << "Size\t\t: " << m_BMPHeader.Size << endl;
	cout << "Reserved\t: " << m_BMPHeader.Reserved << endl;
	cout << "Offset\t\t: " << m_BMPHeader.Offset << endl;

	cout << "Reading DIBHeader" << endl;
	infile.read(reinterpret_cast<char*>(&m_DibHeader), sizeof(DIBHeader));

	cout << "DIBHeaderSize--------: " << m_DibHeader.DIBHeaderSize << endl;
	cout << "Width----------------: " << m_DibHeader.Width << endl;
	cout << "Height---------------: " << m_DibHeader.Height << endl;
	cout << "ColorPlanes----------: " << m_DibHeader.ColorPlanes << endl;
	cout << "BitsPerPixel---------: " << m_DibHeader.BitsPerPixel << endl;
	cout << "Compression----------: " << m_DibHeader.Compression << endl;
	cout << "ImageSize------------: " << m_DibHeader.ImageSize << endl;
	cout << "HorizontalResolution-: " << m_DibHeader.HorizontalResolution << endl;
	cout << "VerticleResolution---: " << m_DibHeader.VerticleResolution << endl;
	cout << "NumColorInPalette----: " << m_DibHeader.NumColorInPalette << endl;
	cout << "ImportantColorsUsed--: " << m_DibHeader.ImportantColorsUsed << endl;

	if (m_DibHeader.BitsPerPixel != 24 && m_DibHeader.BitsPerPixel != 32)
	{
		cout << "Incorrect BitsPerPixel" << m_DibHeader.BitsPerPixel << endl;
		return false;
	}

	if (m_DibHeader.BitsPerPixel == 32)
	{
		infile.read(reinterpret_cast<char*>(&m_BGRAHeader), sizeof(BMPColorHeader32));
	}

	cout << "RedMask---------: " << m_BGRAHeader.RedMask << endl;
	cout << "GreenMask-------: " << m_BGRAHeader.GreenMask << endl;
	cout << "BlueMask--------: " << m_BGRAHeader.BlueMask << endl;
	cout << "AlphaMask-------: " << m_BGRAHeader.AlphaMask << endl;
	cout << "ColorSpaceType--: " << m_BGRAHeader.ColorSpaceType << endl;
	cout << "Unused[0]-------: " << m_BGRAHeader.Unused[0]	<< endl;

	infile.seekg(m_BMPHeader.Offset, ios_base::beg);

	m_PixelRowSize = (((size_t)m_DibHeader.BitsPerPixel * abs(m_DibHeader.Width) + 31) / 32) * 4;

	m_PixelData.resize(m_PixelRowSize * abs(m_DibHeader.Height));
	infile.read(reinterpret_cast<char*>(m_PixelData.data()), m_PixelData.size());
	infile.close();

	return true;
}

void BMPImporter::Write(const char* fileName)
{
	ofstream outFile(fileName, ios_base::binary);
	outFile.write(m_BMPHeader.Header, 2);
	outFile.write(reinterpret_cast<char*>(&m_BMPHeader.Size), 3 * sizeof(int));
	outFile.write(reinterpret_cast<char*>(&m_DibHeader), sizeof(DIBHeader));
	if (m_DibHeader.BitsPerPixel == 32)
	{
		outFile.write(reinterpret_cast<char*>(&m_BGRAHeader), sizeof(BMPColorHeader32));
	}
	outFile.write(reinterpret_cast<char*>(m_PixelData.data()), m_PixelData.size());
	outFile.close();
}

BMPImporter::~BMPImporter()
{

}

int BMPImporter::GetWidth()
{
	return m_DibHeader.Width;
}

int BMPImporter::GetHeight()
{
	return m_DibHeader.Height;
}

int BMPImporter::GetBitPerPixel()
{
	return m_DibHeader.BitsPerPixel;
}

BGRA BMPImporter::GetPixel( int x, int y )
{
	assert( x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight() );

	size_t channels = m_DibHeader.BitsPerPixel / 8;

	uint8_t B = m_PixelData[y * m_PixelRowSize + x * channels + 0];
	uint8_t G = m_PixelData[y * m_PixelRowSize + x * channels + 1];
	uint8_t R = m_PixelData[y * m_PixelRowSize + x * channels + 2];
	uint8_t A = 255;

	if (channels == 4) 
		A = m_PixelData[y * m_PixelRowSize + x * channels + 3];

	return { B, G, R, A };
}

uint8_t* BMPImporter::GetPixelData()
{
	return m_PixelData.data();
}

void BMPImporter::SetPixel(int x, int y, BGRA bgr)
{
	if (x >=0 && x < GetWidth() && y >=0 && y < GetHeight())
	{
		size_t channels = m_DibHeader.BitsPerPixel / 8;

		m_PixelData[y * m_PixelRowSize + x * channels + 0] = bgr.Blue;
		m_PixelData[y * m_PixelRowSize + x * channels + 1] = bgr.Green;
		m_PixelData[y * m_PixelRowSize + x * channels + 2] = bgr.Red;

		if (channels == 4)
		{
			m_PixelData[y * m_PixelRowSize + x * channels + 3] = bgr.Alpha;
		}
	}
}

DIBHeader* BMPImporter::GetBitmapHeader()
{
	return &m_DibHeader;
}
