#include "boardpaint.h"
#include "mainpaint.h"

HBITMAP backbuffer = NULL;

void paintBoard(HDC hdc, HWND hwnd, PAINTSTRUCT *ps) {
  HDC backbuffDC;

  if (backbuffer) {
    backbuffDC = CreateCompatibleDC(hdc);

    SelectObject( backbuffDC, backbuffer );
    BitBlt(hdc, 0, 0, CHILD_WND_WIDTH, CHILD_WND_HEIGHT, backbuffDC, 0, 0, SRCCOPY);

    DeleteDC(backbuffDC);
  }
}

void paintInitialBacteria(HDC hdc, int count) {
  int i, x, y;
  RECT rect;
  HDC backbuffDC;

  if (!backbuffer) {
    DeleteObject(backbuffer);
  }

  backbuffDC = CreateCompatibleDC(hdc);

  backbuffer = CreateCompatibleBitmap( hdc, CHILD_WND_WIDTH, CHILD_WND_HEIGHT);
  rect.left = 0;
  rect.right = CHILD_WND_WIDTH;
  rect.top = 0;
  rect.bottom = CHILD_WND_HEIGHT;

  SelectObject( backbuffDC, backbuffer );
  for (i = 0; i < count; i++) {
    x = rand()%CHILD_WND_WIDTH;
    y = rand()%CHILD_WND_HEIGHT;
    SetPixel(backbuffDC, x, y, RGB(0, 255, 0));
  }

  DeleteDC(backbuffDC);
}
