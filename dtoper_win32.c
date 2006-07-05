/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, 2004, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: dtoper_win32.c
**
**  Description:
**      DtCyber operator interface UI on Microsoft Windows.
**
**--------------------------------------------------------------------------
*/

//#define DEBUG 

/*
**  -------------
**  Include Files
**  -------------
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "resource.h"
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define KeyBufSize      64
#define DisplayMargin   8

// These are used only for the operator interface font
#define FontName            "Lucida Console"
#define FontSmallHeight     12
#define FontMediumHeight    17

/*
**  Size of the window and pixmap.
**  This is: a screen high with marging top and botton.
*/
#define XSize           (02000 + 2 * DisplayMargin)
#define YSize           (01000 + 2 * DisplayMargin)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) ((x) + DisplayMargin)
#define YADJUST(y) (01000 - 1 - DisplayMargin - (y))

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

typedef struct fontInfo
    {
    HFONT           normalId;       /* horizontal position */
    HFONT           boldId;         /* size of font */
    int             width;          /* character width in pixels */
    } FontInfo;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void dtThread(LPVOID foo);
static ATOM dtRegisterClass(HINSTANCE hInstance);
static BOOL dtCreate(void);
static LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);
static void initFont (int size, FontInfo *fi);
static void freeFont (FontInfo *fi);
static void windowQueueKey(char ch);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
extern HINSTANCE hInstance;
extern bool emulationActive;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static HWND hWnd;
static HBRUSH bgBrush;
static RECT rect;
static COLORREF green;
static COLORREF black;
static char dtWinName[100] = "Dtoper" ;
static char keybuf[KeyBufSize+4];
volatile static u32 keyIn, keyOut;
static FontInfo smallOperFont;
static FontInfo mediumOperFont;

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

extern void opDisplay(void);

/*--------------------------------------------------------------------------
**  Purpose:        Initialize the operator window.
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
    **  Create windowing thread.
    */
    hThread = CreateThread( 
        NULL,                                       // no security attribute 
        0,                                          // default stack size 
        (LPTHREAD_START_ROUTINE) dtThread, 
        (LPVOID) 0,                                 // thread parameter 
        0,                                          // not suspended 
        &dwThreadId);                               // returns thread ID 

    if (hThread == NULL)
        {
        MessageBox(NULL, "dt thread creation failed", "Error", MB_OK);
        exit(1);
        }


    /*
    **  All underway...
    */
    while (!emulationActive) ;      // spin until thread is done setting things up
    }

/*--------------------------------------------------------------------------
**  Purpose:        Windows thread.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dtThread(LPVOID foo)
    {
    MSG msg;

    /*
    **  Register the window class.
    */
    dtRegisterClass(hInstance);

    /*
    **  Create the window.
    */
    if (!dtCreate())
        {
        MessageBox(NULL, "dt window creation failed", "Error", MB_OK);
        return;
        }

    /*
    **  Main message loop.
    */
    while (GetMessage(&msg, NULL, 0, 0) > 0) 
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
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
ATOM dtRegisterClass(HINSTANCE hInstance)
    {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    black = RGB(0, 0, 0);
    green = RGB(0, 0xff, 0);
    bgBrush = CreateSolidBrush (black);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)windowProcedure;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_CONSOLE);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = bgBrush;
//    wcex.lpszMenuName   = (LPCSTR)IDC_CONSOLE;
    wcex.lpszClassName  = "DTOPER";
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
static BOOL dtCreate(void)
    {
    int dx, dy;

    dx = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    dy = GetSystemMetrics(SM_CYSIZEFRAME) * 2 +
         GetSystemMetrics(SM_CYCAPTION);
    hWnd = CreateWindow(
        "DTOPER",               // Registered class name
        dtWinName,              // window name
        WS_OVERLAPPEDWINDOW,    // window style
        CW_USEDEFAULT,          // horizontal position of window
        CW_USEDEFAULT,          // vertical position of window
        XSize + dx,             // window width
        YSize + dy,             // window height
        NULL,                   // handle to parent or owner window
        NULL,                   // menu handle or child identifier
        0,                      // handle to application instance
        NULL);                  // window-creation data

    if (!hWnd)
        {
        return FALSE;
        }
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

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
static LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message,
                                       WPARAM wParam, LPARAM lParam)
    {
    int wmId, wmEvent;
    PAINTSTRUCT paint;
    TEXTMETRIC tm;
    HDC hdc;
    
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
        //case IDM_EXIT:
        //    DestroyWindow(hWnd);
        //    break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            }
        break;

    case WM_CREATE:
        initFont (FontSmallHeight, &smallOperFont);
        initFont (FontMediumHeight, &mediumOperFont);
        hdc = GetDC(hWnd);
        GetClientRect(hWnd, &rect);
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, black);
        SetTextColor(hdc, green);
        SelectObject(hdc, smallOperFont.boldId);
        GetTextMetrics(hdc, &tm);
        smallOperFont.width = tm.tmAveCharWidth;
        SelectObject(hdc, mediumOperFont.boldId);
        GetTextMetrics(hdc, &tm);
        mediumOperFont.width = tm.tmAveCharWidth;

        /*
        **  Initialize window
        */
        BitBlt(hdc, 0, 0, XSize, YSize, hdc, 0, 0, BLACKNESS);

        /*
        **  
        */
        ReleaseDC(hWnd, hdc);
        emulationActive = TRUE;
        return DefWindowProc (hWnd, message, wParam, lParam);

    case WM_DESTROY:
        freeFont (&smallOperFont);
        freeFont (&mediumOperFont);
        emulationActive = FALSE;
        PostQuitMessage(0);
        break;

        /*
        **  Paint the main window.
        */
    case WM_PAINT:
        BeginPaint (hWnd, &paint);
        opDisplay ();
        EndPaint (hWnd, &paint);
        break;

        /*
        **  Handle input characters.
        */
    case WM_CHAR:
        windowQueueKey (wParam);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        }

    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Get width of operator display font.
**
**  Parameters:     None.
**
**  Returns:        Width.
**
**------------------------------------------------------------------------*/
int windowGetOperFontWidth(int font)
    {
    if (font == FontSmall)
        {
        return smallOperFont.width;
        }
    else
        {
        return mediumOperFont.width;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Display a string
**
**  Parameters:     None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSendString (int x, int y, int font,
                       bool bold, const char *str, bool blank)
    {
    char cb[80];
    
    FontInfo *currentFontInfo;
    HDC hdc;
    
    if (hWnd == 0)
        {
        return;
        }
    hdc = GetDC(hWnd);
    SetBkMode(hdc, OPAQUE);
    SetBkColor(hdc, black);
    SetTextColor(hdc, green);
    if (font == FontSmall)
        {
        currentFontInfo = &smallOperFont;
        }
    else
        {
        currentFontInfo = &mediumOperFont;
        }
    if (bold)
        {
        SelectObject (hdc, currentFontInfo->boldId);
        }
    else
        {
        SelectObject (hdc, currentFontInfo->normalId);
        }

    if (blank)
        {
        memset (cb, ' ', 64);
        memcpy (cb, str, strlen (str));
        TextOut (hdc, XADJUST(x), YADJUST(y), cb, 64);
        }
    else
        {
        TextOut (hdc, XADJUST(x), YADJUST(y), str, strlen (str));
        }
    ReleaseDC(hWnd, hdc);
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

/*--------------------------------------------------------------------------
**  Purpose:        Window input check.
**
**  Parameters:     Name        Description.
**
**  Returns:        Key received, or 0.
**
**------------------------------------------------------------------------*/
int opWindowInput(void)
    {
    int nextget, key = 0;
    
    if (keyOut == keyIn)
        {
        /* Buffer is empty */
        return 0;
        }
    nextget = keyOut + 1;
    if (nextget == KeyBufSize)
        {
        nextget = 0;
        }
    key = keybuf[keyOut];
    keyOut = nextget;
    return key;
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Queue keyboard input.
**
**  Parameters:     Name        Description.
**                  ch          character to be queued.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowQueueKey(char ch)
    {
    int nextput;
    
    nextput = keyIn + 1;
    if (nextput == KeyBufSize)
        nextput = 0;
    if (nextput != keyOut)
        {
        keybuf[keyIn] = ch;
        keyIn = nextput;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Initialize a FontInfo struct.
**
**  Parameters:     Name        Description.
**                  size        pointsize of font
**                  fi          pointer to FontInfo struct to fill in
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void initFont (int size, FontInfo *fi)
    {
    LOGFONT lfTmp;

    memset(&lfTmp, 0, sizeof(lfTmp));
    lfTmp.lfPitchAndFamily = FIXED_PITCH;
    strcpy(lfTmp.lfFaceName, FontName);
    lfTmp.lfWeight = FW_THIN;
    lfTmp.lfOutPrecision = OUT_TT_PRECIS;
    lfTmp.lfHeight = size;
    fi->normalId = CreateFontIndirect (&lfTmp);
    if (!fi->normalId)
        {
        MessageBox (GetFocus(),
                    "Unable to get regular font", 
                    "CreateFont Error",
                    MB_OK);
        }
    memset(&lfTmp, 0, sizeof(lfTmp));
    lfTmp.lfPitchAndFamily = FIXED_PITCH;
    strcpy(lfTmp.lfFaceName, FontName);
    lfTmp.lfWeight = FW_BOLD;
    lfTmp.lfOutPrecision = OUT_TT_PRECIS;
    lfTmp.lfHeight = size;
    fi->boldId = CreateFontIndirect (&lfTmp);
    if (!fi->boldId)
        {
        MessageBox (GetFocus(),
                    "Unable to get bold font", 
                    "CreateFont Error",
                    MB_OK);
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Free fonts given a FontInfo struct.
**
**  Parameters:     Name        Description.
**                  fi          pointer to FontInfo struct to fill in
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void freeFont (FontInfo *fi)
    {
    if (fi->normalId)
        DeleteObject(fi->normalId);
    if (fi->boldId)
        DeleteObject(fi->boldId);
    }


/*---------------------------  End Of File  ------------------------------*/
