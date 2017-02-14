#include "mainpaint.h"

#include <string.h>

int cxClient = 0;
int cyClient = 0;

HWND childHwnd = NULL;
HWND reportHwnd = NULL;
HWND startPushButtonHwnd;
HWND stopPushButtonHwnd;

TEXTMETRIC tm;

void onCreate(HWND hwnd, char *ChildWndClass, char *ReportWndClass) {
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

  reportHwnd = CreateWindow(
    ReportWndClass,
    NULL,
    WS_CHILDWINDOW | WS_VISIBLE,
    CHILD_WND_WIDTH + 10, (CHILD_WND_HEIGHT + 5) - 100,
    305, 100,
    hwnd,
    NULL,
    (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
    NULL
  );

  startPushButtonHwnd = CreateWindow("button", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    CHILD_WND_WIDTH + 5 + 25, 15, 50, 26, hwnd, (HMENU)IDC_START_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  stopPushButtonHwnd = CreateWindow("button", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    CHILD_WND_WIDTH + 5 + 25 + 50 + 10, 15, 50, 26, hwnd, (HMENU)IDC_STOP_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
  EnableWindow(stopPushButtonHwnd, FALSE);

  InvalidateRect(childHwnd, NULL, TRUE);
  InvalidateRect(reportHwnd, NULL, TRUE);
}

void paintMain(HDC hdc, HWND hwnd, PAINTSTRUCT *ps) {
}
