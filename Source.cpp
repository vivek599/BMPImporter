#include "BMPImporter.h"



int main()
{
	BMPImporter importer("Untitled.bmp");
	importer.Write("out.bmp");
	RGB color = importer.GetPixel( 201, 255);


	for (uint32_t y = 30; y < 30 + 300; ++y) 
	{
		for (uint32_t x = 30; x < 30 + 300; ++x) 
		{
			importer.SetPixel(x, y, {255, 0, 0});
		}
	}

	importer.Write("out.bmp");

	//BMPImporter importer("Untitled32A.bmp");


	//importer.Write("out32A.bmp");

	system("pause");
	return 1;
}