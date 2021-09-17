#include "BMPImporter.h"

BMPImporter::BMPImporter(const char* fileName) : BMPHeader{}, DibHeader{}
{
	ifstream infile;

	infile.open(fileName, ios::binary);

	if (!infile.is_open())
	{
		cout << "Unable to open file : " << fileName << endl;
		return;
	} 

	infile.read(reinterpret_cast<char*>(&BMPHeader.Header), 2);
	infile.read(reinterpret_cast<char*>(&BMPHeader.Size), 3* sizeof(int));

	if (BMPHeader.Header[0] != 'B' || BMPHeader.Header[1] != 'M')
	{
		cout << "Incorrect Header" << BMPHeader.Header[0] << BMPHeader.Header[1] << endl;
		return;
	}

	cout << "Reading BitmapFileHeader" << endl;
	cout << "Header\t\t: " << BMPHeader.Header[0] << BMPHeader.Header[1] << endl;
	cout << "Size\t\t: " << BMPHeader.Size << endl;
	cout << "Reserved\t: " << BMPHeader.Reserved << endl;
	cout << "Offset\t\t: " << BMPHeader.Offset << endl;

	cout << "Reading DIBHeader" << endl;
	infile.read(reinterpret_cast<char*>(&DibHeader), sizeof(DIBHeader));

	cout << "DIBHeaderSize--------: " << DibHeader.DIBHeaderSize << endl;
	cout << "Width----------------: " << DibHeader.Width << endl;
	cout << "Height---------------: " << DibHeader.Height << endl;
	cout << "ColorPlanes----------: " << DibHeader.ColorPlanes << endl;
	cout << "BitsPerPixel---------: " << DibHeader.BitsPerPixel << endl;
	cout << "Compression----------: " << DibHeader.Compression << endl;
	cout << "ImageSize------------: " << DibHeader.ImageSize << endl;
	cout << "HorizontalResolution-: " << DibHeader.HorizontalResolution << endl;
	cout << "VerticleResolution---: " << DibHeader.VerticleResolution << endl;
	cout << "NumColorInPalette----: " << DibHeader.NumColorInPalette << endl;
	cout << "ImportantColorsUsed--: " << DibHeader.ImportantColorsUsed << endl;

	if (DibHeader.BitsPerPixel != 24)
	{
		cout << "Incorrect BitsPerPixel" << DibHeader.BitsPerPixel << endl;
		return;
	}

	infile.seekg(BMPHeader.Offset, ios::beg);

	size_t pixelRowSize = (( (size_t)DibHeader.BitsPerPixel * abs(DibHeader.Width) + 31) / 32) * 4;

	PixelData.resize( pixelRowSize * abs(DibHeader.Height) );
	infile.read(reinterpret_cast<char*>(PixelData.data()), PixelData.size());
 	infile.close();

	ofstream outFile("out.bmp", ios::binary);
	outFile.write( BMPHeader.Header, 2);
	outFile.write( reinterpret_cast<char*>(&BMPHeader.Size), 3 * sizeof(int));
	outFile.write( reinterpret_cast<char*>(&DibHeader), sizeof(DIBHeader));
	outFile.write( reinterpret_cast<char*>(PixelData.data()), PixelData.size());

	outFile.close();

}

BMPImporter::~BMPImporter()
{
}
