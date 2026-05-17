//油耗计算程序 - 无中文字符串版本
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

// 控件ID
#define ID_BTN_IMPORT 1001
#define ID_BTN_CALC   1002
#define ID_EDIT_PATH  1003
#define ID_LIST_DATA  1004
#define ID_STATIC_RESULT 1005
// 将数据类型与逻辑拆到单独文件
#include "fuel.h"

// 全局变量（在此定义，fuel.h 中为 extern 声明）
HWND g_hWnd, g_hEditPath, g_hListData, g_hStaticResult;
WCHAR g_szFilePath[MAX_PATH] = { 0 };

FuelRecord* g_records = NULL;
int g_recordCount = 0;

// 窗口过程声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 初始化公共控件
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);

    // 注册窗口类
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"FuelCalculatorClass";

    if (!RegisterClassEx(&wc)) return 0;

    // 创建窗口
    g_hWnd = CreateWindowEx(0, L"FuelCalculatorClass", L"Fuel Consumption Calculator", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 500, NULL, NULL, hInstance, NULL);

    if (!g_hWnd) return 0;

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    FreeRecords();
    return (int)msg.wParam;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // 创建标签：文件路径
        CreateWindow(L"STATIC", L"CSV File Path:", WS_CHILD | WS_VISIBLE,
            10, 10, 100, 25, hWnd, NULL, GetModuleHandle(NULL), NULL);

        // 创建编辑框显示路径
        g_hEditPath = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
            120, 10, 350, 25, hWnd, (HMENU)ID_EDIT_PATH, GetModuleHandle(NULL), NULL);

        // 创建"导入CSV"按钮
        CreateWindow(L"BUTTON", L"Import CSV", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            480, 10, 100, 25, hWnd, (HMENU)ID_BTN_IMPORT, GetModuleHandle(NULL), NULL);

        // 创建ListView显示数据
        g_hListData = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
            10, 50, 560, 300, hWnd, (HMENU)ID_LIST_DATA, GetModuleHandle(NULL), NULL);

        // 设置ListView列
        LVCOLUMN lvc;
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
        lvc.pszText = (LPWSTR)L"Mileage(km)";
        lvc.cx = 200;
        ListView_InsertColumn(g_hListData, 0, &lvc);
        lvc.pszText = (LPWSTR)L"Amount(Yuan)";
        lvc.cx = 200;
        ListView_InsertColumn(g_hListData, 1, &lvc);

        // 创建"计算油耗"按钮
        CreateWindow(L"BUTTON", L"Calculate Avg", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 370, 120, 30, hWnd, (HMENU)ID_BTN_CALC, GetModuleHandle(NULL), NULL);

        // 创建结果显示区域
        g_hStaticResult = CreateWindow(L"STATIC", L"Average: Not calculated", WS_CHILD | WS_VISIBLE | SS_CENTER,
            150, 370, 400, 30, hWnd, (HMENU)ID_STATIC_RESULT, GetModuleHandle(NULL), NULL);
        // 设置字体稍大
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        SendMessage(g_hStaticResult, WM_SETFONT, (WPARAM)hFont, TRUE);
        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BTN_IMPORT:
            ImportCSV(hWnd);
            break;
        case ID_BTN_CALC:
            CalculateAvg(hWnd);
            break;
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

