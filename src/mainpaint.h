#ifndef __MAINPAINT_H__
#define __MAINPAINT_H__

#include <windows.h>

extern int cxClient;
extern int cyClient;

extern HWND childHwnd;
extern HWND reportHwnd;
extern HWND startPushButtonHwnd;
extern HWND stopPushButtonHwnd;
extern HWND writeLogHwnd;
extern HWND delayTrackHwnd;
extern HWND reseedTrackHwnd;

#define CHILD_WND_WIDTH   700
#define CHILD_WND_HEIGHT  700

#define IDC_START_BUTTON    1
#define IDC_STOP_BUTTON     2
#define IDC_DELAY_TRACK     3
#define IDC_PAUSE_BUTTON    4
#define IDC_RESUME_BUTTON   5

void paintMain(HDC hdc, HWND hwnd, PAINTSTRUCT *ps);

void onCreate(HWND hwnd, char *ChildWndClass, char *ReportWndClass);

void handleDelayTrack(HWND hwnd);

VOID SimStopped();

VOID SimRunning();

VOID SimPaused();

int getDelaySetting();

int getReseedSetting();

#endif
