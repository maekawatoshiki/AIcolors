#include "stdafx.h"
#include "NN.h"
#include "NeuralNetworks.h"
#include <mmsystem.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <commdlg.h>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "comctl32.lib")
#define MAX_LOADSTRING 100
#define WND_WIDTH 200
#define WND_HEIGHT 100
#define ID_NGBUTTON 100
#define ID_OKBUTTON 101
#define ID_BLUEBTN 201
#define ID_GREENBTN 202
#define ID_YELLOWBTN 203
#define ID_CHECKBUTTON 102
#define ID_LEARNINGBUTTON 200

#define SRATE 8192


HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
HWND hOkButton, hText, hWnd;
int colorR = 0, colorG = 0, colorB = 0;
int nextColor = 0;
RECT rc;


ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	srand((unsigned int)time(0));
	w = time(0);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NEURALNETWORKS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NEURALNETWORKS));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NEURALNETWORKS));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_NEURALNETWORKS);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
		100, 100, WND_WIDTH, WND_HEIGHT, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//red blue yellow green white
COLORREF cust[16] = { 0 };
CHOOSECOLOR cc;
int CreateRGB()
{
	static int n = 0;
	if (n == 0)
	{
		cc.lStructSize = sizeof(CHOOSECOLOR);
		cc.hwndOwner = hWnd;
		cc.rgbResult = NULL;
		cc.lpCustColors = cust;
		cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	}
	ChooseColor(&cc);
	colorR = GetRValue(cc.rgbResult);
	colorG = GetGValue(cc.rgbResult);
	colorB = GetBValue(cc.rgbResult);
	return 0;
}


DWORD WINAPI ChangeColor(LPVOID *data)
{
	static double diff[3] = { 0 };
	static int saveNextColor;
	while (1)
	{
		saveNextColor = NNoutput(0);
		diff[0] = (double)(GetRValue(saveNextColor) - GetRValue(nextColor)) / 10.0;
		diff[1] = (double)(GetGValue(saveNextColor) - GetGValue(nextColor)) / 10.0;
		diff[2] = (double)(GetBValue(saveNextColor) - GetBValue(nextColor)) / 10.0;
		
		for (int i = 0; i < 10; i++)
		{
			nextColor += (int)diff[0];
			nextColor += (int)diff[1] << 8;
			nextColor += (int)diff[2] << 16;
			nextColor %= 0xFFFFFF;
			InvalidateRect(hWnd, &rc, TRUE);
			Sleep(20);
		}
		nextColor = saveNextColor;
		InvalidateRect(hWnd, &rc, TRUE);
		Sleep(3000); 
		if(rand() % 10 == 0) 
			w = (time(0) + clock()) ^ (clock() * 0xFFBA781E);
	}
	ExitThread(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int count = 0, xc = 0;
	static int x, y, oldx, oldy;
	static bool selendFlag = false;

	static WAVEFORMATEX wfe;
	static HWAVEOUT hWaveOut;
	static HWAVEIN hWaveIn;
	static BYTE *bWave;
	static WAVEHDR whdr;
	static HPEN hpen;
	static stringstream log;

	int wmId, wmEvent, trackPos;
	double starttime, endtime;
	PAINTSTRUCT ps;
	HDC hdc;

	rc.left = rc.top = 0;
	rc.bottom = 100;
	rc.right = 200;

	switch (message)
	{
	case WM_CREATE:

		CreateWindowEx(0,
			"BUTTON",
			"Check",
			WS_CHILD | WS_VISIBLE,
			0, 330, 400, 50,
			hWnd,
			(HMENU)ID_CHECKBUTTON,
			hInst, NULL);
		xc = 0;
		if (!fopen("w_10.fc", "rb"))
		{
			for (int i = 0; i < 10; i++)
			{
				if (i == 0) MessageBox(0, "好きな色を五つ、選択してください", 0, 0);
				if (i == 5) MessageBox(0, "嫌いな色を五つ、選択してください", 0, 0);
				CreateRGB();
				SelectObject(hdc, CreateSolidBrush(RGB(colorR, colorG, colorB)));
				PatBlt(hdc, 0, 0, 200, 100, PATCOPY);

				txy[i].x[0] = (double)colorR / 0xFF;
				txy[i].x[1] = (double)colorG / 0xFF;
				txy[i].x[2] = (double)colorB / 0xFF;
				if (i >= 5)
					txy[i].y[0] = 0;
				else
					txy[i].y[0] = 1;
				if (!selendFlag) count++;
			}
		}
		log << "Start of learning\r\n";
			starttime = clock();
				NNLearning(count);
			endtime = clock();
		log << "End of learning\r\n" << (endtime - starttime) / 1000.0 << "sec\r\n";

		InvalidateRect(hWnd, &rc, TRUE);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ChangeColor, NULL, 0, NULL);
		selendFlag = true;
		break;

	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (selendFlag)
		{
			SelectObject(hdc, CreateSolidBrush(nextColor));
			PatBlt(hdc, 0, 0, 200, 100, PATCOPY);
		}
		
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/*
#include "stdafx.h"
#include "Genetic_Algorithm_Color_Maker.h"
#include "Genetic.h"
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#define MAX_LOADSTRING 100
#define WND_WIDTH 220
#define WND_HEIGHT 350
#define ID_TRACKBAR 100
#define ID_OKBUTTON 101


HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
HWND hTrackbar, hOkButton;
GeneticColor Gene;


ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
_In_opt_ HINSTANCE hPrevInstance,
_In_ LPTSTR    lpCmdLine,
_In_ int       nCmdShow)
{
UNREFERENCED_PARAMETER(hPrevInstance);
UNREFERENCED_PARAMETER(lpCmdLine);

MSG msg;
HACCEL hAccelTable;

// Initialize global strings
LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
LoadString(hInstance, IDC_GENETIC_ALGORITHM_COLOR_MAKER, szWindowClass, MAX_LOADSTRING);
MyRegisterClass(hInstance);

// Perform application initialization:
if (!InitInstance(hInstance, nCmdShow))
{
return FALSE;
}

hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GENETIC_ALGORITHM_COLOR_MAKER));


// Main message loop:
while (GetMessage(&msg, NULL, 0, 0))
{
if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
{
TranslateMessage(&msg);
DispatchMessage(&msg);
}
}

return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX wcex;

wcex.cbSize = sizeof(WNDCLASSEX);

wcex.style = CS_HREDRAW | CS_VREDRAW;
wcex.lpfnWndProc = WndProc;
wcex.cbClsExtra = 0;
wcex.cbWndExtra = 0;
wcex.hInstance = hInstance;
wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GENETIC_ALGORITHM_COLOR_MAKER));
wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wcex.lpszMenuName = MAKEINTRESOURCE(IDC_GENETIC_ALGORITHM_COLOR_MAKER);
wcex.lpszClassName = szWindowClass;
wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
HWND hWnd;

hInst = hInstance; // Store instance handle in our global variable

hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
100, 100, WND_WIDTH, WND_HEIGHT, NULL, NULL, hInstance, NULL);

if (!hWnd)
return FALSE;

ShowWindow(hWnd, nCmdShow);
UpdateWindow(hWnd);

return TRUE;
}

int counter = 0;
int colorRGB = 0;
int evalRGB = 0;
int wmId, wmEvent, trackPos;
char s[128] = { 0 };
DWORD dwID;
PAINTSTRUCT ps;
HDC hdc;
RECT rc;
HWND hwnd;

DWORD WINAPI ThreadFunc(LPVOID vdParam)
{
int i;
while (1)
{
trackPos = 0;
for (i = 0; i < 300; i += 3)
trackPos +=
(Gene.gene[counter][i] - 0) * (Gene.gene[counter][i] - 0) +
(Gene.gene[counter][i + 1] - 0xFF) * (Gene.gene[counter][i + 1] - 0xFF) +
(Gene.gene[counter][i + 2] - 0xFF) * (Gene.gene[counter][i + 2] - 0xFF);

Gene.eval[counter] = trackPos;
sprintf(s, "%d", trackPos);
SetWindowText(hwnd, s);

if (counter == Gene.geneLen - 1)
{
colorRGB = Gene.evaluation();
if(counter%10000==0) InvalidateRect(hwnd, &rc, TRUE);
counter = 0;
continue;
}
else
{
evalRGB = counter + 1;
if (counter % 10000 == 0) InvalidateRect(hwnd, &rc, TRUE);
}
counter++;
}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
rc.top = 0;
rc.bottom = 200;
rc.left = 0;
rc.right = 300;
switch (message)
{
case WM_CREATE:
hwnd = hWnd;
hOkButton = CreateWindowEx(0,
"BUTTON",
"OK",
WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
0, 250, 200, 30,
hWnd,
(HMENU)ID_OKBUTTON,
hInst, NULL);
evalRGB = colorRGB = 0;
break;

case WM_COMMAND:
wmId = LOWORD(wParam);
wmEvent = HIWORD(wParam);
switch (wmId)
{
case ID_OKBUTTON:
CreateThread(NULL, 0, ThreadFunc, (LPVOID)hWnd, 0, &dwID);
}
break;

case WM_PAINT:
hdc = BeginPaint(hWnd, &ps);
{
int x = 0, y = 0;
for (int i = 0; i < 300; i += 3)
{
SelectObject(hdc, CreateSolidBrush(
RGB(Gene.gene[evalRGB][i], Gene.gene[evalRGB][i+1], Gene.gene[evalRGB][i+2])
));
PatBlt(hdc, x, y, 10, 10, PATCOPY);
x += 10;
if (x % 100 == 0 && x > 0)
{
x = 0; y += 10;
}
DeleteObject(
SelectObject(
hdc, GetStockObject(WHITE_BRUSH)
)
);
}
x = y = 0;
for (int i = 0; i < 300; i += 3)
{
SelectObject(hdc, CreateSolidBrush(
RGB(Gene.gene[colorRGB][i], Gene.gene[colorRGB][i + 1], Gene.gene[colorRGB][i + 2])
));
PatBlt(hdc, x, 120 + y, 10, 10, PATCOPY);
x += 10;
if (x % 100 == 0 && x > 0)
{
x = 0; y += 10;
}
DeleteObject(
SelectObject(
hdc, GetStockObject(WHITE_BRUSH)
)
);
}
}

EndPaint(hWnd, &ps);
break;

case WM_DESTROY:
PostQuitMessage(0);
break;

default:
return DefWindowProc(hWnd, message, wParam, lParam);
}
return 0;
}
*/