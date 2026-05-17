#pragma once
#include <windows.h>

// 数据结构：单条记录
typedef struct {
	double mileage;    // 行驶里程(km)
	double amount;     // 加油金额(元)
} FuelRecord;

// 全局变量声明（在 main.cpp 中定义）
extern FuelRecord* g_records;
extern int g_recordCount;

// 在 main.cpp 中也会定义这些 HWND
extern HWND g_hListData;

// 功能函数
void ImportCSV(HWND hWnd);
void CalculateAvg(HWND hWnd);
void FreeRecords();
void AddToListView(HWND hListView, FuelRecord* records, int count);
