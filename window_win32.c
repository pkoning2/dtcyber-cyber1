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

/*
**  -----------------
**  Private Constants
**  -----------------
*/
#define ListSize            10000
#define KeyBufSize          50
#define MaxPolls			10      // number of poll cycles we track
#define DisplayBufSize      64
#define DisplayMargin       20
// These are used only for the operator interface font
#define FontName            "Lucida Console"
#define FontSmallHeight     12
#define FontMediumHeight    17

#define TIMER_ID        1
#define TIMER_RATE      100

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/

#define XADJUST(x) ((x) + DisplayMargin)
#define YADJUST(y) ((y) + DisplayMargin)

#define DefWinWidth         (XADJUST (02020) + DisplayMargin)
#define DefWinHeight        (YADJUST (512) + DisplayMargin)

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
static void sum (u16 x);
#if CcHersheyFont == 1
static void windowTextPlot(HDC hdc, int xPos, int yPos, char ch, u8 fontSize);
#endif

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
static DispList display[ListSize];
static int listGet, listPut, prevPut, listPutAtGetChar;
static char keybuf[KeyBufSize+4];
volatile static u32 keyListPut, keyListGet;
static char dchars[DisplayBufSize];
static u8 dhits[DisplayBufSize];
static int dcnt, xpos, xstart, ypos;
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
static bool platoActive;
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

/* These functions are declared extern here rather than in proto.h,
** otherwise proto.h would have to #include X.h...
*/
extern bool platoKeypress (WPARAM wParam, int alt, int stat);

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
    sumListGet = sumListPut = 0;
    s1 = s2 = 0;
    
    /*
    **  Initialize the input list
    */
    keyListGet = keyListPut = 0;

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
**  Purpose:        Get width of medium operator display font.
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
**                  x           horizontal coordinate (0 - 0777)
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowSetX(u16 x)
    {
    currentX = x;
    sum (x);
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
    sum (y);
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
        || currentY == -1
        || currentFont == -1)
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
    if (GetQueueStatus(QS_ALLEVENTS) != 0)
        {
        WaitMessage();
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Report state of keyboard buffer.
**
**  Parameters:     Name        Description.
**
**  Returns:        TRUE if input pending, FALSE if not.
**
**------------------------------------------------------------------------*/
bool windowTestKeybuf (void)
    {
    return !(keyListGet == keyListPut);
    }

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
    
    nextput = keyListPut + 1;
    if (nextput == KeyBufSize)
        nextput = 0;
    if (nextput != keyListGet)
        {
        keybuf[keyListPut] = ch;
        keyListPut = nextput;
        }
    }

/*--------------------------------------------------------------------------
**  Purpose:        Indicate that operator mode is finished.
**
**  Parameters:     Name        Description.
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
void windowOperEnd(void)
    {
    currentX = currentY = currentFont = listPutAtGetChar = -1;
    listGet = listPut;
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
    int nextput, i, j, k;
    
    // Remember the x/y sum for this poll cycle
    nextput = sumListPut + 1;
    if (nextput == MaxPolls)
        {
        nextput = 0;
        }
    if (nextput != sumListGet)
        {
        s1list[sumListPut] = s1;
        s2list[sumListPut] = s2;
        listPutsAtGetChar[sumListPut] = listPutAtGetChar = listPut;
        i = sumListGet;
        j = -1;
        while (i != sumListPut)
            {
            if (s1 == s1list[i] && s2 == s2list[i])
                {
                j = listPutsAtGetChar[i];
                k = i;
                }
            i++;
            if (i == MaxPolls)
                {
                i = 0;
                }
            }
        s1 = s2 = 0;
        if (j != -1)
            {
            k++;
            if (k == MaxPolls)
                {
                k = 0;
                }
            listGet = j;
            sumListGet = k;
            }
        sumListPut = nextput;
        }

    // We treat a keyboard poll as the end of a display refresh cycle.
    listPutAtGetChar = listPut;
    windowCheckOutput();
    
    if (keyboardSendUp || keyListGet == keyListPut)
        {
        if (keyboardTrue && !opActive)
            {
            // If we're in true keyboard mode, lack of news means
            // "no change to last input" rather than "all keys up"!
            ppKeyIn = 0200;
            }
        else
            {
            ppKeyIn = 0;
            }
        keyboardSendUp = FALSE;
        return;
        }

    nextget = keyListGet + 1;
    if (nextget == KeyBufSize)
        {
        nextget = 0;
        }
    ppKeyIn = keybuf[keyListGet];
    keyListGet = nextget;
    if (!keyboardTrue || opActive)
        {
        // We're not doing the precise emulation, instead doing
        // regular key rollover.  So ignore key up events,
        // and send a zero code (all up) in between each key code.
        if (ppKeyIn & 0200)
            {
            ppKeyIn = 0;
            }
        else
            {
            keyboardSendUp = TRUE;
            }
        }
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
**                  hdcMem      bitmap context
**                  dx          current font size
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void dflush (HDC hdcMem, int dx)
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
    if (opActive)
        {
        SelectObject(hdcMem, currentFontInfo->normalId);
        }
    for (i = 0; i < dcnt; i++)
        {
        if (dhits[i] >= (dx / 2) - 1)
            {
            if (!bold)
                {
                if (opActive)
                    {
                    SelectObject(hdcMem, currentFontInfo->boldId);
                    }
                bold = TRUE;
                fy = fontY + boldoff;
                }
            }
        else
            {
            if (bold)
                {
                if (opActive)
                    {
                    SelectObject(hdcMem, currentFontInfo->normalId);
                    }
                bold = FALSE;
                fy = fontY;
                }
            }
        if (opActive)
            {
            str[0] = dchars[i];
            TextOut(hdcMem, x, y, str, 1);
            }
        else if (dchars[i] != 0)
            {
            BitBlt (hdcMem, x, y, dx, dx, hdcFont,
                    dx * (dchars[i] & ymask),
                    fy + (dx * ((dchars[i] & ~ymask) >> yshift)),
                    0x00EA02E9);
            }
        x += dx;
        }

    dcnt = 0;
    xstart = 07777;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Sum coordinates
**
**  Parameters:     x or y
**
**  Returns:        Nothing.
**
**------------------------------------------------------------------------*/
static void sum (u16 x)
    {
    // This is a Fletcher checsum of the 16 bit values passed in
    s1 += x;
    s2 += s1;
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
static void dput (HDC hdcMem, char c, int x, int y, int dx)
    {
    int dindx;
    
    // Center the character on the supplied x/y.
    x -= dx / 2;
    if (opActive)
        {
        y += dx * 3 / 8;
        }
    else
        {
        y -= dx / 2;
        }
    
    // Count hits on this position (for intensify)
    dindx = (x - xstart) / dx;
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
        if (opActive)
            {
            dchars[dcnt++] = ' ';
            }
        else
            {
            dchars[dcnt++] = 0;
            }
        }
    
    dhits[dcnt] = 1;
    dchars[dcnt++] = c;
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
        DefWinWidth + dx,       // window width
        DefWinHeight + dy,      // window height
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
        **  Create a bitmap big enough for our client rect.
        */
        hbmMem = CreateCompatibleBitmap(hdc,
                                        DefWinWidth, DefWinHeight);
        
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
#if CcHersheyFont == 1
        hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
        if (!hPen)
            {
            MessageBox (GetFocus(),
                        "Unable to get green pen", 
                        "CreatePen Error",
                        MB_OK);
            }
#endif
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
#if CcHersheyFont == 1
        if (hPen)
            {
            DeleteObject(hPen);
            }
#endif
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
        if (platoActive &&
            platoKeypress (wParam, 1, 0))
            {
            return 0;
            }
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

        case 'O':
        case 'o':
            opActive = TRUE;
            break;

        case 'p':
            if (niuPresent ())
                {
                platoActive = !platoActive;
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

    case WM_CHAR:
        if (!opActive)
            {
/* Non-operator keystrokes are handled in KEYUP/KEYDOWN */
            break;
            }
        windowQueueKey (wParam);
        break;
        
    case WM_KEYUP:
        if (platoActive || !keyboardTrue || opActive)
            {
            return 0;
            }
/* fall through */
    case WM_KEYDOWN:
        if (opActive)
            {
            return 0;
            }
        if (platoActive)
            {
            platoKeypress (wParam, 0, 0);
            return 0;
            }
/* 
 * Not Plato, not operator mode
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
    PAINTSTRUCT ps;
    HFONT hfntOld;
    int dotx, doty, doti;
    char buf[160];

    if (displayOff)
        {
        listGet = prevPut;
        return;
        }

    BeginPaint(hWnd, &ps);
    hdc = GetDC(hWnd);
//    hdc = BeginPaint(hWnd, &ps);
//??    GetClientRect(hWnd, &rect);

    /*
    **  Start with a clean bitmap
    */
    BitBlt(hdcMem, 
           0, 0, DefWinWidth, DefWinHeight,
           hdcMem,
           0, 0,
           BLACKNESS);

    currentFontInfo = &smallFont;
    hfntOld = SelectObject(hdcMem, smallOperFont.normalId);
    oldFont = FontSmall;


    if (debugDisplay)
        {
        /*
        **  Display P registers of PPUs and CPU and current trace mask.
        */
        sprintf(buf, "Refresh: %-10d  PP P-reg: %04o %04o %04o %04o %04o %04o %04o %04o %04o %04o   CPU P-reg: %06o",
                refreshCount++,
                ppu[0].regP, ppu[1].regP, ppu[2].regP, ppu[3].regP, ppu[4].regP,
                ppu[5].regP, ppu[6].regP, ppu[7].regP, ppu[8].regP, ppu[9].regP,
                cpu[0].regP); 

        if (cpuCount > 1)
            {
            sprintf(buf + strlen(buf), " %06o", cpu[1].regP);
            }
            
        sprintf(buf + strlen(buf),
                "   Trace: %c%c%c%c%c%c%c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c%c%c%c%c  %c",
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
                (traceMask & TraceCpu0) ? 'c' : '_',
                (traceMask & TraceCpu1) ? 'C' : '_',
                (traceMask & TraceEcs) ? 'E' : '_',
                (traceMask & TraceXj) ? 'J' : '_',
                (chTraceMask >> 0) & 1 ? '0' : '_',
                (chTraceMask >> 1) & 1 ? '1' : '_',
                (chTraceMask >> 2) & 1 ? '2' : '_',
                (chTraceMask >> 3) & 1 ? '3' : '_',
                (chTraceMask >> 4) & 1 ? '4' : '_',
                (chTraceMask >> 5) & 1 ? '5' : '_',
                (chTraceMask >> 6) & 1 ? '6' : '_',
                (chTraceMask >> 7) & 1 ? '7' : '_',
                (chTraceMask >> 8) & 1 ? '8' : '_',
                (chTraceMask >> 9) & 1 ? '9' : '_',
                (chTraceMask >> 10) & 1 ? 'A' : '_',
                (chTraceMask >> 11) & 1 ? 'B' : '_',
                (platoActive) ? 'P' : ' ');

        TextOut(hdcMem, 0, 0, buf, strlen(buf));
        }

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
#if CcHersheyFont == 1
    SelectObject(hdcMem, hPen);
#endif
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
            dflush (hdcMem, currentFontInfo->width);
            oldFont = curr->fontSize;

            switch (oldFont)
                {
            case FontSmall:
                if (opActive)
                    {
                    currentFontInfo = &smallOperFont;
                    }
                else
                    {
                    currentFontInfo = &smallFont;
                    }
                break;

            case FontMedium:
                if (opActive)
                    {
                    currentFontInfo = &mediumOperFont;
                    }
                else
                    {
                    currentFontInfo = &mediumFont;
                    }
                break;

            case FontLarge:
                currentFontInfo = &largeFont;
                break;
                }
            }

        if (curr->fontSize == FontDot)
            {
            dflush (hdcMem, currentFontInfo->width);
            if (curr->xPos == dotx && curr->yPos == doty &&
                doti < sizeof (dotdx) - 1)
                {
                doti++;
                }
            else
                {
                dotx = curr->xPos;
                doty = curr->yPos;
                doti = 0;
                }
            SetPixel(hdcMem, XADJUST (curr->xPos + dotdx[doti]),
                     YADJUST (curr->yPos + dotdy[doti]), RGB(0, 255, 0));
            }
        else
            {
#if CcHersheyFont == 0
            dput (hdcMem, curr->ch, curr->xPos, curr->yPos,
                  currentFontInfo->width);
#else
            if (opActive)
                {
                dput (hdcMem, curr->ch, curr->xPos, curr->yPos,
                      currentFontInfo->width);
                }
            else
                {
                windowTextPlot(hdcMem, XADJUST (curr->xPos), YADJUST (curr->yPos),
                               curr->ch, curr->fontSize);
                }
#endif
            }
        curr++;
        }

    dflush (hdcMem, currentFontInfo->width);
    listGet = end - display;
    listPutAtGetChar = -1;
    sumListGet = sumListPut;
    currentX = -1;
    currentY = -1;

    if (hfntOld)
        {
        SelectObject(hdcMem, hfntOld);
        }

    /*
    **  Blit the changes to the screen dc.
    */
    BitBlt(hdc, 
           rect.left, rect.top,
           DefWinWidth, DefWinHeight,
           hdcMem,
           0, 0,
           SRCCOPY);

    ReleaseDC(hWnd, hdc);
    EndPaint(hWnd, &ps);
    }

#if CcHersheyFont == 1

/*--------------------------------------------------------------------------
**  Purpose:        Define Hershey glyphs.
**
**------------------------------------------------------------------------*/
const char * const consoleHersheyGlyphs[64] =
    {
#if 1
    /* 00 = ':' */ "PURPRQSQSPRP RRURVSVSURU",
    /* 01 = 'A' */ "MWRMNV RRMVV RPSTS",
    /* 02 = 'B' */ "MWOMOV ROMSMUNUPSQ ROQSQURUUSVOV",
    /* 03 = 'C' */ "MXVNTMRMPNOPOSPURVTVVU",
    /* 04 = 'D' */ "MWOMOV ROMRMTNUPUSTURVOV",
    /* 05 = 'E' */ "MWOMOV ROMUM ROQSQ ROVUV",
    /* 06 = 'F' */ "MVOMOV ROMUM ROQSQ",
    /* 07 = 'G' */ "MXVNTMRMPNOPOSPURVTVVUVR RSRVR",
    /* 10 = 'H' */ "MWOMOV RUMUV ROQUQ",
    /* 11 = 'I' */ "PTRMRV",
    /* 12 = 'J' */ "NUSMSTRVPVOTOS",
    /* 13 = 'K' */ "MWOMOV RUMOS RQQUV",
    /* 14 = 'L' */ "MVOMOV ROVUV",
    /* 15 = 'M' */ "LXNMNV RNMRV RVMRV RVMVV",
    /* 16 = 'N' */ "MWOMOV ROMUV RUMUV",
    /* 17 = 'O' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM",
    /* 20 = 'P' */ "MWOMOV ROMSMUNUQSROR",
    /* 21 = 'Q' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM RSTVW",
    /* 22 = 'R' */ "MWOMOV ROMSMUNUQSROR RRRUV",
    /* 23 = 'S' */ "MWUNSMQMONOOPPTRUSUUSVQVOU",
    /* 24 = 'T' */ "MWRMRV RNMVM",
    /* 25 = 'U' */ "MXOMOSPURVSVUUVSVM",
    /* 26 = 'V' */ "MWNMRV RVMRV",
    /* 27 = 'W' */ "LXNMPV RRMPV RRMTV RVMTV",
    /* 30 = 'X' */ "MWOMUV RUMOV",
    /* 31 = 'Y' */ "MWNMRQRV RVMRQ",
    /* 32 = 'Z' */ "MWUMOV ROMUM ROVUV",
    /* 33 = '0' */ "MWRMPNOPOSPURVTUUSUPTNRM",
    /* 34 = '1' */ "MWPORMRV",
    /* 35 = '2' */ "MWONQMSMUNUPTROVUV",
    /* 36 = '3' */ "MWONQMSMUNUPSQ RRQSQURUUSVQVOU",
    /* 37 = '4' */ "MWSMSV RSMNSVS",
    /* 40 = '5' */ "MWPMOQQPRPTQUSTURVQVOU RPMTM",
    /* 41 = '6' */ "MWTMRMPNOPOSPURVTUUSTQRPPQOS",
    /* 42 = '7' */ "MWUMQV ROMUM",
    /* 43 = '8' */ "MWQMONOPQQSQUPUNSMQM RQQOROUQVSVUUURSQ",
    /* 44 = '9' */ "MWUPTRRSPROPPNRMTNUPUSTURVPV",
    /* 45 = '+' */ "LXRNRV RNRVR",
    /* 46 = '-' */ "LXNRVR",
    /* 47 = '*' */ "MWRORU ROPUT RUPOT",
    /* 50 = '/' */ "MWVLNX",
    /* 51 = '(' */ "OUTKRNQQQSRVTY",
    /* 52 = ')' */ "OUPKRNSQSSRVPY",
    /* 53 = '$' */ "MWRKRX RUNSMQMONOPQQTRUSUUSVQVOU",
    /* 54 = '=' */ "LXNPVP RNTVT",
    /* 55 = ' ' */ "",
    /* 56 = ',' */ "PUSVRVRUSUSWRY",
    /* 57 = '.' */ "PURURVSVSURU",
    /* 60 = '#' */ "MXQLQY RTLTY ROQVQ ROTVT",
    /* 61 = '[' */ "",
    /* 62 = ']' */ "",
    /* 63 = '%' */ "",
    /* 64 = '"' */ "NVPMPQ RTMTQ",
    /* 65 = '_' */ "",
    /* 66 = '!' */ "PURMRR RSMSR RRURVSVSURU",
    /* 67 = '&' */ "LXVRURTSSURVOVNUNSORRQSPSNRMPMONOPQSSUUVVV",
    /* 70 = ''' */ "PTRMRQ",
    /* 71 = '?' */ "NWPNRMSMUNUPRQRRSRSQUP RRURVSVSURU",
    /* 72 = '<' */ "",
    /* 73 = '>' */ "",
    /* 74 = '@' */ "",
    /* 75 = '\' */ "",
    /* 76 = '^' */ "",
    /* 77 = ';' */ "PURPRQSQSPRP RSVRVRUSUSWRY",
#else
    /* 00 = ' ' */ "",
    /* 01 = 'A' */ "MWRMNV RRMVV RPSTS",
    /* 02 = 'B' */ "MWOMOV ROMSMUNUPSQ ROQSQURUUSVOV",
    /* 03 = 'C' */ "MXVNTMRMPNOPOSPURVTVVU",
    /* 04 = 'D' */ "MWOMOV ROMRMTNUPUSTURVOV",
    /* 05 = 'E' */ "MWOMOV ROMUM ROQSQ ROVUV",
    /* 06 = 'F' */ "MVOMOV ROMUM ROQSQ",
    /* 07 = 'G' */ "MXVNTMRMPNOPOSPURVTVVUVR RSRVR",
    /* 10 = 'H' */ "MWOMOV RUMUV ROQUQ",
    /* 11 = 'I' */ "PTRMRV",
    /* 12 = 'J' */ "NUSMSTRVPVOTOS",
    /* 13 = 'K' */ "MWOMOV RUMOS RQQUV",
    /* 14 = 'L' */ "MVOMOV ROVUV",
    /* 15 = 'M' */ "LXNMNV RNMRV RVMRV RVMVV",
    /* 16 = 'N' */ "MWOMOV ROMUV RUMUV",
    /* 17 = 'O' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM",
    /* 20 = 'P' */ "MWOMOV ROMSMUNUQSROR",
    /* 21 = 'Q' */ "MXRMPNOPOSPURVSVUUVSVPUNSMRM RSTVW",
    /* 22 = 'R' */ "MWOMOV ROMSMUNUQSROR RRRUV",
    /* 23 = 'S' */ "MWUNSMQMONOOPPTRUSUUSVQVOU",
    /* 24 = 'T' */ "MWRMRV RNMVM",
    /* 25 = 'U' */ "MXOMOSPURVSVUUVSVM",
    /* 26 = 'V' */ "MWNMRV RVMRV",
    /* 27 = 'W' */ "LXNMPV RRMPV RRMTV RVMTV",
    /* 30 = 'X' */ "MWOMUV RUMOV",
    /* 31 = 'Y' */ "MWNMRQRV RVMRQ",
    /* 32 = 'Z' */ "MWUMOV ROMUM ROVUV",
    /* 33 = '0' */ "MWRMPNOPOSPURVTUUSUPTNRM",
    /* 34 = '1' */ "MWPORMRV",
    /* 35 = '2' */ "MWONQMSMUNUPTROVUV",
    /* 36 = '3' */ "MWONQMSMUNUPSQ RRQSQURUUSVQVOU",
    /* 37 = '4' */ "MWSMSV RSMNSVS",
    /* 40 = '5' */ "MWPMOQQPRPTQUSTURVQVOU RPMTM",
    /* 41 = '6' */ "MWTMRMPNOPOSPURVTUUSTQRPPQOS",
    /* 42 = '7' */ "MWUMQV ROMUM",
    /* 43 = '8' */ "MWQMONOPQQSQUPUNSMQM RQQOROUQVSVUUURSQ",
    /* 44 = '9' */ "MWUPTRRSPROPPNRMTNUPUSTURVPV",
    /* 45 = '+' */ "LXRNRV RNRVR",
    /* 46 = '-' */ "LXNRVR",
    /* 47 = '*' */ "MWRORU ROPUT RUPOT",
    /* 50 = '/' */ "MWVLNX",
    /* 51 = '(' */ "OUTKRNQQQSRVTY",
    /* 52 = ')' */ "OUPKRNSQSSRVPY",
    /* 53 = ' ' */ "",
    /* 54 = '=' */ "LXNPVP RNTVT",
    /* 55 = ' ' */ "",
    /* 56 = ',' */ "PUSVRVRUSUSWRY",
    /* 57 = '.' */ "PURURVSVSURU",
    /* 60 = ' ' */ "",
    /* 61 = ' ' */ "",
    /* 62 = ' ' */ "",
    /* 63 = ' ' */ "",
    /* 64 = ' ' */ "",
    /* 65 = ' ' */ "",
    /* 66 = ' ' */ "",
    /* 67 = ' ' */ "",
    /* 70 = ' ' */ "",
    /* 71 = ' ' */ "",
    /* 72 = ' ' */ "",
    /* 73 = ' ' */ "",
    /* 74 = ' ' */ "",
    /* 75 = ' ' */ "",
    /* 76 = ' ' */ "",
    /* 77 = ' ' */ "",
#endif
    };

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

#endif

/*---------------------------  End Of File  ------------------------------*/
