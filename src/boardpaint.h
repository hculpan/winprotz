#ifndef __BOARDPAINT_H__
#define __BOARDPAINT_H__

#include <windows.h>

void paintBoard(HDC hdc);

VOID Thread(PVOID pvoid);

VOID reportStuff(HWND hwnd, HDC hdc);

VOID rereadSimParams();

extern BOOL ThreadRun;
extern BOOL ThreadPaused;

extern long cycle;

extern int bugCount;

enum GeneModel {
  dewdney,
  simplified_ga,
};

typedef struct SimParams {
  int toroidal;
  int worldWidth;
  int worldHeight;
  int startingBacteria;
  int startingBugs;
  int reseedRate;
  int delay;
  enum GeneModel geneModel;
} SimParams;

#endif
