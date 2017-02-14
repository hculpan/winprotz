#ifndef __MAINPAINT_H__
#define __MAINPAINT_H__

#include <windows.h>

extern int cxClient;
extern int cyClient;

extern HWND childHwnd;
extern HWND reportHwnd;
extern HWND startPushButtonHwnd;
extern HWND stopPushButtonHwnd;

#define CHILD_WND_WIDTH   700
#define CHILD_WND_HEIGHT  700

#define IDC_START_BUTTON    1
#define IDC_STOP_BUTTON     2

void paintMain(HDC hdc, HWND hwnd, PAINTSTRUCT *ps);

void onCreate(HWND hwnd, char *ChildWndClass, char *ReportWndClass);

#endif
