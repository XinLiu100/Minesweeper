#include "stdafx.h"
#include "MyWinMine.h"

#define MAX_LOADSTRING 100


// 全局变量:
HINSTANCE hInst;								// instance
TCHAR szTitle[MAX_LOADSTRING];					// title
TCHAR szWindowClass[MAX_LOADSTRING];			
int nPosX,nPosY;
int nPosXP,nPosYP;								// previous position
int nObgX,nObgY;
int nNbgX,nNbgY;
int nFbgX,nFbgY;
int nTime;										

int M_LENGTH;
int M_WIDTH;
int MINE_NUM;

int nBlankReminNum;								// remaining number
int nFlag;										// Flag number
bool bIsLose;									// 0 in game，1 win，-1 lose
BOOL bExit,bReset;
BOOL bColor;									// true color，false no color
int nGameMode;									// 1 beginner，2 median，3 advanced，0 custom
int anBestTime[3];

int** anMine;//0 no mine，1 mine，2 pressed，4 flag，8 question mark，last 16 bit: mine number surrounding


ATOM				MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Custom(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Rank(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	RankInput(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int ClickProc(MSG msg,HDC hdc,HDC hdcMine,HDC hdcFace);
int MineBitBlt(int nX,int nY,HDC hdc,HDC hdcMine,int nOpType);
int MineBitBltRec(int nX,int nY,HDC hdc,HDC hdcMine);
BOOL CheckIfMine(int x,int y,HDC hdc,HDC hdcMine);
int CheckAround(int ix,int iy,HDC hdc,HDC hdcMine);

int DrawFrame(HWND hWnd,HDC hdcNum,HDC hdcFace,int nLineWidth1,int nLineWidth2,int nLineWidth3,int nBoardWidth);
int ReDraw(HDC hdc,HDC hdcMine,HDC hdcNum,HDC hdcFace,HWND hWnd,int nLineWidth1,int nLineWidth2,int nLineWidth3,int nBoardWidth);
int AlDraw(int x,int y,HDC hdc,HDC hdcMine,bool bWinOrLose);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	HDC hdc,hdcMine,hdcNum,hdcFace;
	HWND hWnd;
	BOOL bStart;
	int nMineNum,nMineInit;
	int nTmpI,nTmpJ;
	int nLengthForDelete;
	HBITMAP hBmpMine,hBmpFace,hBmpNum;

	
	// global string
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MYWINMINE, szWindowClass, MAX_LOADSTRING);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYWINMINE));

	M_LENGTH = 16;
	M_WIDTH = 16;
	MINE_NUM = 40;
	nGameMode = 2;
	bColor = true;
	
	MyRegisterClass(hInstance);
	// app init
	hInst = hInstance; // handle
	hWnd = CreateWindow(szWindowClass, szTitle,  WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_TILED,
		CW_USEDEFAULT, CW_USEDEFAULT, BLOCKWIDTH*M_LENGTH+6+20 , BLOCKWIDTH*M_WIDTH+45+68, NULL, NULL, hInstance, NULL);
	
	HKEY hKey; 		
	DWORD lpdwDisposition;
	BYTE lpName[20];	
	DWORD dwDataLength;
	DWORD dwTime = 0;
	DWORD dwInitTime = 999;
	while(1)
	{
		if(RegOpenKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\winmine", &hKey) != ERROR_SUCCESS)	
		{
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&lpdwDisposition);
			RegSetValueEx(hKey,L"Name1",NULL,REG_SZ,(BYTE*)L"anonymous",5);
			RegSetValueEx(hKey,L"Name2",NULL,REG_SZ,(BYTE*)L"anonymous",5);
			RegSetValueEx(hKey,L"Name3",NULL,REG_SZ,(BYTE*)L"anonymous",5);
			RegSetValueEx(hKey,L"Time1",NULL,REG_DWORD,(BYTE*)&dwInitTime,sizeof(DWORD));
			RegSetValueEx(hKey,L"Time2",NULL,REG_DWORD,(BYTE*)&dwInitTime,sizeof(DWORD));
			RegSetValueEx(hKey,L"Time3",NULL,REG_DWORD,(BYTE*)&dwInitTime,sizeof(DWORD));
		}
		else
		{
			RegQueryValueEx(hKey,L"Time1",NULL,NULL,lpName,&dwDataLength);anBestTime[0] = *((DWORD*)lpName);
			RegQueryValueEx(hKey,L"Time2",NULL,NULL,lpName,&dwDataLength);anBestTime[1] = *((DWORD*)lpName);
			RegQueryValueEx(hKey,L"Time3",NULL,NULL,lpName,&dwDataLength);anBestTime[2] = *((DWORD*)lpName);
		}
		RegCloseKey(hKey);
		if(bExit == TRUE)
			break;
		bStart = FALSE;
		nMineNum = MINE_NUM;nMineInit = 0;
		nPosX = nPosY = nPosX = nPosYP = 0;
		nTime = 0;
		nFlag = 0;
		bExit = FALSE;
		bReset = FALSE;

		nLengthForDelete = M_LENGTH;
		anMine = new int*[M_LENGTH];
		for(int i=0;i<M_LENGTH;i++)
		{
			anMine[i] = new int[M_WIDTH];
		}

		if(bColor)
		{
			hBmpMine = (HBITMAP)LoadImage(hInstance,(LPTSTR)(IDB_MINE),IMAGE_BITMAP,0,0,LR_COPYFROMRESOURCE);	
			hBmpFace = (HBITMAP)LoadImage(hInstance,(LPTSTR)(IDB_FACE),IMAGE_BITMAP,0,0,LR_COPYFROMRESOURCE);	
			hBmpNum  = (HBITMAP)LoadImage(hInstance,(LPTSTR)(IDB_NUM) ,IMAGE_BITMAP,0,0,LR_COPYFROMRESOURCE);	
		}
		else
		{
			hBmpMine = (HBITMAP)LoadImage(hInstance,(LPTSTR)(IDB_MINEBW),IMAGE_BITMAP,0,0,LR_COPYFROMRESOURCE);	
			hBmpFace = (HBITMAP)LoadImage(hInstance,(LPTSTR)(IDB_FACEBW),IMAGE_BITMAP,0,0,LR_COPYFROMRESOURCE);	
			hBmpNum  = (HBITMAP)LoadImage(hInstance,(LPTSTR)(IDB_NUMBW) ,IMAGE_BITMAP,0,0,LR_COPYFROMRESOURCE);	
		}
		hdc = GetDC(hWnd);
		hdcMine = CreateCompatibleDC(hdc);
		SelectObject(hdcMine,hBmpMine);

		hdcNum = CreateCompatibleDC(hdc);
		SelectObject(hdcNum,hBmpNum);

		hdcFace = CreateCompatibleDC(hdc);
		SelectObject(hdcFace,hBmpFace);
		//

	
		if (!hWnd)
		{
			MessageBox(0,L"CreateWindow failed!",L"Error",MB_ICONERROR);
			exit(-1);
		}
	
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	
	
		int nLineWidth1 = 5,nLineWidth2 = 4,nLineWidth3 = 2,nBoardWidth = 36;
		DrawFrame(hWnd,hdcNum,hdcFace,nLineWidth1,nLineWidth2,nLineWidth3,nBoardWidth);
	
		nPosXP = nObgX;
		nPosYP = nObgY;

	
		SetTimer(hWnd,TIMER_TIME,1000,NULL);
		SetTimer(hWnd,TIMER_MINE,10,NULL);


	
		while(1)
		{	
			if(bExit == TRUE || bReset == TRUE)
				break;
			nTime = 0;
			nFlag = 0;
			nMineInit = 0;
			for(int i=0;i<M_LENGTH;i++)
			{
				for(int j=0;j<M_WIDTH;j++)
				{
					anMine[i][j] = 0;
				}
			}
			nBlankReminNum = M_LENGTH*M_WIDTH;
			bIsLose = false;
			srand(time(0));
			while(nMineInit<nMineNum)
			{
				nTmpI = rand()%M_LENGTH;
				nTmpJ = rand()%M_WIDTH;
				if(anMine[nTmpI][nTmpJ] == 0)
					anMine[nTmpI][nTmpJ] = 1;
				else
					continue;
				nMineInit++;
			}
			for(int i=0;i<M_LENGTH;i++)
				for(int j=0;j<M_WIDTH;j++)
					BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,0,SRCCOPY);
			ReDraw(hdc,hdcMine,hdcNum,hdcFace,hWnd,nLineWidth1,nLineWidth2,nLineWidth3,nBoardWidth);
			UpdateWindow(hWnd);
			// Main message loop:
			while (GetMessage(&msg, NULL, 0, 0))
			{
				if(bReset == TRUE)
					break;
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				};
				if(msg.message == WM_PAINT)
				{
					if(!bIsLose)
						ReDraw(hdc,hdcMine,hdcNum,hdcFace,hWnd,nLineWidth1,nLineWidth2,nLineWidth3,nBoardWidth);
					else
					{
						ReDraw(hdc,hdcMine,hdcNum,hdcFace,hWnd,nLineWidth1,nLineWidth2,nLineWidth3,nBoardWidth);
						AlDraw(0,0,hdc,hdcMine,false);
					}
				}
				if(((msg.message == WM_LBUTTONDOWN) ||(msg.message == WM_RBUTTONDOWN))&&!bIsLose)
				{
					if((LOWORD(msg.lParam)-nObgX)>0 && (LOWORD(msg.lParam)-nObgX)<M_LENGTH*BLOCKWIDTH && (HIWORD(msg.lParam)-nObgY)>0 && (HIWORD(msg.lParam)-nObgY)<M_WIDTH*BLOCKWIDTH )
						bStart = true;
				}
		
				if((LOWORD(msg.lParam)>(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2) && LOWORD(msg.lParam)<(nNbgX+nFbgX+3*NUMWIDTH)/2+FACEWIDTH/2 && HIWORD(msg.lParam)>nFbgY && HIWORD(msg.lParam)<nFbgY+FACEWIDTH)
				{
					if(msg.message == WM_LBUTTONUP)
					{
						bStart = false;
						BitBlt(hdc,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,nFbgY,FACEWIDTH,FACEWIDTH,hdcFace,0,SMILE*FACEWIDTH,SRCCOPY);
						break;
					}
					else if((msg.wParam&MK_LBUTTON) != 0)
						BitBlt(hdc,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,nFbgY,FACEWIDTH,FACEWIDTH,hdcFace,0,SMILED*FACEWIDTH,SRCCOPY);
				}

				if(bStart)
				{
					if(msg.message == WM_LBUTTONUP)
						BitBlt(hdc,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,nFbgY,FACEWIDTH,FACEWIDTH,hdcFace,0,SMILE*FACEWIDTH,SRCCOPY);
					else if((msg.wParam&MK_RBUTTON) != 0 || msg.message == WM_LBUTTONDOWN)
						BitBlt(hdc,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,nFbgY,FACEWIDTH,FACEWIDTH,hdcFace,0,OMOUTH*FACEWIDTH,SRCCOPY);
		
					if(msg.message == WM_TIMER)
					{
						if(msg.wParam == TIMER_TIME)
						{
							if(nTime<999)
								nTime++;
							if(nTime<0)
							{
								BitBlt(hdc,nNbgX,nNbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,MINUS*NUMLENGTH,SRCCOPY);
								BitBlt(hdc,nNbgX+NUMWIDTH,nNbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%100)/10)*NUMLENGTH,SRCCOPY);
								BitBlt(hdc,nNbgX+2*NUMWIDTH,nNbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%10))/10*NUMLENGTH,SRCCOPY);
							}
							BitBlt(hdc,nNbgX,nNbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%1000)/100)*NUMLENGTH,SRCCOPY);
							BitBlt(hdc,nNbgX+NUMWIDTH,nNbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%100)/10)*NUMLENGTH,SRCCOPY);
							BitBlt(hdc,nNbgX+2*NUMWIDTH,nNbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%10))*NUMLENGTH,SRCCOPY);
						}
						if(msg.wParam == TIMER_MINE)
						{				
							if((MINE_NUM-nFlag)<0)
							{
								BitBlt(hdc,nFbgX,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,MINUS*NUMLENGTH,SRCCOPY);
								BitBlt(hdc,nFbgX+NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((nFlag-MINE_NUM)%100)/10)*NUMLENGTH,SRCCOPY);
								BitBlt(hdc,nFbgX+2*NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nFlag-MINE_NUM)%10)*NUMLENGTH,SRCCOPY);
							}
							BitBlt(hdc,nFbgX,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((MINE_NUM-nFlag)%1000)/100)*NUMLENGTH,SRCCOPY);
							BitBlt(hdc,nFbgX+NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((MINE_NUM-nFlag)%100)/10)*NUMLENGTH,SRCCOPY);
							BitBlt(hdc,nFbgX+2*NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((MINE_NUM-nFlag)%10))*NUMLENGTH,SRCCOPY);
						}
					}
					ClickProc(msg,hdc,hdcMine,hdcFace);
					if(nBlankReminNum == MINE_NUM && bIsLose == false)
					{
						nFlag = MINE_NUM;
						BitBlt(hdc,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,nFbgY,FACEWIDTH,FACEWIDTH,hdcFace,0,WIN*FACEWIDTH,SRCCOPY);
						//MessageBox(0,L"You Win!",L"Congratulations!",MB_OK);
						ReDraw(hdc,hdcMine,hdcNum,hdcFace,hWnd,nLineWidth1,nLineWidth2,nLineWidth3,nBoardWidth);
						AlDraw(0,0,hdc,hdcMine,true);
						if(nGameMode != 0)
						{
							if(anBestTime[nGameMode-1]>nTime)
							{
								if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&lpdwDisposition))
									MessageBox(0,L"open error!",0,0);
								if(nGameMode == 1)
									if(ERROR_SUCCESS != RegSetValueEx(hKey,L"Time1",NULL,REG_DWORD,(BYTE*)&nTime,sizeof(DWORD)))
									{
										MessageBox(0,0,0,0);
									}
								if(nGameMode == 2)
									RegSetValueEx(hKey,L"Time2",NULL,REG_DWORD,(BYTE*)&nTime,sizeof(DWORD));
								if(nGameMode == 3)
									RegSetValueEx(hKey,L"Time3",NULL,REG_DWORD,(BYTE*)&nTime,sizeof(DWORD));
								RegCloseKey(hKey);
								DialogBox(hInst, MAKEINTRESOURCE(IDD_RANK_INPUT), hWnd, RankInput);
							}
						}
						bStart = false;
						//exit(0);
					}
					else if(bIsLose == true)
					{
						BitBlt(hdc,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,nFbgY,FACEWIDTH,FACEWIDTH,hdcFace,0,LOSE*FACEWIDTH,SRCCOPY);
						bStart = false;
					}
					//UpdateWindow(hWnd);
				}
			}
		}
		KillTimer(hWnd,TIMER_TIME);
		KillTimer(hWnd,TIMER_MINE);

		ReleaseDC(hWnd,hdcFace);
		ReleaseDC(hWnd,hdcMine);
		ReleaseDC(hWnd,hdcNum);
		ReleaseDC(hWnd,hdc);
	
		DeleteObject(hBmpMine);
		DeleteObject(hBmpFace);
		DeleteObject(hBmpNum);
		for(int i=0;i<nLengthForDelete;i++)
		{
			delete[] anMine[i];
		}
		delete[]anMine;
	//	UpdateWindow(hWnd);
	//	InvalidateRect(hWnd,&windowsRect,TRUE);
	}


	return (int) msg.wParam;
}




//
//  register window class 
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MYWINMINE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassEx(&wcex);
}


//
//  main window message
//
//  WM_COMMAND	- menu
//  WM_PAINT	- paint main window
//  WM_DESTROY	- destroy
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT windowsRect;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case ID_NEW:
			bReset = TRUE;
			break;
		case ID_COLOR:
			bColor = !bColor;
			bReset = TRUE;
			break;
		case ID_BASIC:
			nGameMode = 1;
			M_LENGTH = 9;
			M_WIDTH = 9;
			MINE_NUM = 10;
			bReset = TRUE;
			GetWindowRect(hWnd,&windowsRect);
			SetWindowPos(hWnd,0,windowsRect.left,windowsRect.top,BLOCKWIDTH*M_LENGTH+6+20 , BLOCKWIDTH*M_WIDTH+45+68,0);
			break;
		case ID_INTERM:
			nGameMode = 2;
			M_LENGTH = 16;
			M_WIDTH = 16;
			MINE_NUM = 40;
			bReset = TRUE;
			GetWindowRect(hWnd,&windowsRect);
			SetWindowPos(hWnd,0,windowsRect.left,windowsRect.top,BLOCKWIDTH*M_LENGTH+6+20 , BLOCKWIDTH*M_WIDTH+45+68,0);
			break;
		case ID_EXPERT:
			nGameMode = 3;
			M_LENGTH = 30;
			M_WIDTH = 16;
			MINE_NUM = 99;
			bReset = TRUE;
			GetWindowRect(hWnd,&windowsRect);
			SetWindowPos(hWnd,0,windowsRect.left,windowsRect.top,BLOCKWIDTH*M_LENGTH+6+20 , BLOCKWIDTH*M_WIDTH+45+68,0);
			break;
		case ID_CUSTOM:
			nGameMode = 0;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CUSTOM), hWnd, Custom);
			bReset = TRUE;
			GetWindowRect(hWnd,&windowsRect);
			SetWindowPos(hWnd,0,windowsRect.left,windowsRect.top,BLOCKWIDTH*M_LENGTH+6+20 , BLOCKWIDTH*M_WIDTH+45+68,0);
			break;
		case ID_RANK:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_RANK), hWnd, Rank);
			break;
		case IDM_EXIT:
			bExit = TRUE;
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		bExit = TRUE;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// "About" dialog box message
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK RankInput(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HKEY hKey; 
	DWORD lpdwDisposition;
	//BYTE lpName[20];
	wchar_t lpText[20];
	//DWORD dwDataLength;
	DWORD dwTime = 0;

	switch (message)
	{
	case WM_INITDIALOG:	
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_OK)
		{
			GetDlgItemText(hDlg,IDC_NAME,lpText,20);
			if(nGameMode == 1)
			{
				if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&lpdwDisposition))
					MessageBox(0,L"open error!",0,0);
				if(ERROR_SUCCESS != RegSetValueEx(hKey,L"Name1",NULL,REG_SZ,(BYTE*)lpText,2*lstrlen(lpText)+2))
					MessageBox(0,L"set error!",0,0);
				RegCloseKey(hKey);
			}
			if(nGameMode == 2)
			{
				RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&lpdwDisposition);
				RegSetValueEx(hKey,L"Name2",NULL,REG_SZ,(BYTE*)lpText,2*lstrlen(lpText)+2);
				RegCloseKey(hKey);
			}
			if(nGameMode == 3)
			{
				RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&lpdwDisposition);
				RegSetValueEx(hKey,L"Name3",NULL,REG_SZ,(BYTE*)lpText,2*lstrlen(lpText)+2);
				RegCloseKey(hKey);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK Rank(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HKEY hKey; 
	DWORD lpdwDisposition;
	BYTE lpName[20];
	wchar_t lpText[20];
	DWORD dwDataLength;
	DWORD dwTime = 0;

	switch (message)
	{
	case WM_INITDIALOG:	
		if(RegOpenKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\winmine", &hKey) != ERROR_SUCCESS)	
		{
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine\\",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&lpdwDisposition);
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine\\Name1",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&lpdwDisposition);
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine\\Name2",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&lpdwDisposition);
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine\\Name3",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&lpdwDisposition);
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine\\Time1",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&lpdwDisposition);
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine\\Time2",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&lpdwDisposition);
			RegCreateKeyEx(HKEY_CURRENT_USER,L"Software\\Microsoft\\winmine\\Time3",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&lpdwDisposition);
		}
		else
		{
			RegQueryValueEx(hKey,L"Name1",NULL,NULL,lpName,&dwDataLength);
			SetDlgItemText(hDlg,IDC_NAME1,(LPCWSTR)lpName);
			RegQueryValueEx(hKey,L"Name2",NULL,NULL,lpName,&dwDataLength);
			SetDlgItemText(hDlg,IDC_NAME2,(LPCWSTR)lpName);
			RegQueryValueEx(hKey,L"Name3",NULL,NULL,lpName,&dwDataLength);
			SetDlgItemText(hDlg,IDC_NAME3,(LPCWSTR)lpName);

			RegQueryValueEx(hKey,L"Time1",NULL,NULL,lpName,&dwDataLength);dwTime = *((DWORD*)lpName);
			wsprintf(lpText,L"%d 秒",dwTime);
			SetDlgItemText(hDlg,IDC_TIME1,(LPCWSTR)lpText);
			RegQueryValueEx(hKey,L"Time2",NULL,NULL,lpName,&dwDataLength);dwTime = *((DWORD*)lpName);
			wsprintf(lpText,L"%d 秒",dwTime);
			SetDlgItemText(hDlg,IDC_TIME2,(LPCWSTR)lpText);
			RegQueryValueEx(hKey,L"Time3",NULL,NULL,lpName,&dwDataLength);dwTime = *((DWORD*)lpName);
			wsprintf(lpText,L"%d 秒",dwTime);
			SetDlgItemText(hDlg,IDC_TIME3,(LPCWSTR)lpText);
		}
		RegCloseKey(hKey);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK Custom(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	BOOL bSuccess;
	int nTmp;
	switch (message)
	{
	case WM_INITDIALOG:
		nGameMode = 0;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_OK)
		{
			nTmp = GetDlgItemInt(hDlg,IDC_EDIT_M,&bSuccess,false);
			if(bSuccess) MINE_NUM = nTmp;
			nTmp = GetDlgItemInt(hDlg,IDC_EDIT_L,&bSuccess,false);
			if(bSuccess) M_LENGTH = nTmp;
			nTmp = GetDlgItemInt(hDlg,IDC_EDIT_W,&bSuccess,false);
			if(bSuccess) M_WIDTH = nTmp;
			if(MINE_NUM<10) MINE_NUM = 9;if(MINE_NUM>99) MINE_NUM = 99;
			if(M_LENGTH<10) M_LENGTH = 9;if(M_LENGTH>30) MINE_NUM = 30;
			if(M_WIDTH<10)  M_WIDTH = 9;if(M_WIDTH>16)  M_WIDTH = 16;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDC_CANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
int ClickProc(MSG msg,HDC hdc,HDC hdcMine,HDC hdcFace)
{
	bool bU = false,bD = false;
	static bool bIsDbClick = false;
	static bool sbIsStart = false;
	int nDbClickBlankNum = 0;
	nPosX = LOWORD(msg.lParam);
	nPosY = HIWORD(msg.lParam);
	
	nPosX -= nObgX;
	nPosY -= nObgY;
	if(!sbIsStart)
	{
		nPosXP = nPosX;
		nPosYP = nPosY;
		sbIsStart = true;
	}
	if(nPosX<=0||nPosX>=M_LENGTH*BLOCKWIDTH)
		return 0;
	else if(nPosY<=0||nPosY>=M_WIDTH*BLOCKWIDTH)
		return 0;
	MineBitBltRec(0,0,hdc,hdcMine);

	if(int(nPosXP/BLOCKWIDTH)+1<M_LENGTH)
	{
		nDbClickBlankNum += MineBitBltRec(1,0,hdc,hdcMine);		
		if(int(nPosYP/BLOCKWIDTH)<M_WIDTH-1)		
		{
			if(!bU)
			{
				nDbClickBlankNum += MineBitBltRec(0,1,hdc,hdcMine);
				bU = true;
			}
			nDbClickBlankNum += MineBitBltRec(1,1,hdc,hdcMine);	
		}
		if(int(nPosYP/BLOCKWIDTH)>=1)
		{
			if(!bD)
			{
				nDbClickBlankNum += MineBitBltRec(0,-1,hdc,hdcMine);			
				bD = true;
			}
			nDbClickBlankNum += MineBitBltRec(1,-1,hdc,hdcMine);
		}		
	}
	if(int(nPosXP/BLOCKWIDTH)>=1)	
	{
		nDbClickBlankNum += MineBitBltRec(-1,0,hdc,hdcMine);		
		if(int(nPosYP/BLOCKWIDTH)<M_WIDTH-1)
		{	
			if(!bU)
			{
				nDbClickBlankNum += MineBitBltRec(0,1,hdc,hdcMine);
				bU = true;
			}
			nDbClickBlankNum += MineBitBltRec(-1,1,hdc,hdcMine);			
		}	
		if(int(nPosYP/BLOCKWIDTH)>=1)
		{	
			if(!bD)
			{
				nDbClickBlankNum += MineBitBltRec(0,-1,hdc,hdcMine);
				bD = true;
			}
			nDbClickBlankNum += MineBitBltRec(-1,-1,hdc,hdcMine);			
		}	
	}

	if(msg.message == WM_RBUTTONDOWN)
	{	
		if((anMine[int(nPosX/BLOCKWIDTH)][int(nPosY/BLOCKWIDTH)]&4) == 0 && (anMine[int(nPosX/BLOCKWIDTH)][int(nPosY/BLOCKWIDTH)]&8) == 0)
			MineBitBlt(0,0,hdc,hdcMine,FLAG);
		else if((anMine[int(nPosX/BLOCKWIDTH)][int(nPosY/BLOCKWIDTH)]&8) != 0)
			MineBitBlt(0,0,hdc,hdcMine,BLANK);
		else if((anMine[int(nPosX/BLOCKWIDTH)][int(nPosY/BLOCKWIDTH)]&4) != 0)
			MineBitBlt(0,0,hdc,hdcMine,QUES);
	}
	else if(msg.message == WM_RBUTTONUP)
	{
		if((anMine[int(nPosX/BLOCKWIDTH)][int(nPosY/BLOCKWIDTH)]&8) == 0)
		;//	MineBitBlt(0,0,hdc,hdcMine,BLANK);
	}
	else if(msg.message == WM_LBUTTONDOWN)
	{	
		anMine[int(nPosX/BLOCKWIDTH)][int(nPosY/BLOCKWIDTH)] &= 0xFFFFFFF7;
	}
	else if(msg.message == WM_LBUTTONUP)
	{
		if(!bIsDbClick)
			CheckIfMine(0,0,hdc,hdcMine);
		else if((nDbClickBlankNum == ((anMine[int(nPosX/BLOCKWIDTH)][int(nPosY/BLOCKWIDTH)])>>16)) && nDbClickBlankNum!=0)
		{
			bIsDbClick = false;
			bU = bD = false;
			if(int(nPosX/BLOCKWIDTH)+1<M_LENGTH)
			{
				CheckIfMine(1,0,hdc,hdcMine);		
				if(int(nPosY/BLOCKWIDTH)<M_WIDTH-1)		
				{
					if(!bU)
					{
						CheckIfMine(0,1,hdc,hdcMine);
						bU = true;
					}
					CheckIfMine(1,1,hdc,hdcMine);	
				}
				if(int(nPosY/BLOCKWIDTH)>=1)
				{
					if(!bD)
					{
						CheckIfMine(0,-1,hdc,hdcMine);			
						bD = true;
					}
					CheckIfMine(1,-1,hdc,hdcMine);
				}		
			}
			if(int(nPosX/BLOCKWIDTH)>=1)	
			{
				CheckIfMine(-1,0,hdc,hdcMine);		
				if(int(nPosY/BLOCKWIDTH)<M_WIDTH-1)
				{	
					if(!bU)
					{
						CheckIfMine(0,1,hdc,hdcMine);
						bU = true;
					}
					CheckIfMine(-1,1,hdc,hdcMine);			
				}	
				if(int(nPosY/BLOCKWIDTH)>=1)
				{	
					if(!bD)
					{
						CheckIfMine(0,-1,hdc,hdcMine);
						bD = true;
					}
					CheckIfMine(-1,-1,hdc,hdcMine);			
				}	
			}
		}
		else
			bIsDbClick = false;
	}

	if((msg.wParam&MK_LBUTTON) != 0)
	{
		if(msg.wParam == (MK_LBUTTON|MK_RBUTTON))
		{
			bIsDbClick = true;
			MineBitBlt(0,0,hdc,hdcMine,BLANKD);
			if(int(nPosX/BLOCKWIDTH)+1<M_LENGTH)
			{
				MineBitBlt(1,0,hdc,hdcMine,BLANKD);
				if(int(nPosY/BLOCKWIDTH)<M_WIDTH-1)
				{
					MineBitBlt(0,1,hdc,hdcMine,BLANKD);
					MineBitBlt(1,1,hdc,hdcMine,BLANKD);
				}
				if(int(nPosY/BLOCKWIDTH)>=1)
				{
					MineBitBlt(0,-1,hdc,hdcMine,BLANKD);
					MineBitBlt(1,-1,hdc,hdcMine,BLANKD);
				}
			}
			if(int(nPosX/BLOCKWIDTH)>=1)
			{
				MineBitBlt(-1,0,hdc,hdcMine,BLANKD);
				if(int(nPosY/BLOCKWIDTH)<M_WIDTH-1)
				{
					MineBitBlt(0,1,hdc,hdcMine,BLANKD);
					MineBitBlt(-1,1,hdc,hdcMine,BLANKD);
				}
				if(int(nPosY/BLOCKWIDTH)>=1)
				{
					MineBitBlt(0,-1,hdc,hdcMine,BLANKD);
					MineBitBlt(-1,-1,hdc,hdcMine,BLANKD);
				}
			}
		}
		else if(msg.wParam == MK_LBUTTON)
			MineBitBlt(0,0,hdc,hdcMine,BLANKD);
	}
	nPosXP = nPosX;// - nObgX;		
	nPosYP = nPosY;// - nObgY;
	return 0;
}
int MineBitBlt(int nX,int nY,HDC hdc,HDC hdcMine,int nOpType)
{
	if( ((anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY]&2) == 0) || ((anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY]&4) != 0) || ((anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY]&8) != 0) )
	{
		if((nOpType==BLANKD) && (anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY]&4) != 0)
			return 0;
		if(nOpType != BLANKD && nOpType != BLANK && nOpType != QUES)
			anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY] |= 2;
		if(nOpType == FLAG)
		{
			anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY] |= 4;
			anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY] |= 2;
			nFlag++;
		}
		else if(nOpType == QUES)
		{
			anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY] &= 0xFFFFFFF9;
			anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY] |= 8;
			nFlag--;
		}
		else if(nOpType == BLANK)
		{
			if((anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY]&4) != 0)
				return 0;
			anMine[int(nPosX/BLOCKWIDTH)+nX][int(nPosY/BLOCKWIDTH)+nY] &= 0xFFFFFFF1;
		}
		BitBlt(hdc,nObgX+(int(nPosX/BLOCKWIDTH)+nX)*BLOCKWIDTH,nObgY+(int(nPosY/BLOCKWIDTH)+nY)*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,nOpType*BLOCKWIDTH,SRCCOPY);
		return 0;
	}
	else
		return 1;
}

int MineBitBltRec(int nX,int nY,HDC hdc,HDC hdcMine)
{
	if(nPosXP<0||nPosXP>M_LENGTH*BLOCKWIDTH)
		return 0;
	if(nPosYP<0||nPosYP>M_WIDTH*BLOCKWIDTH)
		return 0;
	if((anMine[int(nPosXP/BLOCKWIDTH)+nX][int(nPosYP/BLOCKWIDTH)+nY]&2) == 0 && (anMine[int(nPosXP/BLOCKWIDTH)+nX][int(nPosYP/BLOCKWIDTH)+nY]&8) == 0)
	{
		BitBlt(hdc,nObgX+(int(nPosXP/BLOCKWIDTH)+nX)*BLOCKWIDTH,nObgY+(int(nPosYP/BLOCKWIDTH)+nY)*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,BLANK*BLOCKWIDTH,SRCCOPY);	
		//anMine[int(nPosXP/BLOCKWIDTH)+nX][int(nPosYP/BLOCKWIDTH)+nY] |= 2;
	}
	if((anMine[int(nPosXP/BLOCKWIDTH)+nX][int(nPosYP/BLOCKWIDTH)+nY]&4) != 0)
		return 1;
	else
		return 0;
}

BOOL CheckIfMine(int x,int y,HDC hdc,HDC hdcMine)
{
	int nMineNum = 0;

	if((anMine[int(nPosX/BLOCKWIDTH)+x][int(nPosY/BLOCKWIDTH)+y]&2) !=0 )
	{
	//	MineBitBlt(x,y,hdc,hdcMine,BANG);
	//	AlDraw(x,y,hdc,hdcMine);
		return 0;
	}
	else if((anMine[int(nPosX/BLOCKWIDTH)+x][int(nPosY/BLOCKWIDTH)+y]&1) !=0 )
	{
		MineBitBlt(x,y,hdc,hdcMine,BANG);
		AlDraw(x,y,hdc,hdcMine,false);
		return 1;
	}
	else
		CheckAround(int(nPosX/BLOCKWIDTH)+x,int(nPosY/BLOCKWIDTH)+y,hdc,hdcMine);
	return 0;
}

int CheckAround(int ix,int iy,HDC hdc,HDC hdcMine)
{
	bool bU = false,bD = false;
	int nMineNum = 0;
	if((anMine[ix][iy]&1) !=0 )
		return 1;
	if((anMine[ix][iy]&2) !=0 )
		return 0;
	anMine[ix][iy] |= 2;
	
	if(ix+1<M_LENGTH)
	{
		nMineNum += (anMine[ix+1][iy+0]&1);		
		if(iy<M_WIDTH-1)		
		{
			if(!bU)
			{
				nMineNum += (anMine[ix+0][iy+1]&1);	
				bU = true;
			}
			nMineNum += (anMine[ix+1][iy+1]&1);	
		}
		if(iy>=1)
		{
			if(!bD)
			{
				nMineNum += (anMine[ix+0][iy-1]&1);	
				bD = true;
			}
			nMineNum += (anMine[ix+1][iy-1]&1);	
		}		
	}
	if(ix>=1)	
	{
		nMineNum += (anMine[ix-1][iy+0]&1);	
		if(iy<M_WIDTH-1)
		{		
			if(!bU)
			{
				nMineNum += (anMine[ix+0][iy+1]&1);	
				bU = true;
			}
			nMineNum += (anMine[ix-1][iy+1]&1);	
		}	
		if(iy>=1)
		{		
			if(!bD)
			{
				nMineNum += (anMine[ix+0][iy-1]&1);	
				bD = true;
			}
			nMineNum += (anMine[ix-1][iy-1]&1);	
		}	
	}
	bD = bU = false;
	if(nMineNum != 0)
	{
		BitBlt(hdc,nObgX+ix*BLOCKWIDTH,nObgY+iy*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,(15-nMineNum)*BLOCKWIDTH,SRCCOPY);
		anMine[ix][iy] |= (nMineNum<<16);
		nBlankReminNum--;
	}
	else
	{		
		BitBlt(hdc,nObgX+ix*BLOCKWIDTH,nObgY+iy*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,BLANKD*BLOCKWIDTH,SRCCOPY);	
		anMine[ix][iy] |= (-1<<16);
		nBlankReminNum--;
		//return 0;

		if(ix+1<M_LENGTH)
		{
			CheckAround(ix+1,iy+0,hdc,hdcMine);		
			if(iy<M_WIDTH-1)		
			{
				if(!bU)
				{
					CheckAround(ix+0,iy+1,hdc,hdcMine);//
					bU = true;
				}
				CheckAround(ix+1,iy+1,hdc,hdcMine);	
			}
			if(iy>=1)
			{
				if(!bD)
				{
					CheckAround(ix+0,iy-1,hdc,hdcMine);//			
					bD = true;
				}
				CheckAround(ix+1,iy-1,hdc,hdcMine);
			}		
		}
		if(ix>=1)	
		{
			CheckAround(ix-1,iy+0,hdc,hdcMine);		
			if(iy<M_WIDTH-1)
			{		
				if(!bU)
				{
					CheckAround(ix+0,iy+1,hdc,hdcMine);//
					bU = true;
				}
				CheckAround(ix-1,iy+1,hdc,hdcMine);			
			}	
			if(iy>=1)
			{		
				if(!bD)
				{
					CheckAround(ix+0,iy-1,hdc,hdcMine);//
					bD = true;
				}
				CheckAround(ix-1,iy-1,hdc,hdcMine);			
			}	
		}
	}


	//Sleep(100);
	return 0;
}
int DrawFrame(HWND hWnd,HDC hdcNum,HDC hdcFace,int nLineWidth1,int nLineWidth2,int nLineWidth3,int nBoardWidth)
{
	RECT rect;
	GetClientRect (hWnd, &rect) ;
	HDC hdc2;

	hdc2 = GetDC(hWnd);
	HBRUSH hPbg = CreateSolidBrush(RGB(192,192,192));
	GetClientRect (hWnd, &rect) ;
	FillRect(hdc2,&rect,hPbg);
	
	int nlObgX = rect.left;
	int nlObgY = rect.top;

	int x,y;

	HPEN hPen = CreatePen(PS_SOLID,nLineWidth1,RGB(255,255,255));//白线
	SelectObject ( hdc2 , hPen );
	MoveToEx(hdc2,rect.left,rect.top,0);
	LineTo(hdc2,rect.left,rect.bottom);
	MoveToEx(hdc2,rect.left,rect.top,0);
	LineTo(hdc2,rect.right,rect.top);
	
	//hPen = CreatePen(PS_SOLID,12,RGB(128,128,128));
	hPen = CreatePen(PS_SOLID,nLineWidth2,RGB(192,192,192));
	SelectObject ( hdc2 , hPen );
	MoveToEx(hdc2,rect.left+nLineWidth1+nLineWidth2/2,rect.top+nLineWidth1+nLineWidth2/2,0);
	LineTo(hdc2,rect.left+nLineWidth1+nLineWidth2/2,rect.bottom);
	MoveToEx(hdc2,rect.left+nLineWidth1+nLineWidth2/2,rect.top+nLineWidth1+nLineWidth2/2,0);
	LineTo(hdc2,rect.right,rect.top+nLineWidth1+nLineWidth2/2);
	DeleteObject (hPen) ;
	
	hPen = CreatePen(PS_SOLID,nLineWidth3,RGB(128,128,128));
	SelectObject ( hdc2 , hPen );
	MoveToEx(hdc2,rect.left+nLineWidth1+nLineWidth2+nLineWidth3/2,rect.top+nLineWidth1+nLineWidth2+nLineWidth3/2,0);
	LineTo(hdc2,rect.left+nLineWidth1+nLineWidth2+nLineWidth3/2,nBoardWidth+nLineWidth1+nLineWidth2);
	MoveToEx(hdc2,rect.left+nLineWidth1+nLineWidth2+nLineWidth3/2,rect.top+nLineWidth1+nLineWidth2+nLineWidth3/2,0);
	LineTo(hdc2,rect.right-nLineWidth2,rect.top+nLineWidth1+nLineWidth2+nLineWidth3/2);
	//SetDCBrushColor(hdc2,RGB(127,127,127));
	DeleteObject (hPen) ;

	hPen = CreatePen(PS_SOLID,nLineWidth3,RGB(255,255,255));
	SelectObject ( hdc2 , hPen );
	LineTo(hdc2,rect.right-nLineWidth2,nBoardWidth+nLineWidth1+nLineWidth2);
	LineTo(hdc2,rect.left+nLineWidth1+nLineWidth2+nLineWidth3/2,nBoardWidth+nLineWidth1+nLineWidth2);
	DeleteObject (hPen) ;
	
	nlObgX = nLineWidth1+nLineWidth2+nLineWidth1/2;
	nlObgY = nBoardWidth+nLineWidth1+2*nLineWidth2+2*nLineWidth3;
	hPen = CreatePen(PS_SOLID,nLineWidth3,RGB(128,128,128));
	SelectObject ( hdc2 , hPen );
	MoveToEx(hdc2,nlObgX,nlObgY,0);
	LineTo(hdc2,nlObgX,nlObgY+BLOCKWIDTH*M_WIDTH+nLineWidth3);
	MoveToEx(hdc2,nlObgX,nlObgY,0);
	LineTo(hdc2,nlObgX+BLOCKWIDTH*M_LENGTH+nLineWidth3,nlObgY);
	DeleteObject (hPen) ;

	hPen = CreatePen(PS_SOLID,nLineWidth3,RGB(255,255,255));
	SelectObject ( hdc2 , hPen );
	MoveToEx(hdc2,nlObgX+BLOCKWIDTH*M_LENGTH+nLineWidth3+nLineWidth3/2,nlObgY,0);
	LineTo(hdc2,nlObgX+BLOCKWIDTH*M_LENGTH+nLineWidth3+nLineWidth3/2,nlObgY+BLOCKWIDTH*M_WIDTH+nLineWidth3+nLineWidth3/2);
	//MoveToEx(hdc2,nObgX+BLOCKWIDTH*M_LENGTH+nLineWidth3/2,nObgY+BLOCKWIDTH*M_WIDTH+nLineWidth3,0);
	LineTo(hdc2,nlObgX,nlObgY+BLOCKWIDTH*M_WIDTH+nLineWidth3+nLineWidth3/2);

	x = rect.left+nLineWidth1+nLineWidth2+nLineWidth3+4;
	y = rect.top+nLineWidth1+nLineWidth2+nLineWidth3+4;
	hPen = CreatePen(PS_SOLID,1,RGB(128,128,128));
	SelectObject ( hdc2 , hPen );
	MoveToEx(hdc2,x,y,0);
	LineTo(hdc2,x,y+NUMLENGTH);
	MoveToEx(hdc2,x,y,0);
	LineTo(hdc2,x+3*NUMWIDTH,y);
	//SetDCBrushColor(hdc2,RGB(127,127,127));
	DeleteObject (hPen) ;

	hPen = CreatePen(PS_SOLID,1,RGB(255,255,255));
	SelectObject ( hdc2 , hPen );
	LineTo(hdc2,x+3*NUMWIDTH,y+NUMLENGTH);
	LineTo(hdc2,x,y+NUMLENGTH);
	DeleteObject (hPen) ;

	//地雷数目贴图
	nFbgX = x;nFbgY = y;				
	if((MINE_NUM-nFlag)<0)
	{
		BitBlt(hdc2,nFbgX,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,MINUS*NUMLENGTH,SRCCOPY);
		BitBlt(hdc2,nFbgX+NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((nFlag-MINE_NUM)%100)/10)*NUMLENGTH,SRCCOPY);
		BitBlt(hdc2,nFbgX+2*NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nFlag-MINE_NUM)%10)*NUMLENGTH,SRCCOPY);
	}
	BitBlt(hdc2,nFbgX,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((MINE_NUM-nFlag)%1000)/100)*NUMLENGTH,SRCCOPY);
	BitBlt(hdc2,nFbgX+NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((MINE_NUM-nFlag)%100)/10)*NUMLENGTH,SRCCOPY);
	BitBlt(hdc2,nFbgX+2*NUMWIDTH,nFbgY,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-((MINE_NUM-nFlag)%10))*NUMLENGTH,SRCCOPY);

	x = rect.right-nLineWidth2-nLineWidth3-4-3*NUMWIDTH;
	y = rect.top+nLineWidth1+nLineWidth2+nLineWidth3+4;
	hPen = CreatePen(PS_SOLID,1,RGB(128,128,128));
	SelectObject ( hdc2 , hPen );
	MoveToEx(hdc2,x,y,0);
	LineTo(hdc2,x,y+NUMLENGTH);
	MoveToEx(hdc2,x,y,0);
	LineTo(hdc2,x+3*NUMWIDTH,y);
	//SetDCBrushColor(hdc2,RGB(127,127,127));
	DeleteObject (hPen) ;

	hPen = CreatePen(PS_SOLID,1,RGB(255,255,255));
	SelectObject ( hdc2 , hPen );
	LineTo(hdc2,rect.right-nLineWidth2-nLineWidth3-4,rect.top+nLineWidth1+nLineWidth2+nLineWidth3+4+NUMLENGTH);
	LineTo(hdc2,rect.right-nLineWidth2-nLineWidth3-4-3*NUMWIDTH,rect.top+nLineWidth1+nLineWidth2+nLineWidth3+4+NUMLENGTH);
	DeleteObject (hPen) ;
	//时间贴图
	nNbgX = x;nNbgY = y;
	if(nTime<0)
	{
		BitBlt(hdc2,x,y,NUMWIDTH,NUMLENGTH,hdcNum,0,MINUS*NUMLENGTH,SRCCOPY);
		BitBlt(hdc2,x+NUMWIDTH,y,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%100)/10)*NUMLENGTH,SRCCOPY);
		BitBlt(hdc2,x+2*NUMWIDTH,y,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%10))/10*NUMLENGTH,SRCCOPY);
	}
	else
	{
		BitBlt(hdc2,x,y,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%1000)/100)*NUMLENGTH,SRCCOPY);
		BitBlt(hdc2,x+NUMWIDTH,y,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%100)/10)*NUMLENGTH,SRCCOPY);
		BitBlt(hdc2,x+2*NUMWIDTH,y,NUMWIDTH,NUMLENGTH,hdcNum,0,(11-(nTime%10))*NUMLENGTH,SRCCOPY);
	}
	if(nBlankReminNum == MINE_NUM && bIsLose == false)
		BitBlt(hdc2,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,y,FACEWIDTH,FACEWIDTH,hdcFace,0,WIN*FACEWIDTH,SRCCOPY);
	else if(bIsLose == true)
		BitBlt(hdc2,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,y,FACEWIDTH,FACEWIDTH,hdcFace,0,LOSE*FACEWIDTH,SRCCOPY);
	else
		BitBlt(hdc2,(nNbgX+nFbgX+3*NUMWIDTH)/2-FACEWIDTH/2,y,FACEWIDTH,FACEWIDTH,hdcFace,0,SMILE*FACEWIDTH,SRCCOPY);

	nObgX = nlObgX + nLineWidth3;
	nObgY = nlObgY + nLineWidth3;
	
	ReleaseDC(hWnd,hdc2);
    DeleteObject (hPen) ;
	return 0;
}
int ReDraw(HDC hdc,HDC hdcMine,HDC hdcNum,HDC hdcFace,HWND hWnd,int nLineWidth1,int nLineWidth2,int nLineWidth3,int nBoardWidth)
{
	DrawFrame(hWnd,hdcNum,hdcFace,nLineWidth1,nLineWidth2,nLineWidth3,nBoardWidth);
	for(int i=0;i<M_LENGTH;i++)
	{
		for(int j=0;j<M_WIDTH;j++)
		{
			if(((anMine[i][j]&1) == 0)&&((anMine[i][j]&2) != 0)&&((anMine[i][j]&4) == 0))
			{
				if((anMine[i][j]>>16) == -1)
					BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,BLANKD*BLOCKWIDTH,SRCCOPY);
				else if((anMine[i][j]>>16)>=1&&(anMine[i][j]>>16)<=8)
					BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,(15-(anMine[i][j]>>16))*BLOCKWIDTH,SRCCOPY);
//				else
//					MessageBox(0,L"1",0,0);
			}
			else if((anMine[i][j]&4) != 0)
				BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,FLAG*BLOCKWIDTH,SRCCOPY);
			else if((anMine[i][j]&8) != 0)
				BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,QUES*BLOCKWIDTH,SRCCOPY);
			else if((anMine[i][j]&2) == 0)
				BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,BLANK*BLOCKWIDTH,SRCCOPY);
//			else
//					MessageBox(0,L"2",0,0);
		}
	}
	return 0;
}
int AlDraw(int x,int y,HDC hdc,HDC hdcMine,bool bWinOrLose)
{
	if(bWinOrLose == false)
	{
		bIsLose = true;
		for(int i=0;i<M_LENGTH;i++)
		{
			for(int j=0;j<M_WIDTH;j++)
			{
				//anMine[i][j] |= 2;
				if( ((int((nPosXP)/BLOCKWIDTH)+x) == i) && ((int((nPosYP)/BLOCKWIDTH)+y == j)) )
					BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,BANG*BLOCKWIDTH,SRCCOPY);
				else if((anMine[i][j]&4) != 0 && (anMine[i][j]&1) == 0 )
					BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,MINEX*BLOCKWIDTH,SRCCOPY);
				else if((anMine[i][j]&1) != 0 )
				{
					if((anMine[i][j]&4) != 0)
						BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,FLAG*BLOCKWIDTH,SRCCOPY);
					else 
					{
						anMine[i][j] &= 0xFFFFFFFB;
						BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,MINE*BLOCKWIDTH,SRCCOPY);
					}
				}
		//		else
		//			anMine[i][j] &= 0xFFFFFFFD;
			}
		}
	}
	else if(bWinOrLose == true)
	{
		for(int i=0;i<M_LENGTH;i++)
		{
			for(int j=0;j<M_WIDTH;j++)
			{
				if((anMine[i][j]&1) != 0 )
				{	
					anMine[i][j] |= 4;
					anMine[i][j] |= 2;
					BitBlt(hdc,nObgX+i*BLOCKWIDTH,nObgY+j*BLOCKWIDTH,BLOCKWIDTH,BLOCKWIDTH,hdcMine,0,FLAG*BLOCKWIDTH,SRCCOPY);
				}
			}
		}
	}
	return 0;
}