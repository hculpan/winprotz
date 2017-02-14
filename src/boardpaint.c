#include "boardpaint.h"
#include "mainpaint.h"

#include <stdlib.h>
#include <process.h>
#include <time.h>
#include <stdio.h>

HBITMAP backbuffer = NULL;
HDC backbuffDC = NULL;

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

void updateBoard(HDC hdc) {
  if (backbuffer) {
    SelectObject(backbuffDC, backbuffer);
    BitBlt(hdc, 0, 0, CHILD_WND_WIDTH, CHILD_WND_HEIGHT, backbuffDC, 0, 0, SRCCOPY);
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

VOID AddRandomBacteria() {
  int x, y;

  while (TRUE) {
    x = rand()%CHILD_WND_WIDTH;
    y = rand()%CHILD_WND_HEIGHT;
    if (GetPixel(backbuffDC, x, y) != bacteriaColor) {
      SetPixel(backbuffDC, x, y, bacteriaColor);
      break;
    }
  }
}

VOID paintInitialBacteria(HDC hdc, int count) {
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

  SelectObject(backbuffDC, backbuffer);
  for (i = 0; i < count; i++) {
    AddRandomBacteria();
  }
}

VOID removeBacteria(int x, int y, struct Bug *bug) {
  bug->health += 40;
  if (bug->health > 1500) {
    bug->health = 1500;
  }
}

VOID bugsEatBacteria() {
  int x, y;
  struct Bug *bug = firstBug;

  while (bug) {
    x = bug->x;
    y = bug->y;
    if (GetPixel(backbuffDC, x - 1, y - 1) == bacteriaColor) removeBacteria(x - 1, y - 1, bug);
    if (GetPixel(backbuffDC, x    , y - 1) == bacteriaColor) removeBacteria(x    , y - 1, bug);
    if (GetPixel(backbuffDC, x + 1, y - 1) == bacteriaColor) removeBacteria(x + 1, y - 1, bug);
    if (GetPixel(backbuffDC, x - 1, y    ) == bacteriaColor) removeBacteria(x - 1, y    , bug);
    if (GetPixel(backbuffDC, x    , y    ) == bacteriaColor) removeBacteria(x    , y    , bug);
    if (GetPixel(backbuffDC, x + 1, y    ) == bacteriaColor) removeBacteria(x + 1, y    , bug);
    if (GetPixel(backbuffDC, x - 1, y + 1) == bacteriaColor) removeBacteria(x - 1, y + 1, bug);
    if (GetPixel(backbuffDC, x    , y + 1) == bacteriaColor) removeBacteria(x    , y + 1, bug);
    if (GetPixel(backbuffDC, x + 1, y + 1) == bacteriaColor) removeBacteria(x + 1, y + 1, bug);

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

VOID eraseBugs() {
  struct Bug *bug = firstBug;
  RECT rect;
  HBRUSH brush = (HBRUSH)GetStockObject(BLACK_BRUSH);

  while (bug) {
    rect.left = bug->x - 1;
    rect.top = bug->y - 1;
    rect.right = bug->x + 2;
    rect.bottom = bug->y + 2;
    FillRect(backbuffDC, &rect, brush);

    bug = bug->next;
  }
}

VOID drawBugs() {
  struct Bug *bug = firstBug;
  RECT rect;
  HBRUSH brush = CreateSolidBrush(RGB(100, 100, 255));

  while (bug) {
    rect.left = bug->x - 1;
    rect.top = bug->y - 1;
    rect.right = bug->x + 2;
    rect.bottom = bug->y + 2;
    FillRect(backbuffDC, &rect, brush);

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

VOID insertAfter(struct Bug *bug, struct Bug *newBug) {
  if (!newBug) return;

  newBug->next = NULL;
  newBug->prev = NULL;

  if (!bug) {
    firstBug = newBug;
  } else {
    if (bug->next) bug->next->prev = newBug;
    newBug->next = bug->next;
    newBug->prev = bug;
    bug->next = newBug;
  }
}

struct Bug * deleteBug(struct Bug *bug) {
  struct Bug *result;

  if (!bug) return NULL;

  if (bug->prev) bug->prev->next = bug->next;
  if (bug->next) bug->next->prev = bug->prev;

  if (bug == firstBug) {
    firstBug = bug->next;
    if (firstBug) firstBug->prev = NULL;
  }

  result = bug->next;

  free(bug);

  return result;
}

VOID doCycleStuff() {
  int i, gnum;
  struct Bug *bug = firstBug, *newBug, *nextBug;

  bugCount = 0;
  while (bug) {
    bug->age += 1;
    bug->health -= 1;

    if (bug->health <= 0) {
      bug = deleteBug(bug);
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

      bug->health = bug->health / 2;
      bug->age = 0;
      gnum = rand()%6;
      bug->gene[gnum] = (bug->gene[gnum] + 1) % 6;
      gnum = rand()%6;
      bug->gene[gnum] = ((bug->gene[gnum] - 1) + 6) % 6;
      bug->geneTotal = bug->gene[0] + bug->gene[1] + bug->gene[2] + bug->gene[3] + bug->gene[4] + bug->gene[5];

      insertAfter(bug, newBug);

      bugCount += 2;
    } else {
      bug = bug->next;
      bugCount += 1;
    }
  }

  if (cycle % 10 == 0) {
    InvalidateRect(reportHwnd, NULL, TRUE);
  }
}

VOID createInitialBugs() {
  int i, x, y;
  struct Bug *newBug, *lastBug = NULL;
  newBug = NULL;

  for (i = 0; i < startingBugCount; i++) {
    newBug = malloc(sizeof(struct Bug));
    bugCount += 1;

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

    insertAfter(lastBug, newBug);
    lastBug = newBug;
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

VOID processBugs() {
  eraseBugs();
  doCycleStuff();
  selectNewDirection();
  moveBugs();
  bugsEatBacteria();
  drawBugs();
}

VOID Thread(PVOID pvoid) {
  HDC hdc = GetDC(childHwnd);
  struct Bug *bug, *nextBug;

  //srand(time(NULL))0;
  srand(1);

  backbuffDC = CreateCompatibleDC(hdc);
  createInitialBugs();
  paintInitialBacteria(hdc, 5000);
  processBugs();
  updateBoard(hdc);
  InvalidateRect(reportHwnd, NULL, TRUE);

  while (ThreadRun) {
//    Sleep(50);
    cycle += 1;
    AddRandomBacteria();
    AddRandomBacteria();
    AddRandomBacteria();
    processBugs();
    if (bugCount <= 0) {
      printf("BugCount 0!\n");
      break;
    }
    updateBoard(hdc);
    if (cycle % 100 == 0) {
      printf("Hit cycle %d\n", cycle);
    }
  }

  printf("Done with loop\n");

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
