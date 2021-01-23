// DierctSound_TEST.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "DierctSound_TEST.h"

#define MAX_LOADSTRING 100
#define IDM_DX           1255
#define IDM_LOAD         1245
#define IDM_FIND         1513

#define MAX_AUDIO_BUF 4 
#define BUFFERNOTIFYSIZE 192000 
int sample_rate = 44100;	//PCM sample rate
int channels = 2;			//PCM channel number
int bits_per_sample = 16;	//bits per sample


// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DIERCTSOUNDTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIERCTSOUNDTEST));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIERCTSOUNDTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DIERCTSOUNDTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR er[200];
    switch (message)
    {
    case WM_CREATE:
    {
        CreateWindowEx(0, L"BUTTON", L"PLAY", WS_CHILD | WS_VISIBLE, 10, 10, 50, 50, hWnd, (HMENU)IDM_DX, hInst, NULL);
        CreateWindowEx(0, L"BUTTON", L"LOAD", WS_CHILD | WS_VISIBLE, 70, 10, 50, 50, hWnd, (HMENU)IDM_LOAD, hInst, NULL);
        CreateWindowEx(0, L"BUTTON", L"FIND", WS_CHILD | WS_VISIBLE, 130, 10, 50, 50, hWnd, (HMENU)IDM_FIND, hInst, NULL);
    }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_DX:
            {
                HRESULT hr;
                hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
                IXAudio2* myAudio = NULL;
                UINT32 flag = 0;
#ifdef _DEBUG
                flag |= XAUDIO2_DEBUG_ENGINE;
#endif
                if (FAILED(hr = XAudio2Create(&myAudio, flag)))
                {                  
                    swprintf_s(er, 200,L"初始化失败%#X", hr);
                    MessageBox(hWnd, er, L"error", MB_OK);
                    CoUninitialize();
                    break;
                }
                IXAudio2MasteringVoice* voice = NULL;
                if (FAILED(hr = myAudio->CreateMasteringVoice(&voice)))
                {
                    swprintf_s(er, 200, L"初始化主声场失败%#X", hr);
                    MessageBox(hWnd, er, L"fail", MB_OK);
                    SAFE_RELEASE(myAudio);
                    CoUninitialize();
                    break;
                }
                WCHAR fName[255] = L"F:\\不必勉强.wav";
                PlayPCM(myAudio, fName);
            }
                break;
            case IDM_LOAD:
            {
                int i;
                FILE* fp;
                if ((fp = fopen("H:\\open44116le.pcm", "rb")) == NULL)
                {
                    MessageBox(hWnd, L"cannot open this file\n", L"error", MB_OK);
                    return -1;
                }

                IDirectSound8      * m_pDS        = NULL;
                IDirectSoundBuffer8* m_pDSBuffer8 = NULL;
                IDirectSoundBuffer * m_pDSBuffer  = NULL;
                IDirectSoundNotify8* m_pDSNotify  = NULL;
                DSBPOSITIONNOTIFY m_pDSPosNotify[MAX_AUDIO_BUF];
                HANDLE m_event[MAX_AUDIO_BUF];
                         
                if (FAILED(DirectSoundCreate8(NULL, &m_pDS, NULL)))
                    return FALSE;
                if (FAILED(m_pDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL)))
                    return FALSE;

                DSBUFFERDESC dsbd;
                memset(&dsbd, 0, sizeof(dsbd));
                dsbd.dwSize = sizeof(dsbd);
                dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
                dsbd.dwBufferBytes = MAX_AUDIO_BUF * BUFFERNOTIFYSIZE;
                dsbd.lpwfxFormat = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
                dsbd.lpwfxFormat->wFormatTag = WAVE_FORMAT_PCM;
               
                (dsbd.lpwfxFormat)->nChannels = channels;              
                (dsbd.lpwfxFormat)->nSamplesPerSec = sample_rate;            
                (dsbd.lpwfxFormat)->nAvgBytesPerSec = sample_rate * (bits_per_sample / 8) * channels;              
                (dsbd.lpwfxFormat)->nBlockAlign = (bits_per_sample / 8) * channels;              
                (dsbd.lpwfxFormat)->wBitsPerSample = bits_per_sample;             
                (dsbd.lpwfxFormat)->cbSize = 0;
              
                if (FAILED(m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuffer, NULL))) 
                {
                    return FALSE;
                }
                if (FAILED(m_pDSBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&m_pDSBuffer8))) 
                {
                    return FALSE;
                }          
                if (FAILED(m_pDSBuffer8->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&m_pDSNotify))) 
                {
                    return FALSE;
                }
                for (i = 0; i < MAX_AUDIO_BUF; i++) 
                {
                    m_pDSPosNotify[i].dwOffset = i * BUFFERNOTIFYSIZE;
                    m_event[i] = ::CreateEvent(NULL, false, false, NULL);
                    m_pDSPosNotify[i].hEventNotify = m_event[i];
                }
                m_pDSNotify->SetNotificationPositions(MAX_AUDIO_BUF, m_pDSPosNotify);
                m_pDSNotify->Release();

                BOOL isPlaying = TRUE;
                LPVOID buf = NULL;
                DWORD  buf_len = 0;
                DWORD res = WAIT_OBJECT_0;
                DWORD offset = BUFFERNOTIFYSIZE;

                m_pDSBuffer8->SetCurrentPosition(0);
                m_pDSBuffer8->Play(0, 0, DSBPLAY_LOOPING);
                //Loop
                while (isPlaying)
                {
                    if ((res >= WAIT_OBJECT_0) && (res <= WAIT_OBJECT_0 + 3)) 
                    {
                        m_pDSBuffer8->Lock(offset, BUFFERNOTIFYSIZE, &buf, &buf_len, NULL, NULL, 0);
                        if (fread(buf, 1, buf_len, fp) != buf_len) 
                        {
                            //File End
                            //Loop:
                            fseek(fp, 0, SEEK_SET);
                            fread(buf, 1, buf_len, fp);
                            //Close:
                            //isPlaying=0;
                        }
                        m_pDSBuffer8->Unlock(buf, buf_len, NULL, 0);
                        offset += buf_len;
                        offset %= (BUFFERNOTIFYSIZE * MAX_AUDIO_BUF);
                        printf("this is %7d of buffer\n", offset);
                    }
                    res = WaitForMultipleObjects(MAX_AUDIO_BUF, m_event, FALSE, INFINITE);
                }


            }
                break;
            case IDM_FIND:
            {
              
            }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
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
