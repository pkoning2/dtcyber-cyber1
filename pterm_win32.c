/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, 2004, Tom Hunter, Paul Koning (see license.txt)
**
**  Name: pterm_win32.c
**
**  Description:
**      Simulate Plato IV terminal on Microsoft Windows.
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
#include "ptermversion.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define KeyBufSize      50
#define DisplayMargin   8

/*
**  Size of the window and pixmap.
**  This is: a screen high with marging top and botton.
**  Pixmap has two rows added, which are storage for the
**  patterns for loadable characters.
*/
#define XSize           (512 + 2 * DisplayMargin)
#define YSize           (512 + 2 * DisplayMargin)
#define YPMSize         (512 + 2 * DisplayMargin + 2 * 16)

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) ((x) + DisplayMargin)
#define YADJUST(y) (YSize - 1 - DisplayMargin - (y))

// inverse mapping (for processing touch input)
#define XUNADJUST(x) ((x) - DisplayMargin)
#define YUNADJUST(y) (YSize - 1 - DisplayMargin - (y))

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void ptermThread(LPVOID foo);
static ATOM ptermRegisterClass(HINSTANCE hInstance);
static BOOL ptermCreate(void);
static LRESULT CALLBACK ptermProcedure(HWND, UINT, WPARAM, LPARAM);
static void drawChar (HDC hdc, int x, int y, int snum, int cnum);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
extern int scale;
extern FILE *traceF;
extern char traceFn[];
extern bool tracePterm;
extern HINSTANCE hInstance;
extern bool emulationActive;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 wemode;       // local copy of most recently set wemode
static bool touchEnabled;
static HWND hWnd;
static HDC hdcMem;
static HBITMAP hbmMem, hbmOld;
static HDC hdcFont;
static RECT rect;
static COLORREF orange;
static COLORREF black;
static HPEN hPen = 0;
static HPEN hPenBg = 0;
static HBITMAP fontBitmap;
static bool allowClose = FALSE;
static HCURSOR hcNormal, hcTouch;
static char ptermWinName[100] = "Pterm" ;

/*
**  rasterop codes for a given W/E mode
**  See Win32 rasterops (ternary ops) appendix for explanations...
**             0=inverse, 1=rewrite, 2=erase, 3=write
*/
static const DWORD WeFunc[] = 
{ 0x003F00EA,   // PSan
  MERGECOPY,
  0x002A0CC9,   // DPSana
  0x00EA02E9,   // DPSao
};
static const DWORD MWeFunc[] = 
{ NOTSRCCOPY,
  SRCCOPY,
  0x00220326,   // DSna
  SRCPAINT,
};

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

void ptermSetName (const char *winName);
void ptermLoadChar (int snum, int cnum, const u16 *data);
void ptermSetWeMode (u8 we);
void ptermDrawChar (int x, int y, int snum, int cnum);
bool platoKeypress (WPARAM wParam, int alt, int stat);
bool platoTouch (LPARAM lParam, int stat);
extern void ptermComInit(void);
extern void niuLocalKey(u16 key, int stat);

/*--------------------------------------------------------------------------
**  Purpose:        Initialize the Plato terminal window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermInit(const char *winName, bool closeOk)
    {
    DWORD dwThreadId; 
    HANDLE hThread;

    allowClose = closeOk;                           // Remember whether to honor Ctrl/Z
    strcpy (ptermWinName, winName);

    /*
    **  Create windowing thread.
    */
    hThread = CreateThread( 
        NULL,                                       // no security attribute 
        0,                                          // default stack size 
        (LPTHREAD_START_ROUTINE) ptermThread, 
        (LPVOID) winName,                           // thread parameter 
        0,                                          // not suspended 
        &dwThreadId);                               // returns thread ID 

    if (hThread == NULL)
        {
        MessageBox(NULL, "pterm thread creation failed", "Error", MB_OK);
        exit(1);
        }


    /*
    **  Now do common init stuff
    */
    ptermComInit ();
    while (!emulationActive) ;      // spin until thread is done setting things up
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set window name
**
**  Parameters:     Name        Description.
**                  winName     name to set.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermSetName (const char *winName)
    {
    /*
    **  Set window title.
    */
    if (hWnd != 0)
        {
        SetWindowText (hWnd, winName);
        }
    strcpy (ptermWinName, winName);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Close window.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermClose(void)
    {
    if (!emulationActive)
        {
        return;
        }
    SendMessage(hWnd, WM_DESTROY, 0, 0);
    }


/*--------------------------------------------------------------------------
**  Purpose:        Windows thread.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void ptermThread(LPVOID foo)
    {
    MSG msg;

    /*
    **  Register the window class.
    */
    ptermRegisterClass(hInstance);

    /*
    **  Create the window.
    */
    if (!ptermCreate())
        {
        MessageBox(NULL, "pterm window creation failed", "Error", MB_OK);
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
ATOM ptermRegisterClass(HINSTANCE hInstance)
    {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)ptermProcedure;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_CONSOLE);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = (LPCSTR)IDC_CONSOLE;
    wcex.lpszClassName  = "PTERM";
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
static BOOL ptermCreate(void)
    {
    int dx, dy;

    dx = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    dy = GetSystemMetrics(SM_CYSIZEFRAME) * 2 +
         GetSystemMetrics(SM_CYCAPTION);
    hWnd = CreateWindow(
        "PTERM",                // Registered class name
        ptermWinName,           // window name
        WS_OVERLAPPEDWINDOW,    // window style
        CW_USEDEFAULT,          // horizontal position of window
        CW_USEDEFAULT,          // vertical position of window
        XSize * scale + dx,             // window width
        YSize * scale + dy,             // window height
        NULL,                   // handle to parent or owner window
        NULL,                   // menu handle or child identifier
        0,                      // handle to application instance
        NULL);                  // window-creation data

    if (!hWnd)
        {
        return FALSE;
        }
    ptermSetName (ptermWinName);
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
static LRESULT CALLBACK ptermProcedure(HWND hWnd, UINT message,
                                       WPARAM wParam, LPARAM lParam)
    {
    int wmId, wmEvent;
    HDC hdc;
    PAINTSTRUCT paint;
    int savemode;
    
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
        hdc = GetDC(hWnd);
        GetClientRect(hWnd, &rect);

        /*
        **  Create a compatible DC.
        */
        hdcMem = CreateCompatibleDC(hdc);
        hdcFont = CreateCompatibleDC(hdc);
        
        /*
        **  Create a bitmap big enough for our client rect and loadable chars bitmap.
        */
        hbmMem = CreateCompatibleBitmap(hdc, XSize, YPMSize);
        
        /*
        **  Select the bitmap into the off-screen dc.
        */
        hbmOld = SelectObject(hdcMem, hbmMem);
        
        /*
        **  Initialize bitmap and window
        */
        BitBlt(hdcMem, 0, 0, XSize, YPMSize, hdcMem, 0, 0, BLACKNESS);
        BitBlt(hdc, 0, 0, XSize * scale, YSize * scale, hdcMem, 0, 0, BLACKNESS);

        /*
        **  Load the bitmap for the fonts
        */
        fontBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_FONTBITMAP));
        if (fontBitmap == NULL)
            {
            fprintf (stderr, "Failed to load font bitmap");
            }

        /*
        **  Load the cursors
        */
        hcNormal = LoadCursor (NULL, MAKEINTRESOURCE (IDC_ARROW));
        hcTouch = LoadCursor (NULL, MAKEINTRESOURCE (IDC_CROSS));

        /*
        **  Select the bitmap into the font dc.
        */
        SelectObject(hdcFont, fontBitmap);
        black = RGB(0, 0, 0);
        orange = RGB(0xff, 0x90, 0);
        SetBkMode(hdcMem, OPAQUE);
        SetBkColor(hdcMem, black);
        SetTextColor(hdcMem, orange);
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, black);
        SetTextColor(hdc, orange);
        ReleaseDC(hWnd, hdc);
        hPen = CreatePen(PS_SOLID, 1, orange);
        if (!hPen)
            {
            MessageBox (GetFocus(),
                        "Unable to get orange pen", 
                        "CreatePen Error",
                        MB_OK);
            }
        hPenBg = CreatePen(PS_SOLID, 1, black);
        if (!hPen)
            {
            MessageBox (GetFocus(),
                        "Unable to get black pen", 
                        "CreatePen Error",
                        MB_OK);
            }
        emulationActive = TRUE;
        return DefWindowProc (hWnd, message, wParam, lParam);

    case WM_DESTROY:
        /*
        **  Done with off screen bitmap and dc.
        */
        emulationActive = FALSE;
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteObject(fontBitmap);
        DeleteDC(hdcFont);
        DeleteDC(hdcMem);
        if (hPen)
            {
            DeleteObject(hPen);
            }
        if (hPenBg)
            {
            DeleteObject(hPenBg);
            }
        PostQuitMessage(0);
        break;

        /*
        **  Paint the main window.
        */
    case WM_PAINT:
        BeginPaint (hWnd, &paint);
        hdc = GetDC(hWnd);
        StretchBlt (hdc,
                    rect.left, rect.top,
                    XSize * scale, YSize * scale,
                    hdcMem,
                    0, 0,
                    XSize, YSize,
                    SRCCOPY);

        ReleaseDC(hWnd, hdc);
        EndPaint (hWnd, &paint);
        break;

        /*
        **  Handle input characters.
        */
    case WM_CHAR:
        if (wParam == 032 && allowClose)
            {
            DestroyWindow(hWnd);
            }
        else if (wParam == 035) // control-] : trace
            {
            tracePterm = !tracePterm;
            savemode = wemode;
            if (!tracePterm)
                {
                wemode = 2;
                fflush (traceF);
                }
            else
                {
                if (traceF == NULL)
                    {
                    traceF = fopen (traceFn, "w");
                    }
                wemode = 3;
                }
            ptermSetWeMode (wemode);
            /*
            **  The 1024 is a strange hack to circumvent the
            **  screen edge wrap checking.
            */
            ptermDrawChar (1024 + 512, 512, 1, 024);
            wemode = savemode;
            ptermSetWeMode (wemode);
            return 0;
            }
        break;

    case WM_SYSCHAR:
        platoKeypress (wParam, 1, 1);
        break;

    case WM_KEYDOWN:
        platoKeypress (wParam, 0, 1);
        break;

    case WM_LBUTTONDOWN:
        platoTouch (lParam, 1);
        break;
        
    case WM_SYSKEYDOWN:
        /*
        **  F10 comes in as a syskey because it is used by Win keyboard
        **  conventions to access the menus.  We don't do that, just
        **  divert it to regular keyboard processing.
        */
        if (wParam == VK_F10)
            {
            platoKeypress (wParam, 0, 1);
            break;
            }

        /*
        **  ALT leftarrow is assignment arrow:
        */
        if (wParam == VK_LEFT)
            {
            platoKeypress (wParam, 1, 1);
            break;
            }

        /*
        **  Fall through to default action
        */
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        }

    return 0;
    }


/*--------------------------------------------------------------------------
**  Purpose:        Set W/E mode
**
**  Parameters:     Name        Description.
**                  we          mode byte
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermSetWeMode (u8 we)
    {
    wemode = we;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Draw one character
**
**  Parameters:     Name        Description.
**                  x           X coordinate to draw
**                  y           Y coordinate to draw
**                  snum        character set number
**                  cnum        character number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermDrawChar (int x, int y, int snum, int cnum)
    {
    HDC hdc;

    if (hWnd == 0)
        {
        return;
        }
    hdc = GetDC(hWnd);
    SelectObject(hdc, hPen);
    SelectObject(hdcMem, hPen);
    drawChar (hdc, x, y, snum, cnum);
    ReleaseDC (hWnd, hdc);
    }


/*--------------------------------------------------------------------------
**  Purpose:        Draw a point
**
**  Parameters:     Name        Description.
**                  x           X coordinate of point
**                  y           Y coordinate of point
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermDrawPoint (int x, int y)
    {
    HDC hdc;

    if (hWnd == 0)
        {
        return;
        }
    hdc = GetDC(hWnd);
    x = XADJUST (x);
    y = YADJUST (y);
    if (wemode & 1)
        {
        SetPixel (hdcMem, x, y, orange);
        x *= scale;
        y *= scale;
        SetPixel (hdc, x, y, orange);
        if (scale == 2)
            {
            SetPixel (hdc, x + 1, y, orange);
            SetPixel (hdc, x, y + 1, orange);
            SetPixel (hdc, x + 1, y + 1, orange);
            }
        }
    else
        {
        SetPixel (hdcMem, x, y, black);
        x *= scale;
        y *= scale;
        SetPixel (hdc, x, y, black);
        if (scale == 2)
            {
            SetPixel (hdc, x + 1, y, black);
            SetPixel (hdc, x, y + 1, black);
            SetPixel (hdc, x + 1, y + 1, black);
            }
        }
    ReleaseDC (hWnd, hdc);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Draw a line
**
**  Parameters:     Name        Description.
**                  x1          starting X coordinate of line
**                  y1          starting Y coordinate of line
**                  x2          ending X coordinate
**                  y2          ending Y coordinate
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermDrawLine(int x1, int y1, int x2, int y2)
    {
    HDC hdc;
    POINT line[2];
    int xx1, xx2, yy1, yy2, tx, ty, sx, sy;

    if (hWnd == 0)
        {
        return;
        }
    hdc = GetDC(hWnd);
    if (wemode & 1)
        {
        SelectObject(hdc, hPen);
        SelectObject(hdcMem, hPen);
        }
    else
        {
        SelectObject(hdc, hPenBg);
        SelectObject(hdcMem, hPenBg);
        }
    line[0].x = xx1 = XADJUST (x1);
    line[0].y = yy1 = YADJUST (y1);
    line[1].x = xx2 = XADJUST (x2);
    line[1].y = yy2 = YADJUST (y2);
    Polyline (hdcMem, line, 2);
    if (scale == 1)
        {
        Polyline (hdc, line, 2);
        }
    else
        {
        if (xx1 < xx2)
            {
            tx = xx1;
            sx = xx2 - xx1;
            }
        else
            {
            tx = xx2;
            sx = xx1 - xx2;
            }
        if (yy1 < yy2)
            {
            ty = yy1;
            sy = yy2 - yy1;
            }
        else
            {
            ty = yy2;
            sy = yy1 - yy2;
            }
        sx++;
        sy++;
        StretchBlt (hdc, tx * scale, ty * scale,
                    sx * scale, sy * scale,
                    hdcMem, tx, ty, sx, sy, SRCCOPY);
        }
    ReleaseDC (hWnd, hdc);
    /*
    **  Windows refuses to draw the endpoint of the line, so
    **  we'll do it the hard way.
    */
    ptermDrawPoint (x2, y2);
    }


/*--------------------------------------------------------------------------
**  Purpose:        Process Plato full screen erase.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**
**------------------------------------------------------------------------*/
void ptermFullErase (void)
    {
    HDC hdc;

    if (hWnd == 0)
        {
        return;
        }
    hdc = GetDC(hWnd);
    BitBlt(hdcMem, DisplayMargin, DisplayMargin, 512, 512, hdcMem, 0, 0, BLACKNESS);
    BitBlt(hdc, DisplayMargin * scale, DisplayMargin * scale, 
        512 * scale, 512 * scale, hdcMem, 0, 0, BLACKNESS);
    ReleaseDC (hWnd, hdc);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Write a (loadable set) character to the font storage 
**                  part of the pixmap
**
**  Parameters:     Name        Description.
**                  snum        character set number
**                  cnum        character number
**                  data        vector of 8 uint16s with column pattern
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermLoadChar (int snum, int cnum, const u16 *data)
    {
    int i, j;
    int x = cnum * 8;
    int y = YSize + (snum & 1) * 16 + 15;
    u16 col;

    BitBlt (hdcMem, x, y - 15, 8, 16, hdcMem, 0, 0, BLACKNESS);
    SelectObject(hdcMem, hPen);

    for (i = 0; i < 8; i++)
        {
        col = *data++;
        for (j = 0; j < 16; j++)
            {
            if (col & 1)
                {
                SetPixel (hdcMem, x, y - j, orange);
                }
            col >>= 1;
            }
        x = (x + 1) & 0777;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process XKeyEvent for Plato keyboard
**
**  Parameters:     Name        Description.
**                  wParam      Virtual Key for the keypress if alt==0,
**                              character code if alt==1.
**                  alt         1 if ALT was active, 0 otherwise
**                  stat        Station number
**
**  Returns:        TRUE if key event was a valid Plato keycode.
**
**------------------------------------------------------------------------*/
bool platoKeypress (WPARAM wParam, int alt, int stat)
    {
    SHORT keystate;
    int key;
    u8 shift = 0;
    int pc = -1;
    WORD buf[4];
    BYTE keystatebuf[256];
    bool ctrl = FALSE;

    keystate = GetKeyState (VK_CONTROL);
    if ((signed) keystate < 0)
        {
        ctrl = TRUE;
        }
    keystate = GetKeyState (VK_SHIFT);
    if ((signed) keystate < 0)
        {
        shift = 040;
        }
    if (alt)
        {
        if (wParam == VK_LEFT)
            {
            pc = 015 | shift;       // assignment arrow
            }
        else if (wParam > 0 && wParam < sizeof (altKeyToPlato))
            {
            pc = altKeyToPlato[wParam] | shift;
            }
        if (pc >= 0)
            {
            niuLocalKey (pc, stat);
            return TRUE;
            }
        }
    else
        {
        switch (wParam)
            {
        case ' ':
            pc = 0100;      // space
            break;
        case VK_BACK:
            pc = 023;       // erase
            break;
        case VK_RETURN:
            pc = 026;       // next
            break;
        case VK_HOME:
        case VK_F8:
            pc = 030;       // back
            break;
        case VK_PAUSE:
        case VK_F10:
            pc = 032;       // stop
            break;
        case VK_TAB:
            pc = 014;       // tab
            break;
        case VK_ADD:
            if (ctrl)
                {
                pc = 056;   // Sigma
                }
            else
                {
                pc = 016;   // +
                }
            break;
        case VK_SUBTRACT:
            if (ctrl)
                {
                pc = 057;   // Delta
                }
            else
                {
                pc = 017;   // -
                }
            break;
        case VK_MULTIPLY:
        case VK_DELETE:
            pc = 012;       // multiply sign
            break;
        case VK_DIVIDE:
        case VK_INSERT:
            pc = 013;       // divide sign
            break;
        case VK_LEFT:
            pc = 0101;      // a (left arrow)
            break;
        case VK_RIGHT:
            pc = 0104;      // d (right arrow)
            break;
        case VK_UP:
            pc = 0127;      // w (up arrow)
            break;
        case VK_DOWN:
            pc = 0130;      // x (down arrow)
            break;
        case VK_PRIOR:
            pc = 020;       // super
            break;
        case VK_NEXT:
            pc = 021;       // sub
            break;
        case VK_F3:
            pc = 034;       // square
            break;
        case VK_F2:
            pc = 022;       // ans
            break;
        case VK_F1:
        case VK_F11:
            pc = 033;       // copy
            break;
        case VK_F9:
            pc = 031;       // data
            break;
        case VK_F5:
            pc = 027;       // edit
            break;
        case VK_F4:
            pc = 024;       // micro/font
            break;
        case VK_F6:
            pc = 025;       // help
            break;
        case VK_F7:
            pc = 035;       // lab
            break;
        default:
            pc = -1;
            if (ctrl)
                {
                key = MapVirtualKey (wParam, 2);
                if (isalpha (key))
                    {
                    key &= 037;     // form control-letter key value
                    }
                else
                    {
                    if (key == ']')
                        {
                        /*
                        **  trace toggle was already handled
                        */
                        return 0;
                        }
                    /*
                    **  control-nonletter produces the shift of
                    **  what PLATO has on that keycap
                    */
                    shift = 040;
                    }
                if (key > 0 && key <= 127)
                    {
                    pc = asciiToPlato[key];
                    }
                }
            else
                {
                GetKeyboardState (keystatebuf);
                buf[0] = 0;
                if (ToAscii (wParam, 0, keystatebuf, buf, 0) == 1)
                    {
                    key = buf[0];
                    if (key > 0 && key <= 127)
                        {
                        pc = asciiToPlato[key];
                        }
                    shift = 0;      // shift is accounted for in lookup table
                    }
                }
            }
        if (pc >= 0)
            {
            pc |= shift;
            niuLocalKey (pc, stat);
            return TRUE;
            }
        }
    
    return FALSE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process mouse click for Plato terminal
**
**  Parameters:     Name        Description.
**                  lParam      Mouse click parameter word
**                  stat        Station number
**
**  Returns:        TRUE if mouse event was valid.
**
**------------------------------------------------------------------------*/
bool platoTouch (LPARAM lParam, int stat)
    {
    int x, y;
    
    if (!touchEnabled)
        {
        return FALSE;
        }
    x = XUNADJUST ((lParam & 0xffff) / scale);
    y = YUNADJUST ((lParam >> 16) / scale);
    
    if (x < 0 || x > 511 ||
        y < 0 || y > 511)
        {
        return FALSE;
        }
    x /= 32;
    y /= 32;

    niuLocalKey (0x100 | (x << 4) | y, stat);
    return TRUE;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Enable or disable "touch" input
**
**  Parameters:     Name        Description.
**                  enable      true or false for enable or disable.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void ptermTouchPanel(bool enable)
    {
    if (enable)
        {
        SetClassLong(hWnd, GCL_HCURSOR, (LONG) hcTouch);
        }
    else
        {
        SetClassLong(hWnd, GCL_HCURSOR, (LONG) hcNormal);
        }
    touchEnabled = enable;
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Draw a character
**
**  Parameters:     Name        Description.
**                  hdc         Device context
**                  x           X coordinate to draw
**                  y           Y coordinate to draw
**                  snum        character set number
**                  cnum        character number
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void drawChar (HDC hdc, int x, int y, int snum, int cnum)
    {
    int charX, charY;
    int xb = 8, yb = 16;
    int xoff = 0, yoff = 0;
    int yt = YADJUST (y) - 15;

    /*
    **  Clip path has bizarre side effects, so do clipping manually.
    **  If X is 1024 or more, that's the special internal "don't clip"
    **  marker used for displaying the Trace indicator.
    */
    if (x < 1024)
        {
        if (x > 512 - 8)
            {
            xb = 512 - x;
            }
        else if (x < 0)
            {
            xoff = -x;
            xb = 8 - xoff;
            x = 0;
            }
        if (yt > DisplayMargin + 512 - 16)
            {
            yb = 512 - (yt - DisplayMargin);
            }
        else if (yt < DisplayMargin)
            {
            yoff = DisplayMargin - yt;
            yb = 16 - yoff;
            yt = DisplayMargin;
            }
        }
    
    charX = cnum * 8;
    if (snum < 2)
        {
        /*
        **  "ROM" characters
        */
        charY = snum * 16;
        BitBlt (hdcMem, XADJUST (x & 1023), yt, xb, yb,
                hdcFont, charX + xoff, charY + yoff, WeFunc[wemode]);
        }
    else
        {
        charY = YSize + (snum - 2) * 16;
        BitBlt (hdcMem, XADJUST (x & 1023), yt, xb, yb,
                hdcMem, charX + xoff, charY + yoff, MWeFunc[wemode]);
        }
    StretchBlt(hdc, XADJUST (x & 1023) * scale, yt * scale,
               xb * scale, yb * scale,
               hdcMem, XADJUST (x & 1023), yt, 
               xb, yb, SRCCOPY);
    /*
    **  Handle screen edge wraparound by recursion...
    */
    if (x < 1024)
        {
        if (x > 512 - 8)
            {
            drawChar (hdc, x - 512, y, snum, cnum);
            }
        if (y > 512 - 16)
            {
            drawChar (hdc, x, y - 512, snum, cnum);
            }
        }
    }

/*---------------------------  End Of File  ------------------------------*/
