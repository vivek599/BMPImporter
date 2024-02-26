#include "BMPImporter.h"

BMPImporter::BMPImporter()
{
	m_BMPHeader = {};
	m_DibHeaderV5 = {};
	m_BGRAHeader = {};
	m_PixelRowSize = 0;
	m_Channels = 0;
	m_PixelDataSize = 0;
	m_TempBuffer = false;
	m_PixelData = nullptr;

}

BMPImporter::BMPImporter(const char* fileName)  : BMPImporter()
{
	if( !ReadBMP(fileName) )
		cout << "Bitmap Reading failed..." << endl;
}

void BMPImporter::LoadBuffer(uint8_t* PixelData, int Width, int Height, int Bpp)
{
	m_Channels = Bpp / 8;
	
	if (m_Channels < 3)
	{
		cout << "24Bit or 32 Bit Bmps only..." << endl;
		system("pause");
		return;
	}
	
	m_BMPHeader.BmpTag = 'MB';
	m_BMPHeader.Offset = 54;
	m_BMPHeader.Reserved = 0;
	m_BMPHeader.Size = Width * Height * m_Channels + 54;
	
	m_DibHeaderV5.BitsperPixel = Bpp;
	m_DibHeaderV5.Compression = BI_RGB;
	m_DibHeaderV5.DIBHeaderSize = 40;
	m_DibHeaderV5.Planes = 1;
	m_DibHeaderV5.Height = Height;
	m_DibHeaderV5.Width = Width;
	m_DibHeaderV5.ImageSize = Width * Height * m_Channels;
	m_DibHeaderV5.ColorsinColorTable = 0;
	m_DibHeaderV5.ImportantColorCount = 0;
	m_DibHeaderV5.XPixelsPerMeter = 0;
	m_DibHeaderV5.YPixelsPerMeter = 0;

	m_PixelRowSize = Width * m_Channels + (4 - (Width * m_Channels) % 4) % 4;
	
	delete[] m_PixelData;
	m_PixelData = nullptr;

	try
	{
		m_PixelDataSize = m_PixelRowSize * Height;
		m_PixelData = new uint8_t[m_PixelDataSize];
		memcpy_s(m_PixelData, m_PixelDataSize, PixelData, m_PixelDataSize);
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
		system("pause");
	}
}

void BMPImporter::WriteBmpFromBuffer(const char* fileName, uint8_t* PixelData, int Width, int Height, int Bpp)
{
	m_TempBuffer = true;
	m_Channels = Bpp / 8;

	if (m_Channels < 3)
	{
		cout << "24Bit or 32 Bit Bmps only..." << endl;
		system("pause");
		return;
	}

	m_BMPHeader.BmpTag = 'MB';
	m_BMPHeader.Offset = 54;
	m_BMPHeader.Reserved = 0;
	m_BMPHeader.Size = Width * Height * m_Channels + 54;

	m_DibHeaderV5.BitsperPixel = Bpp;
	m_DibHeaderV5.Compression = BI_RGB;
	m_DibHeaderV5.DIBHeaderSize = 40;
	m_DibHeaderV5.Planes = 1;
	m_DibHeaderV5.Height = Height;
	m_DibHeaderV5.Width = Width;
	m_DibHeaderV5.ImageSize = Width * Height * m_Channels;
	m_DibHeaderV5.ColorsinColorTable = 0;
	m_DibHeaderV5.ImportantColorCount = 0;
	m_DibHeaderV5.XPixelsPerMeter = 0;
	m_DibHeaderV5.YPixelsPerMeter = 0;

	m_PixelRowSize = Width * m_Channels + (4 - (Width * m_Channels) % 4) % 4;

	//delete[] m_PixelData;
	//m_PixelData = nullptr;

	try
	{
		m_PixelDataSize = m_PixelRowSize * Height;
		m_PixelData = PixelData;
		Write(fileName);
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
		system("pause");
	}
	m_PixelData = nullptr;
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
		int OldWidth = m_DibHeaderV5.Width;
		int OldHeight = m_DibHeaderV5.Height;

		int NewWidth = Width;
		int NewHeight = Height;

		float Rx = float(NewWidth) / float(OldWidth);
		float Ry = float(NewHeight) / float(OldHeight);

		if (Rx >= 4.0001f || Ry >= 4.0001f)
			return;

		int NewPixelRowSize = (((size_t)m_DibHeaderV5.BitsperPixel * abs(NewWidth) + 31) / 32) * 4;

		vector<uint8_t> NewPixelData(NewPixelRowSize * abs(NewHeight), 0);
		for (size_t h = 0; h < OldHeight - 1; h++)
		{
			for (size_t w = 0; w < OldWidth - 1; w++)
			{
				int NewPixelX = floor(w * Rx);
				int NewPixelY = floor(h * Ry);

				BGRA X0 = BilinearInterp(Rx, Ry, w, h);
				X0.Scale(4.0f);

				int PixelIndex = NewPixelRowSize * NewPixelY + NewPixelX * m_Channels;
				if (m_Channels >= 3)
				{
					NewPixelData[PixelIndex + 0] = X0.Blue;
					NewPixelData[PixelIndex + 1] = X0.Green;
					NewPixelData[PixelIndex + 2] = X0.Red;
					if (m_Channels == 4)
						NewPixelData[PixelIndex + 3] = X0.Alpha;
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
							int PixelIndex = NewPixelRowSize * NewPixelY + (NewPixelX + d) * m_Channels;
							if (m_Channels >= 3)
							{
								NewPixelData[PixelIndex + 0] = X2.Blue;
								NewPixelData[PixelIndex + 1] = X2.Green;
								NewPixelData[PixelIndex + 2] = X2.Red;
								if (m_Channels == 4)
									NewPixelData[PixelIndex + 3] = X2.Alpha;
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
							int PixelIndex = NewPixelRowSize * (NewPixelY + d) + NewPixelX * m_Channels;
							if (m_Channels >= 3)
							{
								NewPixelData[PixelIndex + 0] = Y2.Blue;
								NewPixelData[PixelIndex + 1] = Y2.Green;
								NewPixelData[PixelIndex + 2] = Y2.Red;
								if (m_Channels == 4)
									NewPixelData[PixelIndex + 3] = Y2.Alpha;
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

							int PixelIndex = NewPixelRowSize * (NewPixelY + c0) + (NewPixelX + c1) * m_Channels;
							if (m_Channels >= 3)
							{
								NewPixelData[PixelIndex + 0] = X3.Blue;
								NewPixelData[PixelIndex + 1] = X3.Green;
								NewPixelData[PixelIndex + 2] = X3.Red;
								if (m_Channels == 4)
									NewPixelData[PixelIndex + 3] = X3.Alpha;
							}
						}
					}
				}
				else if (Rx < 1.0f && Ry < 1.0f)
				{
					BGRA X0 = BilinearInterp(Rx, Ry, w, h);
					X0.Scale(4.0f);

					int PixelIndex = NewPixelRowSize * NewPixelY + NewPixelX * m_Channels;
					if (m_Channels >= 3)
					{
						NewPixelData[PixelIndex + 0] = X0.Blue;
						NewPixelData[PixelIndex + 1] = X0.Green;
						NewPixelData[PixelIndex + 2] = X0.Red;
						if (m_Channels == 4)
							NewPixelData[PixelIndex + 3] = X0.Alpha;
					}
				}

			}
		}

		m_DibHeaderV5.Width = NewWidth;
		m_DibHeaderV5.Height = NewHeight;
		m_DibHeaderV5.ImageSize = NewWidth * NewHeight * m_Channels;
		m_BMPHeader.Size = m_DibHeaderV5.ImageSize + 54;
		m_PixelRowSize = (((size_t)m_DibHeaderV5.BitsperPixel * abs(m_DibHeaderV5.Width) + 31) / 32) * 4;

		try
		{
			delete[] m_PixelData;

			m_PixelDataSize = NewPixelData.size();
			m_PixelData = new uint8_t[m_PixelDataSize];
			memcpy_s(m_PixelData, m_PixelDataSize, NewPixelData.data(), m_PixelDataSize);
		}
		catch (const std::exception& e)
		{
			cout << e.what() << endl;
			system("pause");
		}
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

	infile.read(reinterpret_cast<char*>(&m_BMPHeader.BmpTag), 2);
	infile.read(reinterpret_cast<char*>(&m_BMPHeader.Size), 3 * sizeof(int));

	if (m_BMPHeader.BmpTag != ('MB'))
	{
		cout << "Incorrect Header" << m_BMPHeader.BmpTag << endl;
		return false;
	}

	cout << "Reading BitmapFileHeader" << endl;
	cout << "Header\t\t: " << m_BMPHeader.BmpTag << endl;
	cout << "Size\t\t: " << m_BMPHeader.Size << endl;
	cout << "Reserved\t: " << m_BMPHeader.Reserved << endl;
	cout << "Offset\t\t: " << m_BMPHeader.Offset << endl;

	cout << "Reading DIBHeader" << endl;

	infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.DIBHeaderSize), sizeof(unsigned int));
	infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.Width), sizeof(int));
	infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.Height), sizeof(int));
	infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.Planes), sizeof(short));
	infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.BitsperPixel), sizeof(short));

	if (m_DibHeaderV5.BitsperPixel <= 32)
	{
		infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.Compression), sizeof(unsigned int));
		infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.ImageSize), sizeof(unsigned int));
		infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.XPixelsPerMeter), sizeof(int));
		infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.YPixelsPerMeter), sizeof(int));
		infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.ColorsinColorTable), sizeof(unsigned int));
		infile.read(reinterpret_cast<char*>(&m_DibHeaderV5.ImportantColorCount), sizeof(unsigned int));

		if (m_DibHeaderV5.BitsperPixel == 32)
		{
			infile.read(reinterpret_cast<char*>(&m_BGRAHeader), sizeof(BMPColorHeader32));
		}

		if (m_DibHeaderV5.BitsperPixel <= 8)
		{
			if (m_DibHeaderV5.ColorsinColorTable == 0)
			{
				m_DibHeaderV5.ColorsinColorTable = 1 << 8;
			}
			
			if (m_DibHeaderV5.DIBHeaderSize == 40)
			{
				infile.read(reinterpret_cast<char*>(m_ColorTable), sizeof(char) * m_DibHeaderV5.ColorsinColorTable);
			}
		}
	}

	cout << "DIBHeaderSize--------: " << m_DibHeaderV5.DIBHeaderSize << endl;
	cout << "Width----------------: " << m_DibHeaderV5.Width << endl;
	cout << "Height---------------: " << m_DibHeaderV5.Height << endl;
	cout << "ColorPlanes----------: " << m_DibHeaderV5.Planes << endl;
	cout << "BitsPerPixel---------: " << m_DibHeaderV5.BitsperPixel << endl;
	cout << "Compression----------: " << m_DibHeaderV5.Compression << endl;
	cout << "ImageSize------------: " << m_DibHeaderV5.ImageSize << endl;
	cout << "HorizontalResolution-: " << m_DibHeaderV5.XPixelsPerMeter << endl;
	cout << "VerticleResolution---: " << m_DibHeaderV5.YPixelsPerMeter << endl;
	cout << "NumColorInPalette----: " << m_DibHeaderV5.ColorsinColorTable << endl;
	cout << "ImportantColorsUsed--: " << m_DibHeaderV5.ImportantColorCount << endl;

	m_Channels = m_DibHeaderV5.BitsperPixel / 8;

	cout << "RedMask---------: " << m_BGRAHeader.RedMask << endl;
	cout << "GreenMask-------: " << m_BGRAHeader.GreenMask << endl;
	cout << "BlueMask--------: " << m_BGRAHeader.BlueMask << endl;
	cout << "AlphaMask-------: " << m_BGRAHeader.AlphaMask << endl;
	cout << "ColorSpaceType--: " << m_BGRAHeader.ColorSpaceType << endl;
	cout << "Unused[0]-------: " << m_BGRAHeader.Unused[0]	<< endl;

	infile.seekg(m_BMPHeader.Offset, ios_base::beg);

	m_PixelRowSize = (((size_t)m_DibHeaderV5.BitsperPixel * abs(m_DibHeaderV5.Width) + 31) / 32) * 4;

	m_PixelDataSize = m_PixelRowSize * abs(m_DibHeaderV5.Height);

	try
	{
		m_PixelData = new uint8_t[m_PixelDataSize];
		infile.read(reinterpret_cast<char*>(m_PixelData), m_PixelDataSize);

		if (m_DibHeaderV5.BitsperPixel == 8)
		{
			for (int h = 0; h < m_DibHeaderV5.Height; h++)
			{
				for (int w = 0; w < m_DibHeaderV5.Width; w++)
				{
					int Index = h * m_DibHeaderV5.Width + w;
					//m_PixelData[Index] = m_ColorTable[m_PixelData[Index]];
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		cout << e.what() << endl;
		system("pause");
	}

	infile.close();

	return true;
}

void BMPImporter::Write(const char* fileName)
{
	//FILE* outFile = nullptr;
	//errno_t err = fopen_s(&outFile, fileName, "wb");
	//if (!err)
	//{
	//	fwrite(&m_BMPHeader, sizeof(BitmapFileHeader), 1, outFile);
	//	fwrite(&m_DibHeaderV5, sizeof(DIBHeader), 1, outFile);
	//	if (m_DibHeaderV5.BitsPerPixel == 32)
	//		fwrite(&m_BGRAHeader, sizeof(BMPColorHeader32), 1, outFile);
	//	fwrite(m_PixelData, m_PixelDataSize, 1, outFile);
	//}
	//fclose(outFile);
	
	ofstream outFile(fileName, ios_base::binary|ios_base::out);
	
	if (outFile.good())
	{
		outFile.flush();
		outFile.write(reinterpret_cast<char*>(&m_BMPHeader.BmpTag), sizeof(unsigned short));
		outFile.write(reinterpret_cast<char*>(&m_BMPHeader.Size), 3 * sizeof(unsigned int));
		outFile.write(reinterpret_cast<char*>(&m_DibHeaderV5), sizeof(DIBHeader));
		if (m_DibHeaderV5.BitsperPixel == 32)
			outFile.write(reinterpret_cast<char*>(&m_BGRAHeader), sizeof(BMPColorHeader32));
		outFile.write(reinterpret_cast<char*>(m_PixelData), m_PixelDataSize);
	}
	else
	{
		cout << "Unable To open File to Write..." << endl;
	}
	
	outFile.close();
}

bool BMPImporter::WriteBmp8Bit(const char* FileName)
{
	int OldBpp = m_DibHeaderV5.BitsperPixel;

	m_DibHeaderV5.ColorsinColorTable = 0;
	m_DibHeaderV5.ImageSize = m_DibHeaderV5.Width * m_DibHeaderV5.Height;
	m_DibHeaderV5.Planes = 1;
	m_DibHeaderV5.BitsperPixel = 8;
	m_DibHeaderV5.ImportantColorCount = 1 << m_DibHeaderV5.BitsperPixel;

	uint8_t Zero = 0;
	int32_t Padding = (4 - (m_DibHeaderV5.Width) % 4) % 4;

	m_BMPHeader.BmpTag = 'MB';
	m_BMPHeader.Size = (m_DibHeaderV5.Width + Padding) * m_DibHeaderV5.Height + m_DibHeaderV5.DIBHeaderSize + sizeof(BITMAPFILEHEADER);
	m_BMPHeader.Reserved = 0;
	m_BMPHeader.Offset = m_DibHeaderV5.DIBHeaderSize + sizeof(BITMAPFILEHEADER) + m_DibHeaderV5.ImportantColorCount * sizeof(RGBQUAD);

	ofstream outFile(FileName, ios::binary);
	if (outFile.good())
	{
		outFile.write(reinterpret_cast<char*>(&m_BMPHeader.BmpTag), sizeof(unsigned short));
		outFile.write(reinterpret_cast<char*>(&m_BMPHeader.Size), 3 * sizeof(unsigned int));
		outFile.write(reinterpret_cast<char*>(&m_DibHeaderV5), sizeof(DIBHeader));

		//Indices for Gray Level
		for (int32_t p = 0; p < m_DibHeaderV5.ImportantColorCount; p++)
		{
			outFile.write(reinterpret_cast<char*>(&p), 1);
			outFile.write(reinterpret_cast<char*>(&p), 1);
			outFile.write(reinterpret_cast<char*>(&p), 1);
			outFile.write(reinterpret_cast<char*>(&Zero), 1);
		}

		if (OldBpp >= 24)
		{
			int k1 = 0; 
			int k2 = 0; 
			for (int32_t h = 0; h < m_DibHeaderV5.Height; h++)
			{
				for (int32_t w = 0; w < m_DibHeaderV5.Width; w++)
				{
					m_PixelData[k1] = (m_PixelData[k2 + 0] + m_PixelData[k2 + 1] + m_PixelData[k2 + 2]) / 3;
					
					k2 += 3;
					if (OldBpp == 32)
					{
						m_PixelData[k1] = (m_PixelData[k1] * 3 + m_PixelData[k2 + 3]) / 4;
						k2++;
					}

					outFile.write(reinterpret_cast<char*>(&m_PixelData[k1++]), 1);
				}

				for (int32_t p = 0; p < Padding; p++)
					outFile.write(reinterpret_cast<char*>(&Zero), 1);
			}

			//delete &m_PixelData[k1]; //Trim Array without new allocation?
		}
	}
	
	outFile.close();

	return true;
}

bool BMPImporter::WriteBmp8Bit(uint8_t* Buffer, const char* FileName, int Width, int Height)
{
	m_DibHeaderV5.DIBHeaderSize = sizeof(BITMAPINFOHEADER);
	m_DibHeaderV5.BitsperPixel = 8;
	m_DibHeaderV5.Width = Width;
	m_DibHeaderV5.Height = Height;
	m_DibHeaderV5.Compression = BI_RGB;
	m_DibHeaderV5.ColorsinColorTable = 0;
	m_DibHeaderV5.ImageSize = m_DibHeaderV5.Width * m_DibHeaderV5.Height;
	m_DibHeaderV5.Planes = 1;
	m_DibHeaderV5.ImportantColorCount = 1 << m_DibHeaderV5.BitsperPixel;

	uint8_t Zero = 0;
	int32_t Padding = (4 - (m_DibHeaderV5.Width) % 4) % 4;
	
	m_BMPHeader.BmpTag = 'MB';
	m_BMPHeader.Size = (m_DibHeaderV5.Width + Padding) * m_DibHeaderV5.Height + m_DibHeaderV5.DIBHeaderSize + sizeof(BITMAPFILEHEADER);
	m_BMPHeader.Reserved = 0;
	m_BMPHeader.Offset = m_DibHeaderV5.DIBHeaderSize + sizeof(BITMAPFILEHEADER) + m_DibHeaderV5.ImportantColorCount * sizeof(RGBQUAD);

	ofstream outFile(FileName, ios::binary);
	if (outFile.good())
	{
		outFile.write(reinterpret_cast<char*>(&m_BMPHeader.BmpTag), sizeof(unsigned short));
		outFile.write(reinterpret_cast<char*>(&m_BMPHeader.Size), 3 * sizeof(unsigned int));
		outFile.write(reinterpret_cast<char*>(&m_DibHeaderV5), sizeof(DIBHeader));

		//Indices for Gray Level
		for (int32_t p = 0; p < m_DibHeaderV5.ImportantColorCount; p++)
		{
			outFile.write(reinterpret_cast<char*>(&p), 1);
			outFile.write(reinterpret_cast<char*>(&p), 1);
			outFile.write(reinterpret_cast<char*>(&p), 1);
			outFile.write(reinterpret_cast<char*>(&Zero), 1);
		}

		const uint8_t* pData = Buffer;
		for (int32_t h = 0; h < m_DibHeaderV5.Height; h++)
		{
			for (int32_t w = 0; w < m_DibHeaderV5.Width; w++)
			{
				uint8_t gray = *pData++ ? 255 : 0;
				outFile.write(reinterpret_cast<char*>(&gray), 1);
			}

			for (int32_t p = 0; p < Padding; p++)
				outFile.write(reinterpret_cast<char*>(&Zero), 1);

			pData += Padding;
		}
	}
	outFile.close();

	return true;
}

BMPImporter::~BMPImporter()
{
	if (!m_TempBuffer)
	{
		delete[] m_PixelData;
		m_PixelData = nullptr;
	}
}

int BMPImporter::GetWidth()
{
	return m_DibHeaderV5.Width;
}

int BMPImporter::GetHeight()
{
	return m_DibHeaderV5.Height;
}

int BMPImporter::GetBitPerPixel()
{
	return m_DibHeaderV5.BitsperPixel;
}

BGRA BMPImporter::GetPixel( int x, int y )
{
	BGRA Color;
	
	if (x >= 0 && x < GetWidth() && y >= 0 && y < GetHeight())
	{
		size_t channels = m_DibHeaderV5.BitsperPixel / 8;

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
	return m_PixelData;
}

vector<uint8_t> BMPImporter::GetPixelData32()
{
	vector<uint8_t> PixelData32(m_DibHeaderV5.Width * m_DibHeaderV5.Height*4, 0);
	
	int k1 = 0;
	int k2 = 0;
	for (size_t h = 0; h < m_DibHeaderV5.Height; h++)
	{
		for (size_t w = 0; w < m_DibHeaderV5.Width; w++)
		{
			PixelData32[k2 + 0] = m_PixelData[k1 + 0];
			PixelData32[k2 + 1] = m_PixelData[k1 + 1];
			PixelData32[k2 + 2] = m_PixelData[k1 + 2];

			if (m_Channels == 4)
			{
				PixelData32[k2 + 3] = m_PixelData[k1 + 3];
				k1 += 4;
			}
			else if (m_Channels == 3)
			{
				PixelData32[k2 + 3] = 0xFF;
				k1 += 3;
			}
				
			k2 += 4;
		}
	}

	return PixelData32;
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

DIBHeaderV5* BMPImporter::GetBitmapHeader()
{
	return &m_DibHeaderV5;
}
