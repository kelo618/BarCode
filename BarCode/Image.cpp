/************************************************************
 * File: Image.cpp
 * Author: kelo
 * Created: 2026-08-30
 * Description:
 *   实现不可变条码符号的 PNG 像素布局、QPainter 绘制与原子文件提交。
 *   条边始终对齐整数像素并关闭抗锯齿，以保留扫描所需的清晰黑白边界。
 ************************************************************/

#include "Image.h"

#include <QImage>
#include <QImageWriter>
#include <QPainter>
#include <QSaveFile>
#include <QString>

#include <algorithm>
#include <stdexcept>

namespace barcode {
namespace {

/** @brief 判断当前条运行段是否与零售码护条区间相交。 */
bool isGuardRun(int firstModule, int moduleCount, const std::vector<GuardRange>& guardRanges)
{
    const int lastModule = firstModule + moduleCount; ///< 当前运行段的排他结束模块。
    // 任意护条区间与当前条重叠时，该条需要向下延长。
    return std::any_of(guardRanges.begin(), guardRanges.end(),
                       [firstModule, lastModule](const GuardRange& guardRange) {
                           return firstModule < guardRange.firstModule + guardRange.moduleCount
                               && lastModule > guardRange.firstModule;
                       });
}

/** @brief 从符号和像素选项建立完整黑白条码图像。 */
QImage renderImage(const BarcodeSymbol& symbol, const ImageOptions& options)
{
    // 非正几何无法形成可扫描图案，在分配图像前立即拒绝。
    if (options.moduleWidth <= 0 || options.barHeight <= 0 || options.verticalQuietModules < 0) {
        throw std::invalid_argument("Image dimensions must be positive and quiet zones must not be negative");
    }

    const int guardExtension = symbol.guardRanges().empty() ? 0 : options.moduleWidth * 3; ///< 护条比普通条增加的像素高度。
    const int verticalQuiet = options.verticalQuietModules * options.moduleWidth; ///< 上下留白的像素高度。
    const int width = (symbol.quietZoneLeftModules() + symbol.dataModuleCount()
                       + symbol.quietZoneRightModules()) * options.moduleWidth; ///< 左右标准静区与数据区的总宽度。
    const int height = verticalQuiet + options.barHeight + guardExtension + verticalQuiet; ///< 包含纵向静区、普通条和护条的总高度。

    QImage image(width, height, QImage::Format_ARGB32); ///< 不透明 PNG 像素画布。
    image.fill(Qt::white);
    image.setDotsPerMeterX(3780);
    image.setDotsPerMeterY(3780);

    QPainter painter(&image); ///< 画笔的生命期限定在当前图像渲染过程。
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    int x = symbol.quietZoneLeftModules() * options.moduleWidth; ///< 当前运行段的左侧像素坐标。
    int moduleOffset = 0; ///< 当前运行段在数据区内的模块起点。
    // 条空运行段已由核心保证交替且宽度为正，渲染层只转换为整数像素矩形。
    for (const ModuleRun& run : symbol.runs()) {
        if (run.isBar) {
            const int runHeight = options.barHeight
                + (isGuardRun(moduleOffset, run.modules, symbol.guardRanges()) ? guardExtension : 0); ///< 护条运行段使用延长高度。
            painter.drawRect(x, verticalQuiet, run.modules * options.moduleWidth, runHeight);
        }
        x += run.modules * options.moduleWidth;
        moduleOffset += run.modules;
    }

    painter.end();
    return image;
}

} // namespace

/** @brief 完成编码、PNG 绘制和不覆盖半成品的原子保存。 */
void savePng(const EncodeRequest& request, const std::string& filename, const ImageOptions& options)
{
    // 空路径无法表达本地目标，提前失败可避免 Qt 产生含糊的写入错误。
    if (filename.empty()) {
        throw std::invalid_argument("PNG output path must not be empty");
    }

    const BarcodeSymbol symbol = encode(request); ///< 核心层验证输入并生成不可变条空符号。
    const QImage image = renderImage(symbol, options); ///< 输出文件对应的完整 PNG 画布。
    QSaveFile outputFile(QString::fromUtf8(filename)); ///< 提交成功前保持旧目标不变的临时文件。
    if (!outputFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Unable to open PNG output: " + filename);
    }

    QImageWriter writer(&outputFile, "PNG"); ///< 直接对原子输出设备编码，不产生额外中间文件。
    if (!writer.write(image)) {
        outputFile.cancelWriting();
        throw std::runtime_error("Unable to encode PNG '" + filename + "': " + writer.errorString().toStdString());
    }
    if (!outputFile.commit()) {
        throw std::runtime_error("Unable to commit PNG output: " + filename);
    }
}

} // namespace barcode
