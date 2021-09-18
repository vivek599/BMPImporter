#include "BMPImporter.h"



int main()
{
	BMPImporter importer("Untitled.bmp");
	importer.Write("out.bmp");
	BGR color = importer.GetPixel( 200, 500);

	importer.SetPixel( 511, 1, { 255, 255, 255 });


	importer.Write("out.bmp");

	//BMPImporter importer("Untitled32A.bmp");


	//importer.Write("out32A.bmp");

	system("pause");
	return 1;
}