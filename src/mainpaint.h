#ifndef __MAINPAINT_H__
#define __MAINPAINT_H__

#include <windows.h>

extern int cxClient;
extern int cyClient;

extern HWND childHwnd;
extern HWND startPushButtonHwnd;

#define CHILD_WND_WIDTH   800
#define CHILD_WND_HEIGHT  700

void paintMain(HDC hdc, HWND hwnd, PAINTSTRUCT *ps);

void onCreate(HWND hwnd, char *ChildWndClass);

#endif
