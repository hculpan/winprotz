#ifndef __BOARDPAINT_H__
#define __BOARDPAINT_H__

#include <windows.h>

void paintBoard(HDC hdc);

VOID Thread(PVOID pvoid);

VOID reportStuff(HWND hwnd, HDC hdc);

extern BOOL ThreadRun;

extern long cycle;

extern int bugCount;

#endif
