#include "boardpaint.h"
#include "mainpaint.h"

#include <stdlib.h>
#include <process.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

HBITMAP backbuffer = NULL;
HDC backbuffDC = NULL;

COLORREF bacteriaColor = RGB(0, 255, 0);

int maxGeneValue = 6;

BOOL ThreadRun = FALSE;
BOOL ThreadPaused = FALSE;

FILE *outf;

SimParams simParams;

int delayChoices[] = {0, 1, 2, 3, 5, 10, 50, 250, 1000};

int reseedChoices[] = {10, 20, 33, 50, 66, 75, 100, 200, 300, 500};

int initBacteriaChoices[] = {1000, 2500, 5000, 7500, 10000, 15000, 20000, 25000, 50000, 100000};

int initBugChoices[] = {1, 5, 10, 20, 50, 100, 150, 200, 300, 500};

struct Bug {
  int id;
  int x;
  int y;
  int dir;
  int health;
  int age;

  int parentId;

  int geneTotal;

  int brushIndex;

  // For Dewdney gene model
  int gene[6];
  int geneWeight[6];

  // For simplified_ga gene model
  char *ga_genes;
  int ga_genes_length;
  int ga_op_index;

  struct Bug *next;
  struct Bug *prev;
};

COLORREF bugColor[] = {
  RGB(100, 100, 255),   // blue
  RGB(255, 100, 100),   // red
  RGB(60, 196, 191),    // cyan
  RGB(255, 255, 255)    // white
};
HBRUSH bugBrush[4];

struct Bug *firstBug;
struct Bug *currentBug;

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

int px(int vx) {
  if (vx < 0) {
    return vx + simParams.worldWidth;
  } else if (vx >= simParams.worldWidth - 1) {
    return vx % simParams.worldWidth;
  } else {
    return vx;
  }
}

int py(int vy) {
  if (vy < 0) {
    return vy + simParams.worldHeight;
  } else if (vy >= simParams.worldHeight - 1) {
    return vy % simParams.worldHeight;
  } else {
    return vy;
  }
}

VOID bugsEatBacteria() {
  int x, y;
  struct Bug *bug;

  startBugLoop();

  while (bug = nextBug()) {
    x = bug->x;
    y = bug->y;
    if (GetPixel(backbuffDC, px(x - 1), py(y - 1)) == bacteriaColor)
      removeBacteria(px(x - 1), py(y - 1), bug);
    if (GetPixel(backbuffDC, px(x    ), py(y - 1)) == bacteriaColor)
      removeBacteria(px(x    ), py(y - 1), bug);
    if (GetPixel(backbuffDC, px(x + 1), py(y - 1)) == bacteriaColor)
      removeBacteria(px(x + 1), py(y - 1), bug);
    if (GetPixel(backbuffDC, px(x - 1), py(y    )) == bacteriaColor)
      removeBacteria(px(x - 1), py(y    ), bug);
    if (GetPixel(backbuffDC, px(x    ), py(y    )) == bacteriaColor)
      removeBacteria(px(x    ), py(y    ), bug);
    if (GetPixel(backbuffDC, px(x + 1), py(y    )) == bacteriaColor)
      removeBacteria(px(x + 1), py(y    ), bug);
    if (GetPixel(backbuffDC, px(x - 1), py(y + 1)) == bacteriaColor)
      removeBacteria(px(x - 1), py(y + 1), bug);
    if (GetPixel(backbuffDC, px(x    ), py(y + 1)) == bacteriaColor)
      removeBacteria(px(x    ), py(y + 1), bug);
    if (GetPixel(backbuffDC, px(x + 1), py(y + 1)) == bacteriaColor)
      removeBacteria(px(x + 1), py(y + 1), bug);
  }
}

VOID writeOutBugInfo(struct Bug* bug) {
  if (!outf) return;

  fprintf(outf, "Bug %4d [parent: %4d, age: %8d, health: %4d, x: %3d, y: %3d]\n",
    bug->id, bug->parentId, bug->age, bug->health, bug->x, bug->y);
  if (simParams.geneModel == dewdney) {
    fprintf(outf, "  Genetics: %4d %4d %4d %4d %4d %4d\n",
      bug->gene[0], bug->gene[1], bug->gene[2], bug->gene[3], bug->gene[4], bug->gene[5]);
    fprintf(outf, "  Weights : %4d %4d %4d %4d %4d %4d\n",
      bug->geneWeight[0], bug->geneWeight[1], bug->geneWeight[2],
      bug->geneWeight[3], bug->geneWeight[4], bug->geneWeight[5]);
  } else {
    fprintf(outf, "  Genetics: %s [%d]\n", bug->ga_genes, bug->ga_genes_length);
  }
}

VOID calculateBugWeights(struct Bug *bug) {
  int i;
  int gpow[6];

  if (simParams.geneModel == simplified_ga) return;

  bug->geneTotal = 0;
  for (i = 0; i < 6; i++) {
    gpow[i] = (int)pow(2, bug->gene[i]);
    bug->geneTotal += gpow[i];
  }

  for (i = 0; i < 6; i++) {
    bug->geneWeight[i] = (int)(((double)gpow[i] / (double)bug->geneTotal) * 100);
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

  result->ga_genes = NULL;
  result->ga_genes_length = 0;

  result->id = lastBugId++;
  result->parentId = 0;
  result->dir = rand()%6;

  result->health = 800;
  result->age = 0;

  if (simParams.geneModel == dewdney) {
    result->gene[0] = rand()%maxGeneValue;
    result->gene[1] = rand()%maxGeneValue;
    result->gene[2] = rand()%maxGeneValue;
    result->gene[3] = rand()%maxGeneValue;
    result->gene[4] = rand()%maxGeneValue;
    result->gene[5] = rand()%maxGeneValue;
    calculateBugWeights(result);
    assignBrushToBug(result);
  } else if (simParams.geneModel == simplified_ga) {
    result->ga_genes_length = rand()%20 + 1;
    result->ga_genes = (char *)malloc(result->ga_genes_length + 1);
    for (x = 0; x < result->ga_genes_length; x++) {
      result->ga_genes[x] = 48 + rand()%maxGeneValue;
    }
    result->ga_genes[result->ga_genes_length] = '\0';
    result->ga_op_index = 0;
  }

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

    if (simParams.toroidal) {
      if (bug->x < 0) {
        bug->x += simParams.worldWidth;
      } else if (bug->x >= simParams.worldWidth) {
        bug->x -= simParams.worldWidth;
      }

      if (bug->y < 0) {
        bug->y += simParams.worldHeight;
      } else if (bug->y >= simParams.worldHeight) {
        bug->y -= simParams.worldHeight;
      }
    } else {
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
}

VOID drawBugsOnBoard(HBRUSH brush) {
  RECT rect;
  int x, y;
  struct Bug *bug;
  COLORREF color = RGB(0, 0, 0);

  startBugLoop();

  while (bug = nextBug()) {
    if (bug->x <= 0 || bug->x >= simParams.worldWidth - 1 ||
        bug->y <= 0 || bug->y >= simParams.worldHeight - 1
    ) {
      if (!brush) {
        color = bugColor[bug->brushIndex];
      }
      x = bug->x;
      y = bug->y;
      SetPixel(backbuffDC, px(x - 1), py(y - 1), color);
      SetPixel(backbuffDC, px(x    ), py(y - 1), color);
      SetPixel(backbuffDC, px(x + 1), py(y - 1), color);
      SetPixel(backbuffDC, px(x - 1), py(y    ), color);
      SetPixel(backbuffDC, px(x    ), py(y    ), color);
      SetPixel(backbuffDC, px(x + 1), py(y    ), color);
      SetPixel(backbuffDC, px(x - 1), py(y + 1), color);
      SetPixel(backbuffDC, px(x    ), py(y + 1), color);
      SetPixel(backbuffDC, px(x + 1), py(y + 1), color);

    } else {
      rect.left = bug->x - 1;
      rect.top = bug->y - 1;
      rect.right = bug->x + 2;
      rect.bottom = bug->y + 2;
      if (brush) {
        FillRect(backbuffDC, &rect, brush);
      } else {
        FillRect(backbuffDC, &rect, bugBrush[bug->brushIndex]);
      }
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
    if (simParams.geneModel == dewdney) {
      num = rand()%100;

      for (i = 0; i < 6; i++) {
        num -= bug->geneWeight[i];
        if (num <= 0) {
          bug->dir = (bug->dir + i) % 6;
          break;
        }
      }
    } else if (simParams.geneModel == simplified_ga) {
      //if (outf) fprintf(outf, "MOVE for cycle %d\n", cycle);
      bug->dir = (bug->dir + (bug->ga_genes[bug->ga_op_index] - 48)) % 6;
      //if (outf) fprintf(outf, "  Using %d at %d to select new dir %d\n",
      //  bug->ga_genes[bug->ga_op_index] - 48, bug->ga_op_index, bug->dir);
      bug->ga_op_index = (bug->ga_op_index + 1) % bug->ga_genes_length;
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

  if (bug->ga_genes) free(bug->ga_genes);
  free(bug);

  return result;
}

VOID mutateBug(struct Bug *bug) {
  int i, gnum;
  char tmpval;
  char *tmp;

  if (simParams.geneModel == dewdney) {
    gnum = rand()%6;
    i = rand()%100;
    if (i < 34) {
      bug->gene[gnum] = (bug->gene[gnum] + 1) % maxGeneValue;
    } else if (i < 67){
      bug->gene[gnum] = ((bug->gene[gnum] - 1) + maxGeneValue) % maxGeneValue;
    }
    calculateBugWeights(bug);
    assignBrushToBug(bug);
  } else if (simParams.geneModel == simplified_ga && bug->ga_genes) {
    bug->ga_op_index = 0;
    i = rand()%100;
    if (i < 34 && bug->ga_genes_length > 1) {  // Remove one
      gnum = rand()%bug->ga_genes_length;
      if (outf)
        fprintf(outf, "Mutation for %d: Remove %d\n", bug->id, gnum);
      tmp = (char *)malloc(bug->ga_genes_length);
      if (gnum > 0) {
        strncpy(tmp, bug->ga_genes, gnum);
      }
      if (gnum < bug->ga_genes_length - 1) {
        strcpy(tmp + gnum, bug->ga_genes + gnum + 1);
      } else {
        tmp[bug->ga_genes_length] = '\0';
      }
      free(bug->ga_genes);
      bug->ga_genes = tmp;
      bug->ga_genes_length--;
    } else if (i < 67) { // Add one
      gnum = rand()%(bug->ga_genes_length + 1);
      tmp = (char *)malloc(bug->ga_genes_length + 2);
      if (gnum > 0) {
        strncpy(tmp, bug->ga_genes, gnum);
      }
      tmp[gnum] = 48 + rand()%maxGeneValue;
      if (outf)
        fprintf(outf, "Mutation for %d: Add %s at %d\n", bug->id, tmp[gnum], gnum);
      if (gnum < bug->ga_genes_length - 1) {
        strcpy(tmp + gnum + 1, bug->ga_genes + gnum);
      } else {
        tmp[bug->ga_genes_length + 1] = '\0';
      }
      free(bug->ga_genes);
      bug->ga_genes = tmp;
      bug->ga_genes_length++;
    } else { // Change existing
      gnum = rand()%bug->ga_genes_length;
      i = rand()%100;
      tmpval = 48 - bug->ga_genes[gnum];
      if (i < 51) {
        bug->ga_genes[gnum] = 48 + ((tmpval + 1) % maxGeneValue);
      } else {
        bug->ga_genes[gnum] = 48 + (((tmpval - 1) + maxGeneValue) % maxGeneValue);
      }
      if (outf)
        fprintf(outf, "Mutation for %d: Set %d from %s to %s\n", bug->id, gnum, 48 + tmpval, bug->ga_genes[gnum]);
    }
  }
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

      // Copy dewdney genes
      for (i = 0; i < 6; i++) {
        newBug->gene[i] = bug->gene[i];
      }

      // Copy simplified_ga genes
      if (bug->ga_genes) {
        if (newBug->ga_genes) free(newBug->ga_genes);
        newBug->ga_genes = (char *)malloc(bug->ga_genes_length + 1);
        strcpy(newBug->ga_genes, bug->ga_genes);
        newBug->ga_genes_length = bug->ga_genes_length;
      }

      mutateBug(newBug);
      writeOutBugInfo(newBug);

      bug->health = bug->health / 2;
      bug->id = lastBugId++;
      bug->parentId = parentId;
      bug->age = 0;

      mutateBug(bug);
      writeOutBugInfo(bug);

      insertAfter(bug, newBug);

      bugCount += 2;
    } else {
      bugCount += 1;
    }
  }
}

VOID createInitialBugs(int startingBugCount) {
  int i;
  struct Bug *newBug = NULL, *lastBug = NULL;

  for (i = 0; i < startingBugCount; i++) {
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
  if (bug->geneWeight[0] < 26) {
    bug->brushIndex = 0;
  } else if (bug->geneWeight[0] < 51) {
    bug->brushIndex = 1;
  } else if (bug->geneWeight[0] < 76) {
    bug->brushIndex = 2;
  } else {
    bug->brushIndex = 3;
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

VOID rereadSimParams() {
  simParams.delay = delayChoices[getDelaySetting()];
  simParams.reseedRate = reseedChoices[getReseedSetting()];
}

VOID populateSimParams() {
  simParams.toroidal = TRUE;
  simParams.worldWidth = CHILD_WND_WIDTH;
  simParams.worldHeight = CHILD_WND_HEIGHT;
  simParams.startingBacteria = initBacteriaChoices[getInitBacteriaSetting()];
  simParams.startingBugs = initBugChoices[getInitBugsSetting()];
  simParams.reseedRate = reseedChoices[getReseedSetting()];
  simParams.delay = delayChoices[getDelaySetting()];
  simParams.geneModel = simplified_ga;
}

VOID Thread(PVOID pvoid) {
  HDC hdc = GetDC(childHwnd);
  struct Bug *bug, *nextBug;
  time_t rtime;
  int i, reseeding = 0;

  if ((int)SendMessage(writeLogHwnd, BM_GETCHECK, 0, 0)) {
    outf = openNewLog();
  } else {
    outf = NULL;
  }

  populateSimParams();

  rtime = time(NULL);
  if (outf) fprintf(outf, "Random seed: %u\n", rtime);
  srand(rtime);
  //srand(1);

  for (i = 0; i < 4; i++) {
    bugBrush[i] = CreateSolidBrush(bugColor[i]);
  }

  backgroundBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

  // Initialize run variables
  lastBugId = 1;
  cycle = 0;

  backbuffDC = CreateCompatibleDC(hdc);
  createInitialBugs(simParams.startingBugs);
  paintInitialBacteria(hdc, simParams.startingBacteria);
  processBugs();
  updateBoard(hdc);
  InvalidateRect(reportHwnd, NULL, TRUE);

  while (ThreadRun) {
    while (ThreadPaused) {
      Sleep(250);
    }

    if (simParams.delay) {
      Sleep(simParams.delay);
    }

    reseeding += simParams.reseedRate;
    while (reseeding >= 100) {
      AddRandomBacteria();
      reseeding -= 100;
    }

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

  // Final report to true things up
  InvalidateRect(reportHwnd, NULL, TRUE);

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

  SimStopped();
  ThreadRun = FALSE;
}
