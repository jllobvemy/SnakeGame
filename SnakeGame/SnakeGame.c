// SnakeGame.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "SnakeGame.h"

#define MAX_LOADSTRING 100
#define RANK int
#define RANKNUM 10

// 方向
typedef enum DIRECTION
{
    UP, DOWN, LEFT, RIGHT
}DIRECTION;

// 节点  双链表
typedef struct NODE
{
    int x;
    int y;
    struct NODE* pred;
    struct NODE* succ;
}Snake, Apple;

// 全局变量:
HINSTANCE  hInst;                                                            // 当前实例
WCHAR      szTitle[MAX_LOADSTRING];                                          // 标题栏文本
WCHAR      szWindowClass[MAX_LOADSTRING];                                    // 主窗口类名
FILE*      fp;                                                               // 排行榜文件
RANK       rank[RANKNUM];                                                    // 排行榜数据
RANK       rCurrRank;                                                        // 当前分数
DIRECTION  derection        = RIGHT;                                         // 初始方向
BOOL       flag_gameover    = FALSE;
BOOL       flag_moveable    = TRUE;                                          // 防止一次性两个操作
BOOL       flag_defTimer    = FALSE;                                         // 控制暂停
Snake*     HEAD             = NULL;                                          // 蛇头
Snake*     TAIL             = NULL;                                          // 蛇尾
Apple      apple            = { 5, 6, NULL, NULL };                          // 苹果


//位图资源
HBITMAP hbBackGround,
        hbBody,
        hbApple,
		hbHeadLeft,
		hbHeadRight,
		hbHeadUp,
		hbHeadDown;
        

// 此代码模块中包含的函数的前向声明:
ATOM              MyRegisterClass (HINSTANCE hInstance);                   // 注册窗口类。
BOOL              InitInstance      (HINSTANCE, int);                      // 保存实例句柄并创建主窗口
LRESULT CALLBACK  WndProc           (HWND, UINT, WPARAM, LPARAM);          // 窗口过程
INT_PTR CALLBACK  About             (HWND, UINT, WPARAM, LPARAM);          // 关于对话框
BOOL    CALLBACK  RankList          (HWND, UINT, WPARAM, LPARAM);          // 排行榜对话框
BOOL    CALLBACK  GameHelp          (HWND, UINT, WPARAM, LPARAM);          // 操作说明对话框
// 游戏相关函数:
BOOL              ifTouchApple      ();                                    // 判断是否接触苹果
BOOL              ifTouchWall       ();                                    // 判断是否触墙
BOOL              ifTouchItself     ();                                    // 判断是否接触自己
void              AddNODE           (int x, int y);                        // 添加节点
void              ShowBG            (HDC hdc);                             // 绘制背景
void              ShowApple         (HDC hdc);                             // 绘制苹果
void              ShowSnake         (HDC hdc);                             // 绘制蛇
void              ShowRank          (HWND hwnd, HDC hdc);                  // 绘制分数
void              InitSnakeAndApple (HWND hWnd);                           // 初始化
void              Move              ();                                    // 蛇的移动
void              NewApple          ();                                    // 新建苹果
void              UpdateRank        ();                                    // 分数更新
void              DeleteSnake       ();                                    // 蛇的删除

// 入口：
int APIENTRY wWinMain(_In_    HINSTANCE  hInstance,
                     _In_opt_ HINSTANCE  hPrevInstance,
                     _In_     LPWSTR     lpCmdLine,
                     _In_     int        nCmdShow)
{
    int i;  //计数器
    UNREFERENCED_PARAMETER (hPrevInstance);
    UNREFERENCED_PARAMETER (lpCmdLine);

    //打开排行榜文件
    if ((fp = fopen ("RANKLIST.txt", "r+")) == NULL)
    {
        fp = fopen ("RANKLIST.txt", "w+");
        for (i = 0; i < RANKNUM; i++)
        {
            fputc ('0', fp);
            fputc ('\n', fp);
        }
        fflush(fp);                                  //文件创建后写入
    }
    i = 0;
    while (fscanf (fp, "%d", &rank[i++]) != EOF);    //读取文件

    // 初始化全局字符串
    LoadStringW (hInstance, IDS_APP_TITLE, szTitle,       MAX_LOADSTRING);
    LoadStringW (hInstance, IDC_SNAKEGAME, szWindowClass, MAX_LOADSTRING);

    //初始化位图资源
    hbBackGround  = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_BG2));
    hbApple       = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_APPLE));
    hbBody        = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_BODY));
    hbHeadUp      = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_HEAD_UP));
    hbHeadDown    = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_HEAD_DOWN));
    hbHeadLeft    = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_HEAD_LEFT));
    hbHeadRight   = LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_HEAD_RIGHT));


    srand ((unsigned int)time(0));
    MyRegisterClass (hInstance);
    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKEGAME));

    MSG msg;
    
    // 主消息循环:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    //关闭排行榜文件
    fclose (fp);
    DeleteSnake ();
    return (int) msg.wParam;
}



// 注册窗口类。
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKEGAME));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SNAKEGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// 保存实例句柄并创建主窗口
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(
       szWindowClass,                                      //lpClassName
       szTitle,                                            //lpWindowName
       WS_DLGFRAME | WS_SYSMENU,                           //dwStyle
       CW_USEDEFAULT,                                      //x
       0,                                                  //y
       610,                                                //nWidth
       660,                                                //nHeight
       NULL,                                               //hWndParent
       NULL,                                               //hMenu
       hInstance,                                          //hInstance
       NULL                                                //lParam
   );

   if (!hWnd)
   {
      return FALSE;
   }

   //SetTimer (hWnd, 1, 200, NULL);
   //flag_defTimer = TRUE;
   InitSnakeAndApple (hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// 处理主窗口的消息。
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC           hdc;
    PAINTSTRUCT   ps;

    switch (message)
    {
	case WM_CREATE:
		{
			int scrWidth, scrHeight;
			long height, width;
			RECT rect;
			//获得屏幕尺寸
			scrWidth = GetSystemMetrics (SM_CXSCREEN);
			scrHeight = GetSystemMetrics (SM_CYSCREEN);
			//取得窗口尺寸
			GetWindowRect (hWnd, &rect);
			//重新设置
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
			rect.left = (scrWidth - width) / 2;
			rect.top = (scrHeight - height) / 2;

			//移动窗口到指定的位置
			SetWindowPos (hWnd, HWND_TOP, rect.left, rect.top, width, height, SWP_NOSIZE | SWP_NOZORDER);
			break;
		}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox (hInst, MAKEINTRESOURCE (IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_Rank_List:
                DialogBox (hInst, MAKEINTRESOURCE (IDD_RANK_LIST), hWnd, RankList);
                break;
            case ID_GAME_HELP:
                DialogBox (hInst, MAKEINTRESOURCE (IDD_GAME_HELP), hWnd, GameHelp);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_NEW:
                hdc = BeginPaint (hWnd, &ps);
                UpdateRank ();
                DeleteSnake ();
                InitSnakeAndApple (hWnd);
                SetTimer (hWnd, 1, 200, NULL);
                flag_defTimer = TRUE;
                EndPaint (hWnd, &ps);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            hdc = BeginPaint (hWnd, &ps);
            ShowBG (hdc);
            ShowRank (hWnd, hdc);
            ShowSnake (hdc);
            ShowApple (hdc);
            EndPaint (hWnd, &ps);
        }
         break;
    case WM_TIMER:
        flag_moveable = TRUE;
        Move ();
        if (flag_gameover = ifTouchItself () || ifTouchWall ())
        {
            KillTimer (hWnd, 1);
            flag_defTimer = FALSE;
			UpdateRank ();
            MessageBox (hWnd, TEXT ("游戏结束"), TEXT("提示"), MB_OK);
            break;
        }
        if (ifTouchApple ())
        {
            NewApple ();
            AddNODE (-10, -10);
        }
        hdc = GetDC (hWnd);
        ShowBG (hdc);
        ShowRank (hWnd, hdc);
        ShowSnake (hdc);
        ShowApple (hdc);
        ReleaseDC (hWnd, hdc);
        break;
    case WM_KEYDOWN:
        if (flag_moveable == TRUE)
        {
            switch (wParam)
            {
            case VK_UP:
                if (derection != DOWN)
                {
                    derection = UP;
                }
                break;
            case VK_DOWN:
                if (derection != UP)
                {
                    derection = DOWN;
                }
                break;
            case VK_LEFT:
                if (derection != RIGHT)
                {
                    derection = LEFT;
                }
                break;
            case VK_RIGHT:
                if (derection != LEFT)
                {
                    derection = RIGHT;
                }
                break;
            }
        }
		if (VK_SPACE == wParam && flag_gameover == FALSE)
		{
			if (flag_defTimer == TRUE)
			{
				KillTimer (hWnd, 1);
				flag_defTimer = FALSE;
			}
			else
			{
				SetTimer (hWnd, 1, 200, NULL);
				flag_defTimer = TRUE;
			}
		}
		flag_moveable = FALSE;
        hdc = GetDC (hWnd);
        ShowBG (hdc);
        ShowRank (hWnd, hdc);
        ShowApple (hdc);
        ShowSnake (hdc);
        ReleaseDC (hWnd, hdc);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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

// "排行榜" 框的消息处理程序
BOOL CALLBACK RankList (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR szRank[100];
    static char buffer[100];
    static int cxChar, cyChar;
    static int iLength = 0, i = 0;
	HDC hdc;
	RECT rect;
	TEXTMETRIC tm;
	UNREFERENCED_PARAMETER (lParam);
    switch (message)
    {
    case WM_INITDIALOG:
         return TRUE;
        
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog (hDlg, LOWORD (wParam));
            return TRUE;
        }
        break;
    case WM_PAINT:

		hdc = GetDC (hDlg);
		GetClientRect (hDlg, &rect);
        GetTextMetrics (hdc, &tm);
        cxChar = tm.tmAveCharWidth;
        cyChar = tm.tmHeight + tm.tmExternalLeading;
        for (i = 0; i < RANKNUM; i++)
        {
            itoa (rank[i], buffer, 10);
            mbstowcs (szRank, buffer, strlen (buffer) + 1);                 //将char[]转为TCHAR[]
            iLength = lstrlen (szRank);
            TextOut (hdc, rect.left + 100, rect.top + cyChar * i + 40, szRank, iLength);
        }
		ReleaseDC (hDlg, hdc);
        break;
    }
    return FALSE;
}


// "游戏帮助" 框消息处理程序
BOOL CALLBACK GameHelp (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER (lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return FALSE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog (hDlg, LOWORD (wParam));
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL ifTouchApple ()
{
    if (HEAD->x == apple.x && HEAD->y == apple.y)
    {
        rCurrRank++;
        return TRUE;
    }
    return FALSE;
}

BOOL ifTouchWall ()
{
    if (HEAD->x == 20 || HEAD->x == -1 || HEAD->y == 20 || HEAD->y == -1)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL ifTouchItself ()
{
    Snake* body = HEAD->succ;
    while (body)
    {
        if (body->x == HEAD->x && body->y == HEAD->y)
        {
            return TRUE;
        }
        body = body->succ;
    }
    return FALSE;
}

// 更新分数
void UpdateRank ()
{
    int i, j, posi;
    char buffer[10], rankBuffer[100] = {'\0'};
    for (i = 0; i < RANKNUM; i++)
    {
        if (rank[i] < rCurrRank)
        {
            break;
        }
    }
    posi = i;
    i = 9;
    for (j = RANKNUM - 2; j >= posi; j--)
    {
        if (j == posi)
        {
            rank[posi] = rCurrRank;
            break;
        }
        else
        {
            rank[i--] = rank[j];
        }
    }
    for (i = 0; i < RANKNUM; i++)
    {
        itoa (rank[i], buffer, 10);
        strcat (rankBuffer, buffer);
        strcat (rankBuffer, "\n");
    }
    //下面这的做法是为了清除文件内容，没找到什么更好的办法
    fclose (fp);
	fp = fopen ("RANKLIST.txt", "w");
    fclose (fp);
	fp = fopen ("RANKLIST.txt", "w+");
	fprintf (fp, "%s", rankBuffer);
	fflush (fp);
}

void DeleteSnake ()
{
    Snake* head = HEAD;
    Snake* snake = HEAD;
    while (snake)
    {
        head = head->succ;
        free (snake);
        snake = head;
    }
    HEAD = NULL;
    TAIL = NULL;
}

void InitSnakeAndApple (HWND hWnd)
{
	if (flag_defTimer == TRUE)
	{
		KillTimer (hWnd, 1);
		flag_defTimer = FALSE;
	}
    derection = RIGHT;
    rCurrRank = 0;
    apple.x = 5;
    apple.y = 6;
    apple.succ = NULL;
    apple.pred = NULL;
    AddNODE (5, 5);
    AddNODE (4, 5);
    AddNODE (3, 5);
}

void AddNODE (int x, int y)
{
    Snake* tem = (Snake*)malloc (sizeof (Snake));
    tem->x = x;
    tem->y = y;
    tem->pred = NULL;
    tem->succ = NULL;
    if (HEAD == NULL)
    {
        HEAD = tem;
    }
    else
    {
        TAIL->succ = tem;
        tem->pred = TAIL;
    }
    TAIL = tem;
}

// 显示背景
void ShowBG (HDC hdc)
{
    HDC hdc_c = CreateCompatibleDC (hdc);
    SelectObject (hdc_c, hbBackGround);
    BitBlt (hdc, 0, 0, 600, 600, hdc_c, 0, 0, SRCCOPY);
    DeleteDC (hdc_c);
}

void ShowApple (HDC hdc)
{
    HDC hdc_c = CreateCompatibleDC (hdc);
    SelectObject (hdc_c, hbApple);
    BitBlt (hdc, apple.x * 30, apple.y * 30, 30, 30, hdc_c, 0, 0, SRCCOPY);
    DeleteDC (hdc_c);
}

void ShowSnake (HDC hdc)
{
    Snake* body = HEAD->succ;
    HDC hdc_c = CreateCompatibleDC (hdc);
    switch (derection)
    {
    case UP:
        SelectObject (hdc_c, hbHeadUp);
        break;
    case DOWN:
        SelectObject (hdc_c, hbHeadDown);
        break;
    case LEFT:
        SelectObject (hdc_c, hbHeadLeft);
        break;
    case RIGHT:
        SelectObject (hdc_c, hbHeadRight);
        break;
    }
    BitBlt (hdc, HEAD->x * 30, HEAD->y * 30, 30, 30, hdc_c, 0, 0, SRCCOPY);
    while (body)
    {
        SelectObject (hdc_c, hbBody);
        BitBlt (hdc, body->x * 30, body->y * 30, 30, 30, hdc_c, 0, 0, SRCCOPY);
        body = body->succ;
    }
    DeleteDC (hdc_c);
}

void ShowRank (HWND hwnd, HDC hdc)
{
    TCHAR buffer[15];
    RECT rect;
    GetClientRect (hwnd, &rect);
    swprintf_s (buffer, 15, TEXT ("分数：%d"), rCurrRank);
    SetBkMode (hdc, TRANSPARENT);
    TextOut (hdc, rect.right - 80, 0, buffer, lstrlen(buffer));
}

void Move ()
{
    Snake* body = TAIL;
    while (body != HEAD)
    {
        body->x = body->pred->x;
        body->y = body->pred->y;
        body = body->pred;
    }
    switch (derection)
    {
    case UP:
        HEAD->y--;
        break;
    case DOWN:
        HEAD->y++;
        break;
    case LEFT:
        HEAD->x--;
        break;
    case RIGHT:
        HEAD->x++;
        break;
    }
}

void NewApple ()
{
    Snake* body = HEAD;
    int x;
    int y;
    do
    {
        x = rand () % 18 + 1;
        y = rand () % 18 + 1;
        body = HEAD;
        while (body)
        {
            if (body->x == x && body->y == y)  //防止苹果与蛇身重合
                break;
            body = body->succ;
        }
    } while (body);
    apple.x = x;
    apple.y = y;
}

