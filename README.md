# BarCode

一个简洁的 C++17/Qt 条码图片生成程序。编码核心支持 EAN-8、EAN-13、UPC-A、UPC-E、ITF-14、Code39、Code93、Code128、GS1-128 和 Codabar；当前程序入口使用 Code128 并保存 PNG。

## 构建

需要 CMake 3.20+ 和 Qt 6.8+。在 Qt 开发环境中执行：

```powershell
qt-cmake -S . -B out/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/build
```

## 生成图片

不带参数运行时，程序会在当前目录生成 `barcode.png`，内容为 `AB1234CD`。

```powershell
./out/build/barcode.exe
```

也可传入条码数据和输出路径：

```powershell
./out/build/barcode.exe "ORDER-2026-001" "D:/output/order.png"
```

程序通过 `savePng()` 原子保存图片；输入、PNG 编码或文件写入失败时会返回非零退出码。

## 更新 GitHub

```powershell
./update-github.ps1 -Message "Update barcode generator"
```

脚本会显示变更、创建提交并推送当前分支到 `origin`，不使用强制推送。
