#include "fuel.h"
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>
#include <wchar.h>

// 引用 main 中的全局路径与结果控件
extern HWND g_hEditPath;
extern HWND g_hStaticResult;
extern WCHAR g_szFilePath[MAX_PATH];

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
