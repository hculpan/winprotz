#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "mainpaint.h"
#include "boardpaint.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildWndProc(HWND, UINT, WPARAM, LPARAM);

static char szAppName[]   = "WinProtz";
static char szChildName[] = "WinProtzChild";

FILE *out;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
    HWND        hwnd;
    MSG         msg;
    WNDCLASSEX  wndclass;

    srand( (unsigned)time( NULL ) );

    if(AllocConsole()) {
      out = freopen("CONOUT$", "w", stdout);
    }

    wndclass.cbSize         = sizeof(wndclass);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = szAppName;
    wndclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wndclass);

    wndclass.lpfnWndProc    = ChildWndProc;
    wndclass.hIconSm        = NULL;
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszClassName  = szChildName;
    wndclass.hIconSm        = NULL;

    RegisterClassEx(&wndclass);

    hwnd = CreateWindow(szAppName,
                        "Win Protozoa",
                        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        1224,
                        738,
                        NULL,
                        NULL,
                        hInstance,
                        NULL );

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    fclose(out);

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    HDC         hdc;
    PAINTSTRUCT ps;

    switch (iMsg) {
        case WM_CREATE:
            onCreate(hwnd, szChildName);
            return 0;

        case WM_SIZE:
            cxClient = LOWORD(lParam);
            cyClient = HIWORD(lParam);
            return 0;

        case WM_PAINT:
            printf("Got WM_PAINT in main\n");
            hdc = BeginPaint(hwnd, &ps);
            paintMain(hdc, hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;

        case WM_COMMAND:
            printf("Got WM_COMMAND in main\n");
            hdc = GetDC(childHwnd);
            paintInitialBacteria(hdc, 5000);
            ReleaseDC(childHwnd, hdc);
            InvalidateRect(childHwnd, NULL, FALSE);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
  HDC         hdc;
  PAINTSTRUCT ps;

  switch (iMsg) {
      case WM_PAINT:
          printf("Got WM_PAINT in child\n");
          hdc = BeginPaint(hwnd, &ps);
          paintBoard(hdc, hwnd, &ps);
          EndPaint(hwnd, &ps);
          return 0;

//      case WM_ERASEBKGND:
//          return 1;

  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
