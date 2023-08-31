#include <windows.h>
#include <Windowsx.h>

#include "BMPImporter.h"



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//int main()
//{
//	BMPImporter importer("Untitled.bmp");
//	importer.Write("out.bmp");
//	BGRA color = importer.GetPixel( 200, 500);
//
//	importer.SetPixel( 2, 1, { 255, 255, 255 });
//
//
//	importer.Write("out.bmp");
//
//	//BMPImporter importer("Untitled32A.bmp");
//
//
//	//importer.Write("out32A.bmp");
//
//	system("pause");
//	return 1;
//}

int main()
{
	BMPImporter Texture;

    vector<uint8_t> Pixels;

    int Width = 1024;
    int Height = 1024;
    int Channels = 3;
    int Bpp = Channels * 8;
    int CellSize = Width/16;

    Pixels.resize(Width * Height * Bpp, 0xFF);
    Texture.LoadBuffer(Pixels.data(), Width, Height, Bpp);

    BGRA Black;
    Black.Blue  = 0x00;
    Black.Green = 0x00;
    Black.Red   = 0x00;
    Black.Alpha = 0xFF;
    
    BGRA White;
    White.Blue  = 0xFF;
    White.Green = 0xFF;
    White.Red   = 0xFF;
    White.Alpha = 0xFF;

    BGRA Gray64;
    Gray64.Blue  = 0x40;
    Gray64.Green = 0x40;
    Gray64.Red   = 0x40;
    Gray64.Alpha = 0xFF;
    
    BGRA Gray192;
    Gray192.Blue  = 0x192;
    Gray192.Green = 0x192;
    Gray192.Red   = 0x192;
    Gray192.Alpha = 0xFF;
    
    BGRA Gray128;
    Gray128.Blue  = 0x80;
    Gray128.Green = 0x80;
    Gray128.Red   = 0x80;
    Gray128.Alpha = 0xFF;

    for (int h = 0; h < Height; h++)
    {
        for (int w = 0; w < Width; w++)
        {
            if (w == 1 || h == 1)
            {
                Texture.SetPixel(w, h, Black);
            }
            else if (w == Width - 1 || h == Height - 1)
            {
                Texture.SetPixel(w, h, Black);
            }
            else if (w == Width - 2 || h == Height - 2)
            {
                Texture.SetPixel(w, h, Black);
            }
            else if (w == (Width + 2) / 2 || h == (Height + 2) / 2)
            {
                Texture.SetPixel(w, h, Black);
            }
            else if (w == (Width - 2) / 2 || h == (Height - 2) / 2)
            {
                Texture.SetPixel(w, h, Black);
            }
            else if ((w % CellSize == 0) || (h % CellSize == 0))
            {
                Texture.SetPixel( w, h, Black);
            }
            else
            {
                Texture.SetPixel(w, h, Gray192);
            }
        }
    }

    Texture.Write("PlaceHolderTexture01.bmp");

	//system("pause");
	return 1;
}

#if 0
BMPImporter importer("Untitled32A.bmp");
//BMPImporter importer("Untitled.bmp");

bool CreateUserWindow(HINSTANCE hInstance, int Width, int Height, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Paint Bitmap";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        TEXT("Paint Bitmap"),             // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, Width, Height,
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{

    CreateUserWindow(hInstance, importer.GetWidth()+16, importer.GetHeight()+39, nCmdShow);
    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_MOUSEMOVE:
    {
        if (wParam & MK_LBUTTON)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            importer.SetPixel(xPos, importer.GetHeight() - yPos, { 255, 0, 0, 255 });
            InvalidateRect(GetActiveWindow(), nullptr, TRUE);
        }

    }
    return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        int result = SetDIBitsToDevice(hdc, 0, 0,
            importer.GetWidth(),
            importer.GetHeight(),
            0, 0, 0,
            importer.GetHeight(),
            importer.GetPixelData(),
            reinterpret_cast<BITMAPINFO*>(importer.GetBitmapHeader()),
            DIB_RGB_COLORS);

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif