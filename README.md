# BarCode

A C++17 barcode generation project based on OpenCV.

## Supported barcode types

- `EAN-8` (7-digit input, auto check digit)
- `EAN-13` (12-digit input, auto check digit)
- `Code39`
- `Code128` (A/B/C switching logic)

## Project layout

- `BarCode/BarCode.h`, `BarCode/BarCode.cpp`: shared base classes and rendering flow.
- `BarCode/ean8.*`, `BarCode/ean13.*`: EAN implementations.
- `BarCode/code39.*`, `BarCode/code128.*`: Code implementations.
- `BarCode/BarcodeFactory.h`: barcode factory API.
- `BarCode/main.cpp`: demo entry.
- `CMakeLists.txt`: library + demo build entry (`barcode_core` + `barcode_demo`).

## Build (Visual Studio solution)

1. Open `BarCode.sln`.
2. Configure OpenCV include/lib paths in `BarCode/BarCode.vcxproj` if needed.
3. Build `Debug|x64` or `Release|x64`.

The solution is split into three projects:

- `BarCode`: core static library.
- `BarCodeDemo`: demo executable.
- `BarCodeTests`: regression tests.

## Build (CMake)

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Basic usage

```cpp
#include "BarcodeFactory.h"
using namespace barcode;

auto barcode = BarcodeFactory::create<Code128>(BarcodeSize::STANDARD);
barcode->showLabels(false);
barcode->encode("KSGM6PQ7Q2410S0772");
barcode->save("barcode.png");
```

## Notes

- `show()` uses OpenCV GUI (`imshow` + `waitKey`), suitable for local desktop debugging.
- For server-side usage, prefer `encode()` + `save()` or `getImage()`.

## Regression tests

Build `BarCodeTests` and run:

```powershell
.\x64\Debug\BarCodeTestsd.exe
```

or

```powershell
.\x64\Release\BarCodeTests.exe
```

Current minimal regression checks cover:

- check digit assertions (EAN-8/EAN-13)
- full pattern assertions (EAN-8/EAN-13)
- quiet-zone assertions (EAN-8/EAN-13/Code128)
- size profile assertions (`MINIMUM`/`STANDARD`/`LARGE`)
