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

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define KeyBufSize	    50
#define DisplayMargin	20

// Size of the window and pixmap.
// This is: a screen high with marging top and botton.
// Pixmap has two rows added, which are storage for the
// patterns for loadable characters.
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
static void ptermThread(LPVOID winName);
static ATOM ptermRegisterClass(HINSTANCE hInstance);
static BOOL ptermCreate(char *winName);
static LRESULT CALLBACK ptermProcedure(HWND, UINT, WPARAM, LPARAM);
static void drawChar (HDC hdc, int x, int y, int snum, int cnum);

/*
**  ----------------
**  Public Variables
**  ----------------
*/

extern FILE *traceF;
extern char traceFn[];
extern bool tracePterm;
extern HINSTANCE hInstance;
extern volatile bool ptermActive;

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
static HPEN hPen = 0;
static HBITMAP fontBitmap;
static bool allowClose = FALSE;
static HCURSOR hcNormal, hcTouch;

// rasterop codes for a given W/E mode
// See Win32 rasterops (ternary ops) appendix for explanations...
//            0=inverse, 1=rewrite, 2=erase, 3=write
static const DWORD WeFunc[] = 
{ 0x003F00EA,	// PSan
  MERGECOPY,
  0x002A0CC9,	// DPSana
  0x00EA02E9,	// DPSao
};
static const DWORD MWeFunc[] = 
{ NOTSRCCOPY,
  SRCCOPY,
  0x00220326,	// DSna
  SRCPAINT,
};

// PLATO terminal clip region
static const RECT PlatoRect = 
{ XADJUST (0), YADJUST (0),
  XADJUST (511), YADJUST (511)
};

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

bool platoKeypress (WPARAM wParam, int alt, int stat);
bool platoTouch (LPARAM lParam, int stat);

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

	allowClose = closeOk;							// Remember whether to honor Ctrl/Z

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
	while (!ptermActive) ;		// spin until thread is done setting things up
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
    **  Set window and icon titles.
    */
//    XSetStandardProperties (disp, ptermWindow, winName, "Pterm",
//                            None, NULL, 0, NULL);
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
    if (!ptermActive)
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
static void ptermThread(LPVOID winName)
{
    MSG msg;

    /*
    **  Register the window class.
    */
    ptermRegisterClass(hInstance);

    /*
    **  Create the window.
    */
    if (!ptermCreate((char *) winName)) 
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
static BOOL ptermCreate(char *winName)
{
    int dx, dy;

    dx = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    dy = GetSystemMetrics(SM_CYSIZEFRAME) * 2 +
         GetSystemMetrics(SM_CYCAPTION);
    hWnd = CreateWindow(
        "PTERM",                // Registered class name
        winName,                // window name
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
		orange = RGB(0xff, 0x90, 0);
        SetBkMode(hdcMem, OPAQUE);
        SetBkColor(hdcMem, RGB(0, 0, 0));
        SetTextColor(hdcMem, orange);
		SetBoundsRect(hdcMem, &PlatoRect, DCB_RESET | DCB_ACCUMULATE);
        SetBkMode(hdc, OPAQUE);
        SetBkColor(hdc, RGB(0, 0, 0));
        SetTextColor(hdc, orange);
		SetBoundsRect(hdc, &PlatoRect, DCB_RESET | DCB_ACCUMULATE);
        ReleaseDC(hWnd, hdc);
        hPen = CreatePen(PS_SOLID, 1, orange);
        if (!hPen)
        {
            MessageBox (GetFocus(),
                "Unable to get orange pen", 
                "CreatePen Error",
                MB_OK);
        }
	    ptermActive = TRUE;
        return DefWindowProc (hWnd, message, wParam, lParam);

    case WM_DESTROY:
        /*
        **  Done with off screen bitmap and dc.
        */
	    ptermActive = FALSE;
        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteObject(fontBitmap);
        DeleteDC(hdcFont);
        DeleteDC(hdcMem);
        if (hPen)
        {
            DeleteObject(hPen);
        }
        PostQuitMessage(0);
        break;

        /*
        **  Paint the main window.
        */
    case WM_PAINT:
        BeginPaint (hWnd, &paint);
        hdc = GetDC(hWnd);
        BitBlt(hdc, 
               rect.left, rect.top,
               XSize, YSize,
               hdcMem,
               0, 0,
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
                        // The 1024 is a strange hack to circumvent the
                        // screen edge wrap checking.
                        ptermDrawChar (1024 + 512, 512, 1, 024);
                        wemode = savemode;
                        ptermSetWeMode (wemode);
                        return;
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

	hdc = GetDC(hWnd);
    SetPixel (hdcMem, XADJUST (x), YADJUST (y), orange);
    SetPixel (hdc, XADJUST (x), YADJUST (y), orange);
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

	hdc = GetDC(hWnd);
    SelectObject(hdc, hPen);
    SelectObject(hdcMem, hPen);
    MoveToEx (hdcMem, XADJUST (x1), YADJUST (y1), NULL);
    LineTo (hdcMem, XADJUST (x2), YADJUST (y2));
    MoveToEx (hdc, XADJUST (x1), YADJUST (y1), NULL);
    LineTo (hdc, XADJUST (x2), YADJUST (y2));
	ReleaseDC (hWnd, hdc);
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

	hdc = GetDC(hWnd);
    BitBlt(hdcMem, DisplayMargin, DisplayMargin, 512, 512, hdcMem, 0, 0, BLACKNESS);
    BitBlt(hdc, DisplayMargin, DisplayMargin, 512, 512, hdcMem, 0, 0, BLACKNESS);
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
**								character code if alt==1.
**					alt			1 if ALT was active, 0 otherwise
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
        if (wParam > 0 && wParam < sizeof (altKeyToPlato))
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
                pc = 056;	// Sigma
            }
            else
            {
                pc = 016;   // +
            }
            break;
        case VK_SUBTRACT:
            if (ctrl)
            {
                pc = 057;	// Delta
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
            pc = 015;       // assignment arrow
            break;
        case VK_UP:
            pc = 020;       // super
            break;
        case VK_DOWN:
            pc = 021;       // sub
            break;
        case VK_F3:
            pc = 034;       // square
            break;
        case VK_F2:
            pc = 022;       // ans
            break;
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
            GetKeyboardState (keystatebuf);
            buf[0] = 0;
            if (ToAscii (wParam, 0, keystatebuf, buf, 0) == 1)
            {
                key = buf[0];
                if (key > 0 && key <= 127)
                {
                    pc = asciiToPlato[key];
                    if (!ctrl)
                    {
                        shift = 0;		// shift is accounted for in lookup table
                    }
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
    x = XUNADJUST (lParam & 0xffff);
    y = YUNADJUST (lParam >> 16);
    
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
    
    if (x >= 1024)
    {
        // Special flag coordinate to write to the status field
		SetBoundsRect(hdc, NULL, DCB_RESET);
		SetBoundsRect(hdcMem, NULL, DCB_RESET);
    }
    
    charX = cnum * 8;
    if (snum < 2)
    {
        // "ROM" characters
        charY = snum * 16;
        BitBlt (hdcMem, XADJUST (x & 1023), YADJUST (y) - 15, 8, 16,
                hdcFont, charX, charY, WeFunc[wemode]);
    }
    else
    {
        charY = YSize + (snum - 2) * 16;
        BitBlt (hdcMem, XADJUST (x & 1023), YADJUST (y) - 15, 8, 16,
                hdcMem, charX, charY, MWeFunc[wemode]);
    }
	BitBlt(hdc, XADJUST (x & 1023), YADJUST (y)- 15, 8, 16,
		   hdcMem, XADJUST (x & 1023), YADJUST (y) - 15, SRCCOPY);
    // Handle screen edge wraparound by recursion...
    if (x >= 1024)
    {
        // Restore normal clipping
		SetBoundsRect(hdc, &PlatoRect, DCB_RESET | DCB_ACCUMULATE);
		SetBoundsRect(hdcMem, &PlatoRect, DCB_RESET | DCB_ACCUMULATE);
    }
    else 
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
