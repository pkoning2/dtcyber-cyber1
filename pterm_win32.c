/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003, Tom Hunter, Paul Koning (see license.txt)
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

extern bool tracePterm;
extern HINSTANCE hInstance;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static volatile bool ptermActive = FALSE;
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

	allowClose = closeOk;							// Remember whether to honor Ctrl/D

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
	SHORT keystate;
    PAINTSTRUCT paint;
    
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
		if (allowClose && wParam == '\004')
		{
            DestroyWindow(hWnd);
		}
        break;

    case WM_SYSCHAR:
        platoKeypress (wParam, 1, 1);
        break;

	case WM_KEYDOWN:
		/* Ignore control keys */
		keystate = GetKeyState (VK_CONTROL);
		if ((signed) keystate < 0)
        {
			/* Ignore control keys */
			return 0;
        }
        platoKeypress (wParam, 0, 1);
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
            pc = 030;       // back
            break;
        case VK_PAUSE:
            pc = 032;       // stop
            break;
        case VK_TAB:
            pc = 014;       // tab
            break;
        case VK_ADD:
            pc = 016;       // +
            break;
        case VK_SUBTRACT:
            pc = 017;       // -
            break;
        case VK_MULTIPLY:
            pc = 012;       // multiply sign
            break;
        case VK_DIVIDE:
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
        default:
			GetKeyboardState (keystatebuf);
			if (ToAscii (wParam, 0, keystatebuf, buf, 0) == 1)
			{
				key = buf[0];
	            if (key > 0 && key <= 127)
		        {
			        pc = asciiToPlato[key];
					shift = 0;		// shift is accounted for in lookup table
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
//        XDefineCursor (disp, ptermWindow, curs);
    }
    else
    {
//        XUndefineCursor (disp, ptermWindow);
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
