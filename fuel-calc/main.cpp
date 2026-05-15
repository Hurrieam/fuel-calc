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

// 全局变量
HWND g_hWnd, g_hEditPath, g_hListData, g_hStaticResult;
WCHAR g_szFilePath[MAX_PATH] = { 0 };

// 数据结构：单条记录
typedef struct {
    double mileage;    // 行驶里程(km)
    double amount;     // 加油金额(元)
} FuelRecord;

FuelRecord* g_records = NULL;
int g_recordCount = 0;

// 函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ImportCSV(HWND hWnd);
void CalculateAvg(HWND hWnd);
void FreeRecords();
void AddToListView(HWND hListView, FuelRecord* records, int count);

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

// 导入CSV文件
void ImportCSV(HWND hWnd) {
    OPENFILENAME ofn = { 0 };
    WCHAR szFile[MAX_PATH] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"CSV Files\0*.csv\0All Files\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (!GetOpenFileName(&ofn)) return;

    wcscpy_s(g_szFilePath, szFile);
    SetWindowText(g_hEditPath, g_szFilePath);

    // 打开文件
    FILE* file;
    if (_wfopen_s(&file, g_szFilePath, L"r, ccs=UTF-8") != 0) {
        MessageBox(hWnd, L"Cannot open file", L"Error", MB_ICONERROR);
        return;
    }

    // 清空旧数据
    FreeRecords();
    ListView_DeleteAllItems(g_hListData);
    g_records = NULL;
    g_recordCount = 0;

    // 读取CSV
    WCHAR line[1024];
    int capacity = 10;
    g_records = (FuelRecord*)malloc(capacity * sizeof(FuelRecord));
    if (!g_records) {
        fclose(file);
        MessageBox(hWnd, L"Out of memory", L"Error", MB_ICONERROR);
        return;
    }

    WCHAR* context = NULL;
    int isFirstLine = 1;

    while (fgetws(line, 1024, file)) {
        // 跳过可能的BOM标记(第一行)
        if (isFirstLine && (line[0] == 0xFEFF)) {
            wcscpy_s(line, line + 1);
        }
        isFirstLine = 0;

        // 去除末尾换行符
        size_t len = wcslen(line);
        if (len > 0 && (line[len - 1] == L'\n' || line[len - 1] == L'\r')) {
            line[len - 1] = L'\0';
            len--;
        }
        if (len > 0 && (line[len - 1] == L'\n' || line[len - 1] == L'\r')) {
            line[len - 1] = L'\0';
        }

        // 跳过空行
        if (line[0] == L'\0') continue;

        // 按逗号分割
        WCHAR* token = wcstok_s(line, L",", &context);
        if (!token) continue;
        double mileage = _wtof(token);

        token = wcstok_s(NULL, L",", &context);
        if (!token) continue;
        double amount = _wtof(token);

        // 添加到数组
        if (g_recordCount >= capacity) {
            capacity *= 2;
            FuelRecord* newArr = (FuelRecord*)realloc(g_records, capacity * sizeof(FuelRecord));
            if (!newArr) break;
            g_records = newArr;
        }
        g_records[g_recordCount].mileage = mileage;
        g_records[g_recordCount].amount = amount;
        g_recordCount++;
    }
    fclose(file);

    // 显示到ListView
    AddToListView(g_hListData, g_records, g_recordCount);

    WCHAR msg[256];
    swprintf_s(msg, 256, L"Successfully imported %d records", g_recordCount);
    MessageBox(hWnd, msg, L"Information", MB_OK);
}

// 添加到ListView
void AddToListView(HWND hListView, FuelRecord* records, int count) {
    for (int i = 0; i < count; i++) {
        WCHAR buf1[64], buf2[64];
        swprintf_s(buf1, 64, L"%.2f", records[i].mileage);
        swprintf_s(buf2, 64, L"%.2f", records[i].amount);

        LVITEM lvi;
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = buf1;
        ListView_InsertItem(hListView, &lvi);
        ListView_SetItemText(hListView, i, 1, buf2);
    }
}

// 计算平均油耗
void CalculateAvg(HWND hWnd) {
    if (g_recordCount == 0) {
        MessageBox(hWnd, L"Please import CSV file first", L"Information", MB_ICONINFORMATION);
        return;
    }

    double totalMileage = 0;
    double totalAmount = 0;

    for (int i = 0; i < g_recordCount; i++) {
        totalMileage += g_records[i].mileage;
        totalAmount += g_records[i].amount;
    }

    if (totalMileage <= 0) {
        SetWindowText(g_hStaticResult, L"Average: Total mileage is 0, cannot calculate");
        return;
    }

    double avgCost = totalAmount / totalMileage;  // Yuan/km
    WCHAR result[256];
    swprintf_s(result, 256, L"Average: %.3f Yuan/km (Total: %.2fkm, Cost: %.2fYuan)",
        avgCost, totalMileage, totalAmount);
    SetWindowText(g_hStaticResult, result);
}

// 释放记录内存
void FreeRecords() {
    if (g_records) {
        free(g_records);
        g_records = NULL;
    }
    g_recordCount = 0;
}