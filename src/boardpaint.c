#include "boardpaint.h"
#include "mainpaint.h"

#include <stdlib.h>
#include <process.h>
#include <time.h>
#include <stdio.h>

HBITMAP backbuffer = NULL;

COLORREF bacteriaColor = RGB(0, 255, 0);

BOOL ThreadRun = FALSE;

struct Bug {
  int x;
  int y;
  int dir;
  int gene[6];
  int health;
  int age;

  int geneTotal;

  struct Bug *next;
  struct Bug *prev;
};

struct Bug *firstBug;

int startingBugCount = 20;
int bugCount = 0;

long cycle;

void updateBoard(HDC hdc, HDC *backbuffDC) {
  int bitsperpixel;

  if (backbuffer) {
    bitsperpixel = GetDeviceCaps(*backbuffDC, BITSPIXEL);

    SelectObject(*backbuffDC, backbuffer);
    BitBlt(hdc, 0, 0, CHILD_WND_WIDTH, CHILD_WND_HEIGHT, *backbuffDC, 0, 0, SRCCOPY);
  }
}

void paintBoard(HDC hdc) {
  HDC tempDC;

  if (ThreadRun || backbuffer == NULL) return;

  tempDC = CreateCompatibleDC(hdc);
  SelectObject(tempDC, backbuffer);
  BitBlt(hdc, 0, 0, CHILD_WND_WIDTH, CHILD_WND_HEIGHT, tempDC, 0, 0, SRCCOPY);
  DeleteDC(tempDC);
}

VOID AddRandomBacteria(HDC *hdc) {
  int x, y;

  while (TRUE) {
    x = rand()%CHILD_WND_WIDTH;
    y = rand()%CHILD_WND_HEIGHT;
    if (GetPixel(*hdc, x, y) != bacteriaColor) {
      SetPixel(*hdc, x, y, bacteriaColor);
      break;
    }
  }
}

VOID paintInitialBacteria(HDC hdc, HDC *backbuffDC, int count) {
  int i;
  RECT rect;

  if (!backbuffer) {
    DeleteObject(backbuffer);
  }

  backbuffer = CreateCompatibleBitmap(hdc, CHILD_WND_WIDTH, CHILD_WND_HEIGHT);
  rect.left = 0;
  rect.right = CHILD_WND_WIDTH;
  rect.top = 0;
  rect.bottom = CHILD_WND_HEIGHT;

  SelectObject(*backbuffDC, backbuffer);
  for (i = 0; i < count; i++) {
    AddRandomBacteria(backbuffDC);
  }
}

VOID createInitialBugs() {
  int i, x, y;
  struct Bug *newBug = NULL, *lastBug = NULL;

  for (i = 0; i < startingBugCount; i++) {
    newBug = malloc(sizeof(struct Bug));
    bugCount += 1;
    newBug->prev = lastBug;

    while (TRUE) {
      x = rand()%CHILD_WND_WIDTH;
      y = rand()%CHILD_WND_HEIGHT;
      if (x > 0 && x < CHILD_WND_WIDTH - 1 && y > 0 && y < CHILD_WND_HEIGHT - 1) {
        newBug->x = x;
        newBug->y = y;
        break;
      }
    }

    newBug->dir = rand()%6;
    newBug->gene[0] = rand()%6;
    newBug->gene[1] = rand()%6;
    newBug->gene[2] = rand()%6;
    newBug->gene[3] = rand()%6;
    newBug->gene[4] = rand()%6;
    newBug->gene[5] = rand()%6;
    newBug->health = 800;
    newBug->age = 0;
    newBug->geneTotal = newBug->gene[0] + newBug->gene[1] + newBug->gene[2] + newBug->gene[3] + newBug->gene[4] + newBug->gene[5];
    newBug->next = NULL;

    if (i == 0) {
      firstBug = newBug;
      lastBug = firstBug;
    } else {
      lastBug->next = newBug;
      lastBug = newBug;
    }
  }
}

VOID removeBacteria(int x, int y, struct Bug *bug, HDC *backbuffDC) {
//  SetPixel(*backbuffDC, x, y, RGB(0, 0, 0));
  bug->health += 40;
  if (bug->health > 1500) {
    bug->health = 1500;
  }
}

VOID bugsEatBacteria(HDC *backbuffDC) {
  int x, y;
  struct Bug *bug = firstBug;

  while (bug) {
    x = bug->x;
    y = bug->y;
    if (GetPixel(*backbuffDC, x - 1, y - 1) == bacteriaColor) removeBacteria(x - 1, y - 1, bug, backbuffDC);
    if (GetPixel(*backbuffDC, x    , y - 1) == bacteriaColor) removeBacteria(x    , y - 1, bug, backbuffDC);
    if (GetPixel(*backbuffDC, x + 1, y - 1) == bacteriaColor) removeBacteria(x + 1, y - 1, bug, backbuffDC);
    if (GetPixel(*backbuffDC, x - 1, y    ) == bacteriaColor) removeBacteria(x - 1, y    , bug, backbuffDC);
    if (GetPixel(*backbuffDC, x    , y    ) == bacteriaColor) removeBacteria(x    , y    , bug, backbuffDC);
    if (GetPixel(*backbuffDC, x + 1, y    ) == bacteriaColor) removeBacteria(x + 1, y    , bug, backbuffDC);
    if (GetPixel(*backbuffDC, x - 1, y + 1) == bacteriaColor) removeBacteria(x - 1, y + 1, bug, backbuffDC);
    if (GetPixel(*backbuffDC, x    , y + 1) == bacteriaColor) removeBacteria(x    , y + 1, bug, backbuffDC);
    if (GetPixel(*backbuffDC, x + 1, y + 1) == bacteriaColor) removeBacteria(x + 1, y + 1, bug, backbuffDC);

    bug = bug->next;
  }
}

VOID moveBugs() {
  struct Bug *bug = firstBug;

  while (bug) {
    switch (bug->dir) {
      case 0: bug->y += 2;              break;
      case 1: bug->x += 2; bug->y += 1; break;
      case 2: bug->x += 2; bug->y -= 1; break;
      case 3:              bug->y -= 2; break;
      case 4: bug->x -= 2; bug->y -= 1; break;
      case 5: bug->x -= 2; bug->y += 1; break;
    }

    // Bump walls
    if (bug->x <= 0) {
      bug->x = 1;
    }

    if (bug->x >= CHILD_WND_WIDTH - 1) {
      bug->x = CHILD_WND_WIDTH - 2;
    }

    if (bug->y <= 0) {
      bug->y = 1;
    }

    if (bug->y >= CHILD_WND_HEIGHT - 1) {
      bug->y = CHILD_WND_HEIGHT - 2;
    }

    bug = bug->next;
  }
}

VOID eraseBugs(HDC *backbuffDC) {
  struct Bug *bug = firstBug;
  RECT rect;
  HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);

  while (bug) {
    rect.left = bug->x - 1;
    rect.top = bug->y - 1;
    rect.right = bug->x + 2;
    rect.bottom = bug->y + 2;
    FillRect(*backbuffDC, &rect, brush);

    bug = bug->next;
  }
}

VOID drawBugs(HDC *backbuffDC) {
  struct Bug *bug = firstBug;
  RECT rect;
  HBRUSH brush = CreateSolidBrush(RGB(100, 100, 255));

  while (bug) {
    rect.left = bug->x - 1;
    rect.top = bug->y - 1;
    rect.right = bug->x + 2;
    rect.bottom = bug->y + 2;
    FillRect(*backbuffDC, &rect, brush);

    bug = bug->next;
  }
}

VOID selectNewDirection() {
  struct Bug *bug = firstBug;
  int i, num;

  while (bug) {
    num = rand()%bug->geneTotal;

    for (i = 0; i < 6; i++) {
      num -= bug->gene[i];
      if (num <= 0) {
        bug->dir = (bug->dir + i) % 6;
      }
    }

    bug = bug->next;
  }
}

VOID doCycleStuff() {
  int i, gnum;
  struct Bug *bug = firstBug, *newBug, *nextBug;

  bugCount = 0;
  cycle += 1;
  while (bug) {
    bug->age += 1;
    bug->health -= 1;

    if (bug->health <= 0) {
      nextBug = bug->next;
      if (bug->prev) bug->prev->next = bug->next;
      if (bug->next) bug->next->prev = bug->prev;
      free(bug);
      if (bug == firstBug) {
        firstBug = nextBug;
        if (firstBug) firstBug->prev = NULL;
      }
      bug = nextBug;
    } else if (bug->health >= 1000 && bug->age >= 800) {
      newBug = malloc(sizeof(struct Bug));
      newBug->health = bug->health / 2;
      newBug->age = 0;
      newBug->x = bug->x;
      newBug->y = bug->y;
      newBug->dir = (bug->dir + 3) % 6;
      for (i = 0; i < 6; i++) {
        newBug->gene[i] = bug->gene[i];
      }

      gnum = rand()%6;
      newBug->gene[gnum] = (newBug->gene[gnum] + 1) % 6;
      gnum = rand()%6;
      newBug->gene[gnum] = ((newBug->gene[gnum] - 1) + 6) % 6;
      newBug->geneTotal = newBug->gene[0] + newBug->gene[1] + newBug->gene[2] + newBug->gene[3] + newBug->gene[4] + newBug->gene[5];
      newBug->next = bug->next;
      newBug->prev = bug;

      bug->health = bug->health / 2;
      bug->age = 0;
      gnum = rand()%6;
      bug->gene[gnum] = (bug->gene[gnum] + 1) % 6;
      gnum = rand()%6;
      bug->gene[gnum] = ((bug->gene[gnum] - 1) + 6) % 6;
      bug->geneTotal = bug->gene[0] + bug->gene[1] + bug->gene[2] + bug->gene[3] + bug->gene[4] + bug->gene[5];
      bug->next = newBug;

      bugCount += 2;
      bug = newBug->next;
    } else {
      bug = bug->next;
      bugCount += 1;
    }
  }

  if (cycle % 10 == 0) {
    InvalidateRect(reportHwnd, NULL, TRUE);
  }
}

VOID reportStuff(HWND hwnd, HDC hdc) {
  char msg[100];
  int len;

  len = sprintf(msg, "Cycle: %d", cycle);
  TextOut(hdc, 5, 5, msg, len);

  len = sprintf(msg, "Bugs: %d", bugCount);
  TextOut(hdc, 5, 35, msg, len);
}

VOID processBugs(HDC *backbuffDC) {
  eraseBugs(backbuffDC);
  doCycleStuff();
  selectNewDirection();
  moveBugs();
  bugsEatBacteria(backbuffDC);
  drawBugs(backbuffDC);
}

VOID Thread(PVOID pvoid) {
  HDC hdc = GetDC(childHwnd);
  HDC backbuffDC;
  struct Bug *bug, *nextBug;

  //srand(time(NULL))0;
  srand(1);

  backbuffDC = CreateCompatibleDC(hdc);
  createInitialBugs();
  paintInitialBacteria(hdc, &backbuffDC, 5000);
  processBugs(&backbuffDC);
  updateBoard(hdc, &backbuffDC);
  InvalidateRect(reportHwnd, NULL, TRUE);

  while (ThreadRun) {
//    Sleep(50);
    AddRandomBacteria(&backbuffDC);
    AddRandomBacteria(&backbuffDC);
    AddRandomBacteria(&backbuffDC);
    processBugs(&backbuffDC);
    if (bugCount <= 0) {
      break;
    }
    updateBoard(hdc, &backbuffDC);
  }

  DeleteDC(backbuffDC);
  backbuffDC = NULL;
  ReleaseDC(childHwnd, hdc);

  bug = firstBug;
  while (bug) {
    nextBug = bug->next;
    free(bug);
    bug = nextBug;
  }

  ThreadRun = FALSE;
}
