/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
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
#include "dd60.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define LineBufSize         64
#define KeyBufSize          50

#define TIMER_ID            1
#define TIMER_RATE          100

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
typedef struct fontInfo
    {
    HFONT           normalId;       /* horizontal position */
    HFONT           boldId;         /* size of font */
    int             width;          /* character width in pixels */
    } FontInfo;

/* Data for buffered text in lineBuf */
typedef struct
    {
    char            c;              /* Character to display */
    u8              hits;           /* Number of repeats for this char */
    } CharData;

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/
static void windowThread(void);
ATOM windowRegisterClass(HINSTANCE hInstance);
BOOL windowCreate(void);
LRESULT CALLBACK windowProcedure(HWND, UINT, WPARAM, LPARAM);
static void initFont (int size, FontInfo *fi);
static void freeFont (FontInfo *fi);
static void windowStoreChar (HDC hdcMem, char c, int x, int y, int dx);
static void windowShowLine (HDC hdcMem, int dx);
static void windowTextPlot(HDC hdc, int xPos, int yPos, char ch, u8 fontSize);

/*
**  ----------------
**  Public Variables
**  ----------------
*/
HBITMAP fontBitmap;
HINSTANCE hInstance;
bool keyboardTrue;

/*
**  -----------------
**  Private Variables
**  -----------------
*/
static u8 currentFont;
static FontInfo *currentFontInfo;
static i16 currentX = -1;
static i16 currentY = -1;
static CharData lineBuf[LineBufSize];
static int lineBufCnt, xpos, xstart, ypos;
static char keybuf[KeyBufSize+4];
volatile static u32 keyIn, keyOut;
static HWND hWnd;
static FontInfo smallFont;
static FontInfo mediumFont;
static FontInfo largeFont;
static FontInfo smallOperFont;
static FontInfo mediumOperFont;
static bool keyboardSendUp;
static HDC hdcMem;
static HBITMAP hbmMem, hbmOld;
static HDC hdc;
static HDC hdcFont;
static HBRUSH hBrush;
static RECT rect;
static u32 s1,s2;
static u32 s1list[MaxPolls], s2list[MaxPolls];
static int listPutsAtGetChar[MaxPolls];
static int sumListGet, sumListPut;
static bool displayOff = FALSE;
static const i8 dotdx[] = { 0, 1, 0, 1, -1, -1,  0, -1,  1 };
static const i8 dotdy[] = { 0, 0, 1, 1, -1,  0, -1,  1, -1 };
//static XKeyboardControl kbPrefs;
#if CcHersheyFont == 1
static HPEN hPen = 0;
#endif

/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/


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
    **  Get our instance
    */
    hInstance = GetModuleHandle(NULL);

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

    /*
    **  Initialize the widths for the bitmap fonts
    */
    smallFont.width = 8;
    mediumFont.width = 16;
    largeFont.width = 32;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set keyboard emulation to "true" or "easy".
**
**  Parameters:     TRUE for accurate, FALSE for easy.
**
**  Returns:        nothing.
**
**------------------------------------------------------------------------*/
void windowSetKeyboardTrue (bool flag)
    {
    keyboardTrue = flag;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Set X coordinate.
**
**  Parameters:     Name        Description.
**                  x           horizontal coordinate (0 - 0777)
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
**                  y           vertical coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetY(u16 y)
    {
    currentY = 0777 - y;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process a Set X command
**
**  Parameters:     Name        Description.
**                  mode        New mode position
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetMode (int mode)
    {
    int newFont, newOffset;
    
    /*
    **  Handle left screen/right screen selection
    */
    if (mode & Dd60ScreenR)
        {
        newOffset = OffRightScreen;
        }
    else
        {
        newOffset = OffLeftScreen;
        }

    if (newOffset != currentXOffset)
        {
        /*
        **  Send out any buffered line of text.
        */
        windowShowLine ();
        currentXOffset = newOffset;
        }
        
    /*
    **  Setup new font if necessary.
    */
    switch (mode & 3)
        {
    case Dd60CharSmall:
        newFont = FontSmall;
        break;
    case Dd60CharMedium:
        newFont = FontMedium;
        break;
    case Dd60CharLarge:
        newFont = FontLarge;
        break;
    case Dd60Dot:
        newFont = FontDot;
        break;
        }

        if (currentFont != newFont)
        {
            windowShowLine ();
            currentFont = newFont;
            switch (newFont)
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
    }

/*--------------------------------------------------------------------------
**  Purpose:        Process a byte of character data
**
**  Parameters:     Name        Description.
**                  ch          character (display code)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowProcessChar (int ch)
    {
    /*
    **  If in dot mode, or no set mode received yet, just ignore the char
    */
    if (currentFont <= 0)
        {
        return;
        }
    if (hersheyMode)
        {
        windowTextPlot(XADJUST (currentX), YADJUST (currentY),
                       ch, currentFont);
        }
    else
        {
        windowStoreChar (ch, currentX, currentY,
                         currentFont);
        }
    currentX += currentFont;
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
int windowInput(void)
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
    if (!keyboardTrue)
        {
        // We're not doing the precise emulation, instead doing
        // regular key rollover, so ignore key up events.
        if (key & 0200)
            {
            key = 0;
            }
        }
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
void windowQueueKey(char ch)
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
**  Purpose:        Flush pending characters to the bitmap.
**
**  Parameters:     Name        Description.
**                  hdcMem      bitmap context
**                  dx          current font size
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowShowLine (HDC hdcMem, int dx)
    {
    int i;
    int x, y;
    bool bold = FALSE;
    int fontY, fy, boldoff, yshift, ymask;
    char str[2];

    switch (dx)
        {
    case 8:
        fontY = 32;
        boldoff = 8;
        yshift = 6;
        ymask = 077;
        break;
    case 16:
        fontY = 48;
        boldoff = 32;
        yshift = 5;
        ymask = 037;
        break;
    case 32:
        fontY = 112;
        boldoff = 128;
        yshift = 4;
        ymask = 017;
        break;
        }
    fy = fontY;
    str[1] = '\0';
    x = XADJUST (xstart);
    y = YADJUST (ypos);
    for (i = 0; i < lineBufCnt; i++)
        {
        if (lineBuf[i].hits >= (dx / 2) - 1)
            {
            if (!bold)
                {
                bold = TRUE;
                fy = fontY + boldoff;
                }
            }
        else
            {
            if (bold)
                {
                bold = FALSE;
                fy = fontY;
                }
            }
        if (lineBuf[i].c != 0)
            {
            BitBlt (hdcMem, x, y, dx, dx, hdcFont,
                    dx * (lineBuf[i].c & ymask),
                    fy + (dx * ((lineBuf[i].c & ~ymask) >> yshift)),
                    0x00EA02E9);
            }
        x += dx;
        }

    lineBufCnt = 0;
    xstart = 07777;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Put a character to the display, buffering a line
**                  at a time so we can do bold handling etc.
**
**  Parameters:     Name        Description.
**                  hdcMem      bitmap context
**                  c           character (ASCII)
**                  x           x position
**                  y           y position
**                  dx          current font size
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowStoreChar (HDC hdcMem, char c, int x, int y, int dx)
    {
    int dindx;
    
    // Center the character on the supplied x/y.
    x -= dx / 2;
    y -= dx / 2;
    
    // Count hits on this position (for intensify)
    dindx = (x - xstart) / dx;
    if (y == ypos &&
        x < xpos && x >= xstart &&
        dindx * dx == x - xstart &&
        lineBuf[dindx].c == c)
        {
        ++lineBuf[dindx].hits;
        return;
        }
    if (lineBufCnt == DisplayBufSize ||
        y != ypos ||
        x < xpos || 
        dindx >= DisplayBufSize ||
        dindx * dx != x - xstart)
        {
        windowShowLine (hdcMem, dx);
        xpos = xstart = x;
        ypos = y;
        }
    /*
    ** If we're skipping to a spot further down this line,
    ** space fill the range in between.
    */
    for ( ; xpos < x; xpos += dx)
        {
        lineBuf[lineBufCnt].hits = 1;
        lineBuf[lineBufCnt++].c = 0;
        }
    
    lineBuf[lineBufCnt].hits = 1;
    lineBuf[lineBufCnt++].c = c;
    xpos += dx;
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
    int dx, dy;

    dx = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    dy = GetSystemMetrics(SM_CYSIZEFRAME) * 2 +
         GetSystemMetrics(SM_CYCAPTION);
    hWnd = CreateWindow(
        "CONSOLE",              // Registered class name
        DtCyberVersion,         // window name
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
    u32 nextput;
    TEXTMETRIC tm;
    HDC hdc;
    SHORT keystate;
    BYTE keystatebuf[256];
    WORD buf[4];
    UINT i;

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
        initFont (FontSmallHeight, &smallOperFont);
        initFont (FontMediumHeight, &mediumOperFont);
        hdc = GetDC(hWnd);
        GetClientRect(hWnd, &rect);
        SelectObject(hdc, smallOperFont.boldId);
        GetTextMetrics(hdc, &tm);
        smallOperFont.width = tm.tmAveCharWidth;
        SelectObject(hdc, mediumOperFont.boldId);
        GetTextMetrics(hdc, &tm);
        mediumOperFont.width = tm.tmAveCharWidth;
        
        /*
        **  Create a compatible DC.
        */
        hdcMem = CreateCompatibleDC(hdc);
        hdcFont = CreateCompatibleDC(hdc);
        
        /*
        **  Create a bitmap big enough for our client rect,
        **  and initialize it.
        */
        hbmMem = CreateCompatibleBitmap(hdc, XSize, YSize);
        BitBlt(hdcMem, 
               0, 0, DefWinWidth, DefWinHeight,
               hdcMem,
               0, 0,
               BLACKNESS);
        
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
        
        SetBkMode(hdcMem, OPAQUE);
        SetBkColor(hdcMem, RGB(0, 0, 0));
        SetTextColor(hdcMem, RGB(0, 255, 0));
        ReleaseDC(hWnd, hdc);
        hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
        if (!hPen)
            {
            MessageBox (GetFocus(),
                        "Unable to get green pen", 
                        "CreatePen Error",
                        MB_OK);
            }
        return DefWindowProc (hWnd, message, wParam, lParam);

    case WM_DESTROY:
        freeFont (&smallFont);
        freeFont (&mediumFont);
        /*
        **  Done with off screen bitmap and dc.
        */
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

    case WM_TIMER:
        windowDisplay(hWnd);
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
            debugDisplay |= (traceMask != 0);
            traceStop ();
            break;

        case 'c':
            traceMask ^= TraceCpu0;
            debugDisplay |= (traceMask != 0);
            traceStop ();
            break;

        case 'C':
            traceMask ^= TraceCpu1;
            debugDisplay |= (traceMask != 0);
            traceStop ();
            break;

        case 'E':
        case 'e':
            traceMask ^= TraceEcs;
            debugDisplay |= (traceMask != 0);
            traceStop ();
            break;

        case 'J':
        case 'j':
            traceMask ^= TraceXj;
            debugDisplay |= (traceMask != 0);
            traceStop ();
            break;

        case 'X':
        case 'x':
            if (traceMask == 0 && chTraceMask == 0)
                {
                traceMask = ~0;
                debugDisplay = TRUE;
                }
            else
                {
                traceMask = 0;
                chTraceMask = 0;
                traceStop ();
                }
            break;

        case 'q':
            displayOff = FALSE;
            break;

        case 's':
            displayOff = TRUE;
            break;

        case 't':
            // this is useful in Plato mode when Alt-S and Alt-Q
            // have other meanings.
            displayOff = !displayOff;
            break;
            }
        break;

    case WM_KEYUP:
        if (!keyboardTrue)
            {
            return 0;
            }
/* fall through */
    case WM_KEYDOWN:
/* 
 * Translate the key to display code
 * Ignore control keys
 */
        keystate = GetKeyState (VK_CONTROL);
        if ((signed) keystate < 0)
            {
/* Ignore control keys */
            return 0;
            }
        GetKeyboardState (keystatebuf);
        buf[0] = 0;
        if (ToAscii (wParam, 0, keystatebuf, buf, 0) == 1)
            {
            i = buf[0];
            if (i != 0 && i <= 127)
                {
                if ((lParam & (1 << 31)) != 0)
                    {
/* this is a "key up" message */
                    i |= 0200;
                    }
//				printf ("keycode %03o lparam %x\n", i, lParam);
                windowQueueKey (i);
                }
            }
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        }

    return 0;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Show generated display data on the window.
**
**  Parameters:     None.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowShowDisplay (void)
    {
    PAINTSTRUCT ps;
    HFONT hfntOld;

    BeginPaint(hWnd, &ps);
    hdc = GetDC(hWnd);

    /*
    **  Output any pending data from the line buffer
    */
    windowShowLine (hdcMem, currentFontInfo->width);

    /*
    **  Blit the changes to the screen dc.
    */
    BitBlt(hdc, 
           rect.left, rect.top,
           XSize, YSize,
           hdcMem,
           0, 0,
           SRCCOPY);

    /*
    **  Make a clean bitmap for next block of data
    */
    BitBlt(hdcMem, 
           0, 0, DefWinWidth, DefWinHeight,
           hdcMem,
           0, 0,
           BLACKNESS);

    ReleaseDC(hWnd, hdc);
    EndPaint(hWnd, &ps);
    }


/*--------------------------------------------------------------------------
**  Purpose:        Plot a character using the Hershey glyphs.
**
**  Parameters:     Name        Description.
**                  hWnd        window handle.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void windowTextPlot(HDC hdc, int xPos, int yPos, char ch, u8 fontSize)
    {
    bool penDown = FALSE;
    const unsigned char *glyph;
    int x, y;
    int charSize = fontSize / 8;
    
    xPos += 8;
    glyph = (const unsigned char *)(consoleHersheyGlyphs[ch]);

    if (*glyph != '\0') /* nonempty glyph */
        {
        glyph += 2;

        while (*glyph)
            {
            x = (int)glyph[0];

            if (x == (int)' ')
                penDown = FALSE;
            else
                {
                x = charSize * (x - (int)'R') + xPos;
                y = charSize * ((int)glyph[1] - (int)'R') + yPos;
                if (penDown)
                    {
                    LineTo(hdc, x, y);  
                    }
                else
                    {
                    MoveToEx(hdc, x, y, NULL);  
                    }

                penDown = TRUE;
                }

            glyph += 2; /* on to next pair */
            }
        }
    }

/*---------------------------  End Of File  ------------------------------*/
