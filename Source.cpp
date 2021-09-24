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

BMPImporter importer("Untitled.bmp");

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

    CreateUserWindow(hInstance, importer.GetWidth(), importer.GetHeight(), nCmdShow);
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