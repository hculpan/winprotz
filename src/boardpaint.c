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

FILE *outf;

struct Bug {
  int id;
  int x;
  int y;
  int dir;
  int gene[6];
  int health;
  int age;

  int parentId;

  int geneTotal;

  HBRUSH brush;

  struct Bug *next;
  struct Bug *prev;
};

HBRUSH bugBrush[4];

struct Bug *firstBug;
struct Bug *currentBug;

int startingBugCount = 20;
int bugCount = 0;
int lastBugId = 1;

HBRUSH backgroundBrush;

long cycle;

// forward declarations
VOID assignBrushToBug(struct Bug *bug);

VOID startBugLoop() {
  currentBug = firstBug;
}

struct Bug *nextBug() {
  struct Bug *result = currentBug;
  if (result) currentBug = result->next;
  return result;
}

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
  struct Bug *bug;

  startBugLoop();

  while (bug = nextBug()) {
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
  }
}

VOID writeOutBugInfo(struct Bug* bug) {
  if (!outf) return;

  fprintf(outf, "Bug %4d [parent: %4d, age: %8d, health: %4d, x: %3d, y: %3d]\n", bug->id, bug->parentId, bug->age, bug->health, bug->x, bug->y);
  if (bug->gene[0] == 0) {
    fprintf(outf, "  Genetics: %d_%d_%d_%d_%d_%d [%1.2f]\n",
      bug->gene[0], bug->gene[1], bug->gene[2], bug->gene[3], bug->gene[4], bug->gene[5],
      0.0
    );
  } else {
    fprintf(outf, "  Genetics: %d_%d_%d_%d_%d_%d [%1.2f]\n",
      bug->gene[0], bug->gene[1], bug->gene[2], bug->gene[3], bug->gene[4], bug->gene[5],
      (float)bug->gene[0]/(float)bug->geneTotal
    );
  }
}

struct Bug *createNewBug() {
  int x, y;
  struct Bug *result = malloc(sizeof(struct Bug));

  while (TRUE) {
    x = rand()%CHILD_WND_WIDTH;
    y = rand()%CHILD_WND_HEIGHT;
    if (x > 0 && x < CHILD_WND_WIDTH - 1 && y > 0 && y < CHILD_WND_HEIGHT - 1) {
      result->x = x;
      result->y = y;
      break;
    }
  }

  result->id = lastBugId++;
  result->parentId = 0;
  result->dir = rand()%6;
  result->gene[0] = rand()%6;
  result->gene[1] = rand()%6;
  result->gene[2] = rand()%6;
  result->gene[3] = rand()%6;
  result->gene[4] = rand()%6;
  result->gene[5] = rand()%6;
  result->health = 800;
  result->age = 0;
  result->geneTotal = result->gene[0] + result->gene[1] + result->gene[2] + result->gene[3] + result->gene[4] + result->gene[5];
  assignBrushToBug(result);

  return result;
}

VOID moveBugs() {
  struct Bug *bug;

  startBugLoop();

  while (bug = nextBug()) {
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
  }
}

VOID drawBugsOnBoard(HBRUSH brush) {
  RECT rect;
  struct Bug *bug;

  startBugLoop();

  while (bug = nextBug()) {
    rect.left = bug->x - 1;
    rect.top = bug->y - 1;
    rect.right = bug->x + 2;
    rect.bottom = bug->y + 2;
    if (brush) {
      FillRect(backbuffDC, &rect, brush);
    } else {
      FillRect(backbuffDC, &rect, bug->brush);
    }
  }
}

VOID eraseBugs() {
  drawBugsOnBoard(backgroundBrush);
}

VOID drawBugs() {
  drawBugsOnBoard(NULL);
}

VOID selectNewDirection() {
  int i, num;
  struct Bug *bug;

  startBugLoop();

  while (bug = nextBug()) {
    num = rand()%bug->geneTotal;

    for (i = 0; i < 6; i++) {
      num -= bug->gene[i];
      if (num <= 0) {
        bug->dir = (bug->dir + i) % 6;
      }
    }
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
  int i, gnum, parentId;
  struct Bug *bug, *newBug;

  startBugLoop();

  bugCount = 0;
  while (bug = nextBug()) {
    bug->age += 1;
    bug->health -= 1;

    if (bug->health <= 0) {
      if (outf) {
        fprintf(outf, "Bug %d has died at age %d, how sad!\n", bug->id, bug->age);
      }
      bug = deleteBug(bug);
    } else if (bug->health >= 1000 && bug->age >= 800) {
      parentId = bug->id;

      newBug = createNewBug();
      newBug->parentId = parentId;
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
      assignBrushToBug(newBug);
      writeOutBugInfo(newBug);

      bug->health = bug->health / 2;
      bug->id = lastBugId++;
      bug->parentId = parentId;
      bug->age = 0;
      gnum = rand()%6;
      bug->gene[gnum] = (bug->gene[gnum] + 1) % 6;
      gnum = rand()%6;
      bug->gene[gnum] = ((bug->gene[gnum] - 1) + 6) % 6;
      bug->geneTotal = bug->gene[0] + bug->gene[1] + bug->gene[2] + bug->gene[3] + bug->gene[4] + bug->gene[5];
      assignBrushToBug(bug);
      writeOutBugInfo(bug);

      insertAfter(bug, newBug);

      bugCount += 2;
    } else {
      bugCount += 1;
    }
  }
}

VOID createInitialBugs() {
  int i;
  struct Bug *newBug = NULL, *lastBug = NULL;

  for (i = 0; i < startingBugCount; i++) {
    printf("Creating bug\n");
    bugCount += 1;

    newBug = createNewBug();
    writeOutBugInfo(newBug);

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

  if (outf) {
    fprintf(outf, "Cycle: %8d, Bugs: %4d\n", cycle, bugCount);
  }
}

VOID processBugs() {
  eraseBugs();
  doCycleStuff();
  selectNewDirection();
  moveBugs();
  bugsEatBacteria();
  drawBugs();
}

VOID assignBrushToBug(struct Bug *bug) {
  float forward = (float)bug->gene[0]/(float)bug->geneTotal;

  if (forward < 0.26) {
    bug->brush = bugBrush[0];
  } else if (forward < 0.51) {
    bug->brush = bugBrush[1];
  } else if (forward < 0.76) {
    bug->brush = bugBrush[2];
  } else {
    bug->brush = bugBrush[3];
  }
}

FILE *openNewLog() {
  char fname[50];
  time_t rawtime;
  struct tm *td;

  time(&rawtime);
  td = localtime(&rawtime);

  sprintf(fname, "winprot-%04d%02d%02d%02d%02d%02d.log",
    td->tm_year + 1900, td->tm_mon, td->tm_mday, td->tm_hour, td->tm_min, td->tm_sec
  );
  free(td);


  return fopen(fname, "w");
}

VOID Thread(PVOID pvoid) {
  HDC hdc = GetDC(childHwnd);
  struct Bug *bug, *nextBug;

  if ((int)SendMessage(writeLogHwnd, BM_GETCHECK, 0, 0)) {
    outf = openNewLog();
  } else {
    outf = NULL;
  }

  srand(time(NULL));
  //srand(1);
  bugBrush[0] = CreateSolidBrush(RGB(100, 100, 255));
  bugBrush[1] = CreateSolidBrush(RGB(255, 100, 100));
  bugBrush[2] = CreateSolidBrush(RGB(100, 255, 100));
  bugBrush[3] = CreateSolidBrush(RGB(255, 255, 255));

  backgroundBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

  cycle = 0;
  backbuffDC = CreateCompatibleDC(hdc);
  createInitialBugs();
  paintInitialBacteria(hdc, 20000);
  processBugs();
  updateBoard(hdc);
  InvalidateRect(reportHwnd, NULL, TRUE);

  while (ThreadRun) {
//    Sleep(50);
    AddRandomBacteria();
//    AddRandomBacteria();
//    AddRandomBacteria();
    processBugs();
    if (bugCount <= 0) {
      if (outf) {
        fprintf(outf, "Cycle %8d: BUGAPOCALYPSE!  All bugs died!\n", cycle);
      }
      break;
    }
    updateBoard(hdc);

    // Generate report
    if (cycle % 10 == 0) {
      InvalidateRect(reportHwnd, NULL, TRUE);
    }
    cycle += 1;
  }

  DeleteDC(backbuffDC);
  backbuffDC = NULL;
  ReleaseDC(childHwnd, hdc);
  DeleteObject(backgroundBrush);
  DeleteObject(bugBrush[0]);
  DeleteObject(bugBrush[1]);
  DeleteObject(bugBrush[2]);
  DeleteObject(bugBrush[3]);

  if (outf) {
    fclose(outf);
  }

  bug = firstBug;
  while (bug) {
    nextBug = bug->next;
    free(bug);
    bug = nextBug;
  }

  ThreadRun = FALSE;
}
