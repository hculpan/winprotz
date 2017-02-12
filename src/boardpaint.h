#ifndef __BOARDPAINT_H__
#define __BOARDPAINT_H__

#include <windows.h>

void paintInitialBacteria(HDC hdc, int count);

void paintBoard(HDC hdc, HWND hwnd, PAINTSTRUCT *ps);

#endif
