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
#define KeyBufSize			50	// MUST be even
#define DisplayBufSize		64
#define DisplayMargin		20
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

#define XADJUST(x) ((x) * ScaleX / 10 + DisplayMargin)
#define YADJUST(y) ((y) + ScaleY / 10 + DisplayMargin)

#define DefWinWidth			(XADJUST (02020) + DisplayMargin)
#define DefWinHeight		(YADJUST (512) + DisplayMargin)

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
static void windowThread(void);
ATOM windowRegisterClass(HINSTANCE hInstance);
BOOL windowCreate(void);
LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);
static void windowDisplay(HWND hWnd);
static void initFont (int size, FontInfo *fi);
static void freeFont (FontInfo *fi);
static void dput (HDC hdcMem, char c, int x, int y, int dx);
static void dflush (HDC hdcMem, int dx);

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
static FontInfo *currentFontInfo;
static i16 currentX = -1;
static i16 currentY = -1;
static DispList display[ListSize];
static int listGet, listPut, prevPut, listPutAtGetChar;
static char keybuf[KeyBufSize];
static u32 keyListPut, keyListGet;
static char dchars[DisplayBufSize];
static u8 dhits[DisplayBufSize];
static int dcnt, xpos, xstart, ypos;
static HWND hWnd;
static FontInfo smallFont;
static FontInfo mediumFont;
static FontInfo largeFont;

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
    listGet = listPut = 0;
    listPutAtGetChar = -1;

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
    int nextput;

    if (   currentX == -1
        || currentY == -1)
        {
        return;
        }

    if (ch != 0)
        {
        nextput = listPut + 1;
        if (nextput == ListSize)
            nextput = 0;
        if (nextput != listGet)
			{
			elem = display + listPut;
			listPut = nextput;
			elem->ch = ch;
			elem->fontSize = currentFont;
			elem->xPos = currentX;
			elem->yPos = currentY;
			}
        }

    currentX += currentFont;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Check whether it's time to do output
**
**  Parameters:     Name        Description.
**                  None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowCheckOutput(void)
    {
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
    int nextget;

    // We treat a keyboard poll as the end of a display refresh cycle.
    listPutAtGetChar = listPut;
    windowCheckOutput();
    
    if (keyListGet == keyListPut)
	return;

    nextget = keyListGet + 1;
    if (nextget == KeyBufSize)
	nextget = 0;
    ppKeyIn = keybuf[keyListGet];
    keyListGet = nextget;
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
**  Purpose:        Flush pending characters to the bitmap.
**
**  Parameters:     Name        Description.
**					hdcMem		bitmap context
**					dx			current font size
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dflush (HDC hdcMem, int dx)
    {
	int i;
	int x, y;
	bool bold = FALSE;
	char str[2];

	str[1] = '\0';
	x = XADJUST (xstart);
	y = YADJUST (ypos);
	SelectObject(hdcMem, currentFontInfo->normalId);
	for (i = 0; i < dcnt; i++)
		{
		if (dhits[i] >= dx / 2)
			{
			if (!bold)
				{
                SelectObject(hdcMem, currentFontInfo->boldId);
				bold = TRUE;
				}
			}
		else
			{
			if (bold)
				{
                SelectObject(hdcMem, currentFontInfo->normalId);
				bold = FALSE;
				}
			}
		str[0] = dchars[i];
	    TextOut(hdcMem, x, y, str, 1);
		x += dx;
		}
    dcnt = 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Put a character to the display, buffering a line
**					at a time so we can do bold handling etc.
**
**  Parameters:     Name        Description.
**					hdcMem		bitmap context
**					c			character (ASCII)
**					x			x position
**					y			y position
**					dx			current font size
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dput (HDC hdcMem, char c, int x, int y, int dx)
    {
    int dindx = (x - xstart) / dx;
    
    // Count hits on this position (for intensify)
    if (y == ypos &&
        x < xpos && x >= xstart &&
            dindx * dx == x - xstart &&
            dchars[dindx] == c)
        {
        ++dhits[dindx];
        return;
        }
    if (dcnt == DisplayBufSize ||
        y != ypos ||
        x < xpos || 
        dindx >= DisplayBufSize ||
        dindx * dx != x - xstart)
        {
        dflush (hdcMem, dx);
        xpos = xstart = x;
        ypos = y;
        }
    /*
    ** If we're skipping to a spot further down this line,
    ** space fill the range in between.
    */
    for ( ; xpos < x; xpos += dx)
        {
        dhits[dcnt] = 1;
        dchars[dcnt++] = ' ';
        }
    
    dhits[dcnt] = 1;
    dchars[dcnt++] = c;
    xpos += dx;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Initialize a FontInfo struct.
**
**  Parameters:     Name        Description.
**					size		pointsize of font
**					fi			pointer to FontInfo struct to fill in
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void initFont (int size, FontInfo *fi)
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
			"Unable to get regular font", 
			"CreateFont Error",
			MB_OK);
	}
}

/*--------------------------------------------------------------------------
**  Purpose:        Free fonts given a FontInfo struct.
**
**  Parameters:     Name        Description.
**					fi			pointer to FontInfo struct to fill in
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void freeFont (FontInfo *fi)
{
	if (fi->normalId)
		DeleteObject(fi->normalId);
	if (fi->boldId)
		DeleteObject(fi->boldId);
}

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
        CW_USEDEFAULT,          // vertical position of window
        DefWinWidth,            // window width
        DefWinHeight,           // window height
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
    RECT rt;
    u32 nextput;

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
		initFont (FontSmallHeight, &smallFont);
		initFont (FontMediumHeight, &mediumFont);
		initFont (FontLargeHeight, &largeFont);
        return DefWindowProc (hWnd, message, wParam, lParam);

    case WM_DESTROY:
        freeFont (&smallFont);
        freeFont (&mediumFont);
        freeFont (&largeFont);
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
		nextput = keyListPut + 2;
		if (nextput == KeyBufSize)
			nextput = 0;
		if (nextput != keyListGet)
		{
			keybuf[keyListPut] = wParam;
			/*
			** Stick a null after the real character 
			** to represent "key up" after the key down.
			** Without this, NOS DSD loses many keystrokes.
			*/
			keybuf[keyListPut + 1] = 0;
			keyListPut = nextput;
		}
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

    currentFontInfo = &smallFont;
    hfntOld = SelectObject(hdcMem, smallFont.normalId);
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

	if (listPutAtGetChar >= 0)
		{
		prevPut = listPutAtGetChar;
		}
	else
		{
		prevPut = listPut;
		}
    end = display + prevPut;
    curr = display + listGet;
    for (;;)
        {
        /*
        **  Check for wrap and done
        */
        if (curr == display + ListSize)
            curr = display;
        if (curr == end)
            break;

        if (oldFont != curr->fontSize)
            {
			dflush (hdcMem, oldFont);
            oldFont = curr->fontSize;

            switch (oldFont)
                {
            case FontSmall:
                currentFontInfo = &smallFont;
                break;

            case FontMedium:
                currentFontInfo = &mediumFont;
                break;
    
            case FontLarge:
                currentFontInfo = &largeFont;
                break;
                }
            }

        if (curr->fontSize == FontDot)
            {
            SetPixel(hdcMem, XADJUST (curr->xPos), YADJUST (curr->yPos) + 10, RGB(0, 255, 0));
            }
        else
            {
			dput (hdcMem, curr->ch, curr->xPos, curr->yPos, oldFont);
            }
        curr++;
        }

    dflush (hdcMem, oldFont);
    listGet = end - display;
    listPutAtGetChar = -1;
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
