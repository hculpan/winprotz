#include "boardpaint.h"
#include "mainpaint.h"

void paintInitialBacteria(HDC hdc, HWND hwnd, PAINTSTRUCT *ps, int count) {
  int i, x, y;

  for (i = 0; i < count; i++) {
    x = rand()%CHILD_WND_WIDTH;
    y = rand()%CHILD_WND_HEIGHT;
    SetPixel(hdc, x, y, RGB(0, 255, 0));
  }
}
