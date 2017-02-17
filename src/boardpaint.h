#ifndef __BOARDPAINT_H__
#define __BOARDPAINT_H__

#include <windows.h>

void paintBoard(HDC hdc);

VOID Thread(PVOID pvoid);

VOID reportStuff(HWND hwnd, HDC hdc);

extern BOOL ThreadRun;

extern long cycle;

extern int bugCount;

typedef struct SimParams {
  int toroidal;
  int worldWidth;
  int worldHeight;
  int startingBacteria;
  int startingBugs;
  int reseedRate;
  int delay;
} SimParams;

#endif
