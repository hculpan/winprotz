#include "mainpaint.h"

#include <string.h>

int cxClient = 0;
int cyClient = 0;

HWND childHwnd = NULL;
HWND startPushButtonHwnd;

TEXTMETRIC tm;

void onCreate(HWND hwnd, char *ChildWndClass) {
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
}

void paintMain(HDC hdc, HWND hwnd, PAINTSTRUCT *ps) {
}
