#include "BMPImporter.h"



int main()
{
	BMPImporter importer("Untitled.bmp");


	importer.Write("out.bmp");






	system("pause");
	return 1;
}