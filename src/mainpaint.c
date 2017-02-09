#include "mainpaint.h"

#include <string.h>

int cxClient = 0;
int cyClient = 0;

HWND childHwnd;
HWND startPushButtonHwnd;

TEXTMETRIC tm;

void onCreate(HWND hwnd, char *ChildWndClass) {
//  HDC hdc = GetDC(hwnd);

  childHwnd = CreateWindow(
    ChildWndClass,
    NULL,
    WS_CHILDWINDOW | WS_VISIBLE,
    5, 5,
    CHILD_WND_WIDTH, CHILD_WND_HEIGHT,
    hwnd,
    NULL,
    (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
    NULL
  );

  startPushButtonHwnd = CreateWindow("button", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    CHILD_WND_WIDTH + 5 + 25, 15, 50, 26, hwnd, (HMENU)1, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  InvalidateRect(childHwnd, NULL, TRUE);
//  ReleaseDC(hwnd, hdc);
}

void paint(HDC hdc, HWND hwnd, PAINTSTRUCT *ps) {
/*  RECT        rect;
  char        buff[100];
  int         len;

  HDC displayHdc;

  GetClientRect(hwnd, &rect);
  //DrawText(hdc, "Hello, Windows!", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  len = wsprintf(buff, "Client area: %dx%d", rect.right, rect.bottom);
  TextOut(hdc, cxClient - ((len + 2) * tm.tmAveCharWidth), cyClient - (10 + tm.tmHeight + tm.tmExternalLeading), buff, len);*/
}
