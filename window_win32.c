/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter (see license.txt)
**
**  Name: window_win32.c
**
**  Description:
**      Simulate CDC 6612 console display on MS Windows.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "resource.h"
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define ListSize            5000
#define FontName            "Lucida Console"
#if CcLargeWin32Screen == 1
#define FontSmallHeight     15
#define FontMediumHeight    20
#define FontLargeHeight     30
#define ScaleX              12
#define ScaleY              18
#else
#define FontSmallHeight     10
#define FontMediumHeight    15
#define FontLargeHeight     20
#define ScaleX              10
#define ScaleY              10
#endif

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/
typedef struct dispList
    {
    u16             xPos;           /* horizontal position */
    u16             yPos;           /* vertical position */
    u8              fontSize;       /* size of font */
    char            ch;             /* character to be displayed */
    } DispList;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void windowThread(void);
ATOM windowRegisterClass(HINSTANCE hInstance);
BOOL windowCreate(void);
LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);
void windowDisplay(HWND hWnd);

/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 currentFont;
static i16 currentX = -1;
static i16 currentY = -1;
static DispList display[ListSize];
static u32 listEnd;
static HWND hWnd;
static HFONT hSmallFont=0;
static HFONT hMediumFont=0;
static HFONT hLargeFont=0;

/*--------------------------------------------------------------------------
**  Purpose:        Create WIN32 thread which will deal with all windows
**                  functions.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowInit(void)
    {
    DWORD dwThreadId; 
    HANDLE hThread;

    /*
    **  Create display list pool.
    */
    listEnd = 0;

    /*
    **  Create windowing thread.
    */
    hThread = CreateThread( 
        NULL,                                       // no security attribute 
        0,                                          // default stack size 
        (LPTHREAD_START_ROUTINE) windowThread, 
        (LPVOID) NULL,                              // thread parameter 
        0,                                          // not suspended 
        &dwThreadId);                               // returns thread ID 

    if (hThread == NULL)
        {
        MessageBox(NULL, "thread creation failed", "Error", MB_OK);
        exit(1);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set font size.
**                  functions.
**
**  Parameters:     Name        Description.
**                  size        font size in points.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetFont(u8 font)
    {
    currentFont = font;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set X coordinate.
**
**  Parameters:     Name        Description.
**                  x           horinzontal coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetX(u16 x)
    {
    currentX = x;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set Y coordinate.
**
**  Parameters:     Name        Description.
**                  y           horinzontal coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetY(u16 y)
    {
    currentY = 0777 - y;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Queue characters.
**
**  Parameters:     Name        Description.
**                  ch          character to be queued.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowQueue(char ch)
    {
    DispList *elem;

    if (   listEnd  >= ListSize
        || currentX == -1
        || currentY == -1)
        {
        return;
        }

    if (ch != 0)
        {
        elem = display + listEnd++;
        elem->ch = ch;
        elem->fontSize = currentFont;
        elem->xPos = currentX;
        elem->yPos = currentY;
        }

    currentX += currentFont;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Update window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowUpdate(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Poll the keyboard (dummy for X11)
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing
**
**------------------------------------------------------------------------*/
void windowGetChar(void)
    {
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowClose(void)
    {
    SendMessage(hWnd, WM_DESTROY, 0, 0);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Windows thread.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowThread(void)
    {
    MSG msg;
    HINSTANCE hInstance = 0;

    /*
    **  Register the window class.
    */
    windowRegisterClass(hInstance);

    /*
    **  Create the window.
    */
    if (!windowCreate()) 
        {
        MessageBox(NULL, "window creation failed", "Error", MB_OK);
        return;
        }

    /*
    **  Main message loop.
    */
    while (GetMessage(&msg, NULL, 0, 0) > 0) 
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        Sleep(25);
        }
    }


/*--------------------------------------------------------------------------
**  Purpose:        Register the window class.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
ATOM windowRegisterClass(HINSTANCE hInstance)
    {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)windowProcedure;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_CONSOLE);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCSTR)IDC_CONSOLE;
    wcex.lpszClassName  = "CONSOLE";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Create the main window.
**
**  Parameters:     Name        Description.
**
**  Returns:        TRUE if successful, FALSE otherwise.
**
**------------------------------------------------------------------------*/
static BOOL windowCreate(void)
    {
    hWnd = CreateWindow(
        "CONSOLE",              // Registered class name
        DtCyberVersion,         // window name
        WS_OVERLAPPEDWINDOW,    // window style
        CW_USEDEFAULT,          // horizontal position of window
        0,                      // vertical position of window
        CW_USEDEFAULT,          // window width
        0,                      // window height
        NULL,                   // handle to parent or owner window
        NULL,                   // menu handle or child identifier
        0,                      // handle to application instance
        NULL);                  // window-creation data

    if (!hWnd)
        {
        return FALSE;
        }

    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hWnd);

#define TIMER_ID        1
#define TIMER_RATE      200
//#define TIMER_RATE      20      // <<<<<<<<<<<< testing

    SetTimer(hWnd, TIMER_ID, TIMER_RATE, NULL);

    return TRUE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process messages for the main window.
**
**  Parameters:     Name        Description.
**
**  Returns:        LRESULT
**
**------------------------------------------------------------------------*/
static LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
    int wmId, wmEvent;
    LOGFONT lfTmp;
    RECT rt;

    switch (message) 
        {
    /*
    **  Process the application menu.
    */
    case WM_COMMAND:
        wmId    = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 

        switch (wmId)
            {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            }
        break;

    case WM_ERASEBKGND:
        return(1);
        break;

    case WM_CREATE:
        memset(&lfTmp, 0, sizeof(lfTmp));
        lfTmp.lfPitchAndFamily = FIXED_PITCH;
        strcpy(lfTmp.lfFaceName, FontName);
        lfTmp.lfWeight = FW_THIN;
        lfTmp.lfOutPrecision = OUT_TT_PRECIS;
        lfTmp.lfHeight = FontSmallHeight;
        hSmallFont = CreateFontIndirect (&lfTmp);
        if (!hSmallFont)
            {
            MessageBox (GetFocus(),
                "Unable to get font in 15 point", 
                "CreateFont Error",
                MB_OK);
            }

        memset(&lfTmp, 0, sizeof(lfTmp));
        lfTmp.lfPitchAndFamily = FIXED_PITCH;
        strcpy(lfTmp.lfFaceName, FontName);
        lfTmp.lfWeight = FW_THIN;
        lfTmp.lfOutPrecision = OUT_TT_PRECIS;
        lfTmp.lfHeight = FontMediumHeight;
        hMediumFont = CreateFontIndirect (&lfTmp);
        if (!hMediumFont)
            {
            MessageBox (GetFocus(),
                "Unable to get font in 20 point", 
                "CreateFont Error",
                MB_OK);
            }

        memset(&lfTmp, 0, sizeof(lfTmp));
        lfTmp.lfPitchAndFamily = FIXED_PITCH;
        strcpy(lfTmp.lfFaceName, FontName);
        lfTmp.lfWeight = FW_THIN;
        lfTmp.lfOutPrecision = OUT_TT_PRECIS;
        lfTmp.lfHeight = FontLargeHeight;
        hLargeFont = CreateFontIndirect (&lfTmp);
        if (!hLargeFont)
            {
            MessageBox (GetFocus(),
                "Unable to get font in 30 point", 
                "CreateFont Error",
                MB_OK);
            }

        return DefWindowProc (hWnd, message, wParam, lParam);

    case WM_DESTROY:
        if (hSmallFont)
            {
            DeleteObject(hSmallFont);
            }
        if (hMediumFont)
            {
            DeleteObject(hMediumFont);
            }
        if (hLargeFont)
            {
            DeleteObject(hLargeFont);
            }
        PostQuitMessage(0);
        break;

    case WM_TIMER:
        GetClientRect(hWnd, &rt);
        InvalidateRect(hWnd, &rt, TRUE);
        break;

    /*
    **  Paint the main window.
    */
    case WM_PAINT:
        windowDisplay(hWnd);
        break;

    /*
    **  Handle input characters.
    */
    case WM_SYSCHAR:
        switch (wParam)
            {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            traceMask ^= (1 << (wParam - '0'));
            break;

        case 'C':
        case 'c':
            traceMask ^= (1 << 14);
            break;

        case 'E':
        case 'e':
            traceMask ^= (1 << 15);
            break;

        case 'X':
        case 'x':
            if (traceMask == 0)
                {
                traceMask = ~0;
                }
            else
                {
                traceMask = 0;
                }
            break;

        case 'O':
        case 'o':
            opActive = TRUE;
            break;
            }
        break;

    case WM_CHAR:
        ppKeyIn = wParam;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        }

    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Display current list.
**
**  Parameters:     Name        Description.
**                  hWnd        window handle.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowDisplay(HWND hWnd)
    {
    static refreshCount = 0;
    char str[2] = " ";
    DispList *curr;
    DispList *end;
    u8 oldFont = 0;

    RECT rect;
    PAINTSTRUCT ps;
    HDC hdc;
    HBRUSH hBrush;

    HDC hdcMem;
    HBITMAP hbmMem, hbmOld;
    HFONT hfntOld;


//    if (listEnd == 0)
//        return;

    hdc = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &rect);

    /*
    **  Create a compatible DC.
    */

    hdcMem = CreateCompatibleDC(ps.hdc);

    /*
    **  Create a bitmap big enough for our client rect.
    */
    hbmMem = CreateCompatibleBitmap(ps.hdc,
                                    rect.right-rect.left,
                                    rect.bottom-rect.top);

    /*
    **  Select the bitmap into the off-screen dc.
    */
    hbmOld = SelectObject(hdcMem, hbmMem);

    hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect (hdcMem, &rect, hBrush);
    DeleteObject(hBrush);

SetBkMode(hdcMem, TRANSPARENT);     // <<<<<<<<<<<< testing
    SetBkColor(hdcMem, RGB(0, 0, 0));
    SetTextColor(hdcMem, RGB(0, 255, 0));

    hfntOld = SelectObject(hdcMem, hSmallFont);
    oldFont = FontSmall;

#if CcDebug == 1
    {
    char buf[160];

    /*
    **  Display P registers of PPUs and CPU and current trace mask.
    */
    sprintf(buf, "Refresh: %-10d  PP P-reg: %04o %04o %04o %04o %04o %04o %04o %04o %04o %04o   CPU P-reg: %06o",
        refreshCount++,
        ppu[0].regP, ppu[1].regP, ppu[2].regP, ppu[3].regP, ppu[4].regP,
        ppu[5].regP, ppu[6].regP, ppu[7].regP, ppu[8].regP, ppu[9].regP,
        cpu.regP); 

    sprintf(buf + strlen(buf), "   Trace: %c%c%c%c%c%c%c%c%c%c%c%c",
        (traceMask >> 0) & 1 ? '0' : '_',
        (traceMask >> 1) & 1 ? '1' : '_',
        (traceMask >> 2) & 1 ? '2' : '_',
        (traceMask >> 3) & 1 ? '3' : '_',
        (traceMask >> 4) & 1 ? '4' : '_',
        (traceMask >> 5) & 1 ? '5' : '_',
        (traceMask >> 6) & 1 ? '6' : '_',
        (traceMask >> 7) & 1 ? '7' : '_',
        (traceMask >> 8) & 1 ? '8' : '_',
        (traceMask >> 9) & 1 ? '9' : '_',
        (traceMask >> 14) & 1 ? 'C' : '_',
        (traceMask >> 15) & 1 ? 'E' : '_');

    TextOut(hdcMem, 0, 0, buf, strlen(buf));
    }
#endif

    if (opActive)
        {
        static char opMessage[] = "Operator Interface Active";
        hfntOld = SelectObject(hdcMem, hLargeFont);
        oldFont = FontLarge;
        TextOut(hdcMem, (0 * ScaleX) / 10, (256 * ScaleY) / 10, opMessage, strlen(opMessage));
        }

    curr = display;
    end = display + listEnd;
    for (curr = display; curr < end; curr++)
        {
        if (oldFont != curr->fontSize)
            {
            oldFont = curr->fontSize;

            switch (oldFont)
                {
            case FontSmall:
                SelectObject(hdcMem, hSmallFont);
                break;

            case FontMedium:
                SelectObject(hdcMem, hMediumFont);
                break;
    
            case FontLarge:
                SelectObject(hdcMem, hLargeFont);
                break;
                }
            }

        if (curr->fontSize == FontDot)
            {
            SetPixel(hdcMem, (curr->xPos * ScaleX) / 10, (curr->yPos * ScaleY) / 10 + 30, RGB(0, 255, 0));
            }
        else
            {
            str[0] = curr->ch;
            TextOut(hdcMem, (curr->xPos * ScaleX) / 10, (curr->yPos * ScaleY) / 10 + 20, str, 1);
            }
        }

    listEnd = 0;
    currentX = -1;
    currentY = -1;

    if (hfntOld)
        {
        SelectObject(hdcMem, hfntOld);
        }


    /*
    **  Blit the changes to the screen dc.
    */
    BitBlt(ps.hdc, 
           rect.left, rect.top,
           rect.right-rect.left, rect.bottom-rect.top,
           hdcMem,
           0, 0,
           SRCCOPY);

    /*
    **  Done with off screen bitmap and dc.
    */
    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);


    EndPaint(hWnd, &ps);
    }

/*---------------------------  End Of File  ------------------------------*/
