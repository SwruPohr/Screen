#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

typedef struct PixelBGRA {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} PixelBGRA;

typedef struct tagMPOINT {
    double x, y;
} MPOINT;
/*
typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT
typedef const RECT *LPCRECT;
typedef struct tagRECTL {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECTL,*PRECTL,*LPRECTL;
typedef const RECTL *LPCRECTL;
typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT,POINTL,*PPOINT,*LPPOINT,*PPOINTL,*LPPOINTL;
typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE,SIZEL,*PSIZE,*LPSIZE,*PSIZEL,*LPSIZEL;
typedef struct tagPOINTS {
    SHORT x;
    SHORT y;
} POINTS,*PPOINTS,*LPPOINTS;
*/

static LONG width = 0, height = 0;
static LONG wres = 256, hres = 256;

static inline LONG si(LONG x) {
    return 1 - 2 * (x % 2);
}

// Use integer pixel position
static inline int f(const POINT pos, const MPOINT ratio, PixelBGRA* col, const uint8_t time) {
    #define r col->r
    #define g col->g
    #define b col->b
    #define a col->a
    
    const LONG x = pos.x;
    const LONG y = pos.y;
    const double u =  (x / (double)width)
    const double v =  (y / (double)height)
    const double xs = (x * ratio.x)
    const double ys = (y * ratio.y)
    const LONG xm = x % wres;
    const LONG ym = y % hres;


    //double d = 

    /*
    g(x, y) = si(y) * x^y / (x!y!)

    if (w == 1 && h == 1)
        MAX = -1
    if (w == 1)
        MAX =  si(h) / h!
    if (h == 1) 
        MAX = -1 / (w - 1)!

    if (h => )
    MAX = si(h) * w^h / (w!h!)


    */

    if (x == 0) {
        *col = (PixelBGRA){0, 0, 0, 255};
        return 0;
    }

    r = (uint8_t)(u * u * 255);

    g = (uint8_t)((y * 255) / (hres - 1));

    b = (uint8_t) 0;

    a = 255;

    return 0;
}
#undef r
#undef g
#undef b
#undef a
#undef u
#undef v


static BITMAPINFO g_bmi;
static PixelBGRA* g_pixels = NULL; // instead of uint8_t*
static size_t g_pixels_size = 0;

static int ensure_buffer(int w, int h) {
    if (w <= 0 || h <= 0) return 1;
    if (w == width && h == height && g_pixels) return 0;
    /*
    if (w < width && h < height && g_pixels) {
        width = (LONG)w;
        height = (LONG)h;
        return 0;
    }
    */
    if (g_pixels) {
        free(g_pixels);
        g_pixels = NULL;
    }

    width = (LONG)w;
    height = (LONG)h;

    g_pixels_size = (size_t)w * (size_t)h;

    g_pixels = (PixelBGRA*)malloc(g_pixels_size * sizeof(PixelBGRA));
    if (!g_pixels) {
        width = 0L;
        height = 0L;
        return 2;
    }

    ZeroMemory(&g_bmi, sizeof(g_bmi));
    g_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth = width;
    g_bmi.bmiHeader.biHeight = -height;     // top-down
    g_bmi.bmiHeader.biPlanes = 1;
    g_bmi.bmiHeader.biBitCount = 32;
    g_bmi.bmiHeader.biCompression = BI_RGB;
    return 0;
}

static int render_frame(void) {
    MPOINT ratio = {(double)wres / width, (double)hres / height};

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            POINT pos = {x, y};

            PixelBGRA* poi = g_pixels + (size_t)y * (size_t)width + (size_t)x;
            
            int x = f(pos, ratio, poi, 1);
            
            if (x != 0) return x;
        }
    }
}

static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    (void)wParam; (void)lParam;

    switch (msg) {
        case WM_SIZE: {
            RECT cr;
            GetClientRect(hwnd, &cr);
            
            int x = ensure_buffer(cr.right - cr.left, cr.bottom - cr.top);
            if (x != 0) return x;
            render_frame();

            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (g_pixels && width > 0 && height > 0) {
                StretchDIBits(
                    hdc,
                    0, 0, width, height,      // dest
                    0, 0, width, height,      // src
                    g_pixels, // const *VOID
                    &g_bmi,
                    DIB_RGB_COLORS,
                    SRCCOPY
                );
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}


static inline int isWhitespace(char c) {
    return (c == ' ' || (c >= '\t' && c <= '\r'));
}

static inline int LONG_from_string(LPSTR str, LONG *n) {
    LONG num = 0;
    int i = 0;
    // whitespace
    while (isWhitespace(str[i])) str++;

    // digits
    while (str[i] >= '0' && str[i] <= '9') {
        num = (num * 10) + (str[i] - '0');
        i++;
    }
    // set value
    if (i) *n = num;

    // return found
    return i;
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrev; (void)lpCmdLine; (void)nCmdShow;
    if (!lpCmdLine) { return 1; }

    int i = LONG_from_string(lpCmdLine, &wres);
    if (i == 0) { return 1; }
    i = LONG_from_string(lpCmdLine + i, &hres);
    if (i == 0) { return 1; }
    printf("%i", wres);
    printf("%i", hres);

    const wchar_t* className = L"PixelFuncWindow";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = wndproc;
    wc.hInstance = hInst;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);



    HWND hwnd = CreateWindowExW(
        0, className, L"Hello",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        900, 600,
        NULL, NULL, hInst, NULL
    );
    if (!hwnd) return 1;

    RECT rc;
    GetClientRect(hwnd, &rc);
    ensure_buffer(rc.right - rc.left, rc.bottom - rc.top);
    render_frame();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_pixels) free(g_pixels);
    return 0;
}
