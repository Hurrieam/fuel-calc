# Fuel Consumption Calculator

A simple Windows desktop application for calculating average fuel cost (Yuan/km) based on imported CSV data. Built with Win32 API and C.

## Features

- Import CSV files containing mileage and fuel cost records
- Display imported data in a list view
- Calculate average cost per kilometer
- Support for UTF‑8 encoded CSV files with BOM

## Requirements

- Windows OS (Windows 7 or later)
- No additional runtime dependencies

## Build Instructions

You can compile the program using **Visual Studio** or **MinGW‑w64**.

### Using Visual Studio (Command Line)

```cmd
cl /EHsc /Fe:FuelCalculator.exe main.cpp /link user32.lib comctl32.lib comdlg32.lib
```

### Using MinGW‑w64

```bash
g++ -mwindows -o FuelCalculator.exe main.cpp -lcomctl32 -lcomdlg32
```

## How to Use

1. Launch `FuelCalculator.exe`
2. Click **Import CSV** and select a CSV file with the following format:
   - Two columns: `mileage (km)`, `amount (Yuan)`
   - Columns separated by commas
   - Example:
     ```
     120.5,80.0
     95.3,65.5
     ```
3. The imported records will appear in the table.
4. Click **Calculate Avg** to see the average cost per kilometer.

## CSV Format Rules

- First line can be data or header (the program does **not** auto‑skip headers)
- Empty lines are ignored
- Supports UTF‑8 encoding (including BOM)
- Decimal separator must be a dot (`.`)

## Interface Overview

| Component         | Description                            |
| ----------------- | -------------------------------------- |
| CSV File Path     | Displays the selected file path        |
| Import CSV        | Opens file dialog and loads data       |
| List View         | Shows mileage and amount per record    |
| Calculate Avg     | Computes average Yuan/km               |
| Result Label      | Shows the calculation result           |

## Notes

- The program expects **valid numeric values** in the CSV.
- If total mileage is zero, the calculation will be skipped.
- Data is kept in memory until a new file is imported or the program exits.

## Limitations

- Does not support custom decimal separators (e.g., comma as decimal point)
- No CSV header skipping (you must remove header lines manually)
- CSV parsing is basic; quoted fields are **not** supported

## License

Feel free to use, modify, and distribute this program as needed.