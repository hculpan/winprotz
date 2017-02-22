#include "mainpaint.h"

#include <string.h>
#include <commctrl.h>
#include <stdio.h>

int cxClient = 0;
int cyClient = 0;

HWND childHwnd = NULL;
HWND reportHwnd = NULL;
HWND startPushButtonHwnd;
HWND stopPushButtonHwnd;
HWND writeLogHwnd;
HWND startingBacteriaHwnd;
HWND delayTrackHwnd;
HWND delayLabelHwnd;
HWND pauseButtonHwnd;
HWND resumeButtonHwnd;
HWND reseedTrackHwnd;
HWND reseedLabelHwnd;
HWND initBacteriaTrackHwnd;
HWND initBacteriaLabelHwnd;
HWND initBugsTrackHwnd;
HWND initBugsLabelHwnd;


char *delayText[] = {
  "None",
  "1 ms",
  "2 ms",
  "3 ms",
  "5 ms",
  "10 ms",
  "50 ms",
  "1/4 sec",
  "1 sec"
};

char *reseedText[] = {
  "1/10",
  "2/5",
  "1/3",
  "1/2",
  "2/3",
  "3/4",
  "1/1",
  "2/1",
  "3/1",
  "5/1"
};

char *initBacteriaText[] = {
  "1k",
  "2.5k",
  "5k",
  "7.5k",
  "10k",
  "15k",
  "20k",
  "25k",
  "50k",
  "100k"
};

char *initBugsText[] = {
  "1",
  "5",
  "10",
  "20",
  "50",
  "100",
  "150",
  "200",
  "300",
  "500"
};

TEXTMETRIC tm;

VOID SimStopped() {
  EnableWindow(startPushButtonHwnd, TRUE);
  EnableWindow(stopPushButtonHwnd, FALSE);
  EnableWindow(pauseButtonHwnd, FALSE);
  EnableWindow(resumeButtonHwnd, FALSE);
  EnableWindow(writeLogHwnd, TRUE);
  EnableWindow(delayTrackHwnd, TRUE);
  EnableWindow(reseedTrackHwnd, TRUE);
  EnableWindow(initBacteriaTrackHwnd, TRUE);
  EnableWindow(initBugsTrackHwnd, TRUE);
}

VOID SimRunning() {
  EnableWindow(startPushButtonHwnd, FALSE);
  EnableWindow(stopPushButtonHwnd, TRUE);
  EnableWindow(pauseButtonHwnd, TRUE);
  EnableWindow(resumeButtonHwnd, FALSE);
  EnableWindow(writeLogHwnd, FALSE);
  EnableWindow(delayTrackHwnd, FALSE);
  EnableWindow(reseedTrackHwnd, FALSE);
  EnableWindow(initBacteriaTrackHwnd, FALSE);
  EnableWindow(initBugsTrackHwnd, FALSE);
}

VOID SimPaused() {
  EnableWindow(startPushButtonHwnd, FALSE);
  EnableWindow(stopPushButtonHwnd, TRUE);
  EnableWindow(pauseButtonHwnd, FALSE);
  EnableWindow(resumeButtonHwnd, TRUE);
  EnableWindow(writeLogHwnd, FALSE);
  EnableWindow(delayTrackHwnd, TRUE);
  EnableWindow(reseedTrackHwnd, TRUE);
  EnableWindow(initBacteriaTrackHwnd, FALSE);
  EnableWindow(initBugsTrackHwnd, FALSE);
}

int getDelaySetting() {
  return (int)SendMessage(delayTrackHwnd, TBM_GETPOS, 0, 0);
}

int getReseedSetting() {
  return (int)SendMessage(reseedTrackHwnd, TBM_GETPOS, 0, 0);
}

int getInitBacteriaSetting() {
  return (int)SendMessage(initBacteriaTrackHwnd, TBM_GETPOS, 0, 0);
}

int getInitBugsSetting() {
  return (int)SendMessage(initBugsTrackHwnd, TBM_GETPOS, 0, 0);
}

// CreateTrackbar - creates and initializes a trackbar.
//
// Global variable
//     g_hinst - instance handle
//
HWND WINAPI CreateTrackbar(
    HWND hwnd,     // handle of dialog box (parent window)
    UINT iMin,     // minimum value in trackbar range
    UINT iMax,     // maximum value in trackbar range
    UINT iPos,     // Initial value
    UINT xPos,
    UINT yPos,
    UINT trackbarId,
    HWND *trackHwnd,
    HWND *labelHwnd) {

    InitCommonControls(); // loads common control's DLL

    *labelHwnd = CreateWindow("static", "ST_U",
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                             xPos, yPos, 115, 30,
                             hwnd, (HMENU)(501),
                             (HINSTANCE) GetWindowLong (hwnd, GWL_HINSTANCE), NULL);

    *trackHwnd = CreateWindowEx(
        0,                               // no extended styles
        TRACKBAR_CLASS,                  // class name
        "Trackbar Control",              // title (caption)
        WS_CHILD |
        WS_VISIBLE |
        TBS_AUTOTICKS,                   // style
        xPos + 120, yPos,                 // position
        160, 30,                         // size
        hwnd,                            // parent window
        (HMENU)trackbarId,               // control identifier
        (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
        NULL                             // no WM_CREATE parameter
        );

    SendMessage(*trackHwnd, TBM_SETRANGE,
        (WPARAM) TRUE,                   // redraw flag
        (LPARAM) MAKELONG(iMin, iMax));  // min. & max. positions

    SendMessage(*trackHwnd, TBM_SETPAGESIZE,
        0, (LPARAM) 4);                  // new page size

    SendMessage(*trackHwnd, TBM_SETPOS, TRUE, (LPARAM)iPos);

    return *trackHwnd;
}

void handleDelayTrack(HWND hwnd) {
  int pos;
  char msg[50];

  pos = getDelaySetting();
  sprintf(msg, "Delay: %s", delayText[pos]);
  SetWindowText(delayLabelHwnd, msg);
}

void handleReseedTrack(HWND hwnd) {
  int pos;
  char msg[50];

  pos = getReseedSetting();
  sprintf(msg, "Reseed: %s", reseedText[pos]);
  SetWindowText(reseedLabelHwnd, msg);
}

void handleInitBacteriaTrack(HWND hwnd) {
  int pos;
  char msg[50];

  pos = getInitBacteriaSetting();
  sprintf(msg, "Bacteria: %s", initBacteriaText[pos]);
  SetWindowText(initBacteriaLabelHwnd, msg);
}

void handleInitBugsTrack(HWND hwnd) {
  int pos;
  char msg[50];

  pos = getInitBugsSetting();
  sprintf(msg, "Bugs: %s", initBugsText[pos]);
  SetWindowText(initBugsLabelHwnd, msg);
}

void onCreate(HWND hwnd, char *ChildWndClass, char *ReportWndClass) {
  childHwnd = CreateWindow(
    ChildWndClass,
    NULL,
    WS_CHILDWINDOW | WS_VISIBLE,
    5, 5,
    CHILD_WND_WIDTH, CHILD_WND_HEIGHT,
    hwnd,
    NULL,
    (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
    NULL
  );

  reportHwnd = CreateWindow(
    ReportWndClass,
    NULL,
    WS_CHILDWINDOW | WS_VISIBLE,
    CHILD_WND_WIDTH + 10, (CHILD_WND_HEIGHT + 5) - 100,
    305, 100,
    hwnd,
    NULL,
    (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
    NULL
  );

  startPushButtonHwnd = CreateWindow("button", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    CHILD_WND_WIDTH + 30, 15, 60, 26, hwnd, (HMENU)IDC_START_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  pauseButtonHwnd = CreateWindow("button", "Pause", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    CHILD_WND_WIDTH + 100, 15, 60, 26, hwnd, (HMENU)IDC_PAUSE_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
  EnableWindow(pauseButtonHwnd, FALSE);

  resumeButtonHwnd = CreateWindow("button", "Resume", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    CHILD_WND_WIDTH + 170, 15, 60, 26, hwnd, (HMENU)IDC_RESUME_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
  EnableWindow(resumeButtonHwnd, FALSE);

  stopPushButtonHwnd = CreateWindow("button", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    CHILD_WND_WIDTH + 240, 15, 60, 26, hwnd, (HMENU)IDC_STOP_BUTTON, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
  EnableWindow(stopPushButtonHwnd, FALSE);

  writeLogHwnd = CreateWindow("button", "Write log", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
    CHILD_WND_WIDTH + 5 + 25, 55, 100, 26, hwnd, (HMENU)0, (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), NULL);

  CreateTrackbar(hwnd, 0, 8, 0, CHILD_WND_WIDTH + 30, 105, IDC_DELAY_TRACK, &delayTrackHwnd, &delayLabelHwnd);
  handleDelayTrack(hwnd);

  CreateTrackbar(hwnd, 0, 9, 6, CHILD_WND_WIDTH + 30, 150, IDC_DELAY_TRACK, &reseedTrackHwnd, &reseedLabelHwnd);
  handleReseedTrack(hwnd);

  CreateTrackbar(hwnd, 0, 9, 6, CHILD_WND_WIDTH + 30, 195, IDC_DELAY_TRACK, &initBacteriaTrackHwnd, &initBacteriaLabelHwnd);
  handleInitBacteriaTrack(hwnd);

  CreateTrackbar(hwnd, 0, 9, 3, CHILD_WND_WIDTH + 30, 240, IDC_DELAY_TRACK, &initBugsTrackHwnd, &initBugsLabelHwnd);
  handleInitBugsTrack(hwnd);

  InvalidateRect(childHwnd, NULL, TRUE);
  InvalidateRect(reportHwnd, NULL, TRUE);
}

void paintMain(HDC hdc, HWND hwnd, PAINTSTRUCT *ps) {
}
