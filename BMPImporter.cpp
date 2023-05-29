#include "BMPImporter.h"

BMPImporter::BMPImporter(const char* fileName) : m_BMPHeader{}, m_DibHeader{}
{
	if( !ReadBMP(fileName) )
		cout << "Bitmap Reading failed..." << endl;
}

BMPImporter::BMPImporter(uint8_t* PixelData, int Width = 256, int Height = 256, int Bpp = 24)
{
	m_Channels = Bpp / 8;
	
	if (m_Channels < 3)
	{
		cout << "24Bit or 32 Bit Bmps only..." << endl;
		system("pause");
		return;
	}
	
	m_BMPHeader.Header[0] = 'B';
	m_BMPHeader.Header[1] = 'M';
	m_BMPHeader.Offset = 54;
	m_BMPHeader.Reserved = 0;
	m_BMPHeader.Size = Width * Height * m_Channels + 54;
	
	m_DibHeader.BitsPerPixel = Bpp;
	m_DibHeader.Compression = BI_RGB;
	m_DibHeader.DIBHeaderSize = 40;
	m_DibHeader.ColorPlanes = 1;
	m_DibHeader.Height = Height;
	m_DibHeader.Width = Width;
	m_DibHeader.ImageSize = Width * Height * m_Channels;
	m_DibHeader.NumColorInPalette = 0;
	m_DibHeader.ImportantColorsUsed = 0;
	m_DibHeader.HorizontalResolution = 0;
	m_DibHeader.VerticleResolution = 0;

	m_PixelRowSize = Width * m_Channels + (4 - (Width * m_Channels) % 4) % 4;

	m_PixelData.clear();
	m_PixelData.resize(m_PixelRowSize * Height);
	int k = 0;
	for (size_t h = 0; h < Height; h++)
	{
		for (size_t w = 0; w < Width; w++)
		{
			if (m_Channels >= 3)
			{
				m_PixelData[k + 0] = PixelData[k + 0];
				m_PixelData[k + 1] = PixelData[k + 1];
				m_PixelData[k + 2] = PixelData[k + 2];
				k += 3;

				if (m_Channels == 4)
				{
					m_PixelData[k + 3] = PixelData[k + 3];
					k++;
				}
			}
		}
	}
}

BGRA BMPImporter::BilinearInterp(float Rx, float Ry, int w, int h)
{
	BGRA X0 = GetPixel(w, h);
	BGRA X1 = GetPixel(w + 1, h);

	BGRA Y0 = GetPixel(w, h + 1);
	BGRA Y1 = GetPixel(w + 1, h + 1);

	X0 = X0.Lerp(X1, Rx);
	Y0 = Y0.Lerp(Y1, Rx);
	X0 = X0.Lerp(Y0, Ry);
	X0 = X0.Lerp(Y0, Ry);

	return X0;
}

void BMPImporter::Resize(int Width, int Height)
{
	if (Width && Height)
	{
		int OldWidth = m_DibHeader.Width;
		int OldHeight = m_DibHeader.Height;

		int NewWidth = Width;
		int NewHeight = Height;

		float Rx = float(NewWidth) / float(OldWidth);
		float Ry = float(NewHeight) / float(OldHeight);

		if (Rx >= 4.0001f || Ry >= 4.0001f)
			return;

		int m_NewPixelRowSize = (((size_t)m_DibHeader.BitsPerPixel * abs(NewWidth) + 31) / 32) * 4;

		vector<uint8_t> m_NewPixelData(m_NewPixelRowSize * abs(NewHeight), 0);
		for (size_t h = 0; h < OldHeight - 1; h++)
		{
			for (size_t w = 0; w < OldWidth - 1; w++)
			{
				int NewPixelX = floor(w * Rx);
				int NewPixelY = floor(h * Ry);

				BGRA X0 = BilinearInterp(Rx, Ry, w, h);
				X0.Scale(4.0f);

				int PixelIndex = m_NewPixelRowSize * NewPixelY + NewPixelX * m_Channels;
				if (m_Channels >= 3)
				{
					m_NewPixelData[PixelIndex + 0] = X0.Blue;
					m_NewPixelData[PixelIndex + 1] = X0.Green;
					m_NewPixelData[PixelIndex + 2] = X0.Red;
					if (m_Channels == 4)
						m_NewPixelData[PixelIndex + 3] = X0.Alpha;
				}

				continue;

				int DiffX = NewPixelX - w;
				int DiffY = NewPixelY - h;

				int Cx = ceil(Rx);
				int Cy = ceil(Ry);

				if (Rx > 1.0f && Ry < 1.0f)
				{
					int DiffX = NewPixelX - w;
				
					if (DiffX > 0)
					{
						BGRA X0 = BilinearInterp(Rx, Ry, w, h);
						//X0.Scale(4.0f);

						BGRA X1 = BilinearInterp(Rx, Ry, w + 1, h);
						//X1.Scale(4.0f);

						for (size_t d = 0; d < ceil(Rx); d++)
						{
							BGRA X2 = X0.Lerp(X1, float(d) / Cx);
							int PixelIndex = m_NewPixelRowSize * NewPixelY + (NewPixelX + d) * m_Channels;
							if (m_Channels >= 3)
							{
								m_NewPixelData[PixelIndex + 0] = X2.Blue;
								m_NewPixelData[PixelIndex + 1] = X2.Green;
								m_NewPixelData[PixelIndex + 2] = X2.Red;
								if (m_Channels == 4)
									m_NewPixelData[PixelIndex + 3] = X2.Alpha;
							}
						}
					}
				}
				else if (Rx < 1.0f && Ry > 1.0f)
				{
					if (DiffY > 0)
					{
						BGRA Y0 = BilinearInterp(Rx, Ry, w, h);
						//Y0.Scale(4.0f);

						BGRA Y1 = BilinearInterp(Rx, Ry, w, h + 1);
						//Y1.Scale(4.0f);

						for (size_t d = 0; d < Cy; d++)
						{
							BGRA Y2 = Y0.Lerp(Y1, float(d) / Cy);
							int PixelIndex = m_NewPixelRowSize * (NewPixelY + d) + NewPixelX * m_Channels;
							if (m_Channels >= 3)
							{
								m_NewPixelData[PixelIndex + 0] = Y2.Blue;
								m_NewPixelData[PixelIndex + 1] = Y2.Green;
								m_NewPixelData[PixelIndex + 2] = Y2.Red;
								if (m_Channels == 4)
									m_NewPixelData[PixelIndex + 3] = Y2.Alpha;
							}
						}
					}
				}
				else if (Rx > 1.0f && Ry > 1.0f)
				{
					for (size_t c0 = 0; c0 < Cy; c0++)
					{
						for (size_t c1 = 0; c1 < Cx; c1++)
						{
							BGRA X0 = BilinearInterp(Rx, Ry, w, h);
							//X0.Scale(4.0f);

							BGRA X1 = BilinearInterp(Rx, Ry, w + 1, h);
							//X1.Scale(4.0f);

							BGRA X2 = X0.Lerp(X1, float(c1) / Cx);
						
							BGRA Y0 = BilinearInterp(Rx, Ry, w, h);
							//X0.Scale(4.0f);

							BGRA Y1 = BilinearInterp(Rx, Ry, w, h + 1);
							//X1.Scale(4.0f);

							BGRA Y2 = Y0.Lerp(Y1, float(c1) / Cx);
						
							BGRA X3 = X2.Lerp(Y2, float(c0) / Cy);

							int PixelIndex = m_NewPixelRowSize * (NewPixelY + c0) + (NewPixelX + c1) * m_Channels;
							if (m_Channels >= 3)
							{
								m_NewPixelData[PixelIndex + 0] = X3.Blue;
								m_NewPixelData[PixelIndex + 1] = X3.Green;
								m_NewPixelData[PixelIndex + 2] = X3.Red;
								if (m_Channels == 4)
									m_NewPixelData[PixelIndex + 3] = X3.Alpha;
							}
						}
					}
				}
				else if (Rx < 1.0f && Ry < 1.0f)
				{
					BGRA X0 = BilinearInterp(Rx, Ry, w, h);
					X0.Scale(4.0f);

					int PixelIndex = m_NewPixelRowSize * NewPixelY + NewPixelX * m_Channels;
					if (m_Channels >= 3)
					{
						m_NewPixelData[PixelIndex + 0] = X0.Blue;
						m_NewPixelData[PixelIndex + 1] = X0.Green;
						m_NewPixelData[PixelIndex + 2] = X0.Red;
						if (m_Channels == 4)
							m_NewPixelData[PixelIndex + 3] = X0.Alpha;
					}
				}

			}
		}

		m_DibHeader.Width = NewWidth;
		m_DibHeader.Height = NewHeight;
		m_DibHeader.ImageSize = NewWidth * NewHeight * m_Channels;
		m_BMPHeader.Size = m_DibHeader.ImageSize + 54;
		m_PixelRowSize = (((size_t)m_DibHeader.BitsPerPixel * abs(m_DibHeader.Width) + 31) / 32) * 4;

		m_PixelData.clear();
		m_PixelData = m_NewPixelData;
	}
	else
	{
		cout << "Incorrect Dimensions..." << endl;
		return;
	}

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
	
	m_Channels = m_DibHeader.BitsPerPixel / 8;

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
	BGRA Color;
	
	if (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight())
	{
		size_t channels = m_DibHeader.BitsPerPixel / 8;

		Color.Blue = m_PixelData[y * m_PixelRowSize + x * channels + 0];
		Color.Green = m_PixelData[y * m_PixelRowSize + x * channels + 1];
		Color.Red = m_PixelData[y * m_PixelRowSize + x * channels + 2];
		Color.Alpha = 255;

		if (channels == 4)
			Color.Alpha = m_PixelData[y * m_PixelRowSize + x * channels + 3];
	}
	else
	{
		cout << "Incorrect Dimensions..." << endl;
	}

	return Color;
}

uint8_t* BMPImporter::GetPixelData()
{
	return m_PixelData.data();
}

void BMPImporter::SetPixel(int x, int y, BGRA bgr)
{
	if (x >=0 && x < GetWidth() && y >=0 && y < GetHeight())
	{
		m_PixelData[y * m_PixelRowSize + x * m_Channels + 0] = bgr.Blue;
		m_PixelData[y * m_PixelRowSize + x * m_Channels + 1] = bgr.Green;
		m_PixelData[y * m_PixelRowSize + x * m_Channels + 2] = bgr.Red;

		if (m_Channels == 4)
			m_PixelData[y * m_PixelRowSize + x * m_Channels + 3] = bgr.Alpha;
	}
	else
	{
		cout << "Incorrect Dimensions..." << endl;
	}
}

DIBHeader* BMPImporter::GetBitmapHeader()
{
	return &m_DibHeader;
}
