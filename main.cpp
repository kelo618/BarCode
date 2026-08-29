/************************************************************
 * File: main.cpp
 * Author: kelo
 * Created: 2026-08-30
 * Description:
 *   提供项目唯一的运行入口，将 Code128 文本编码并保存为本地 PNG 图片。
 *   命令行可覆盖默认数据和输出路径，成功后向终端输出图片的绝对路径。
 ************************************************************/

#include "BarCode/Image.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <exception>
#include <iostream>
#include <string>

/**
 * @brief 生成一张 Code128 PNG 图片并返回可用于脚本的退出码。
 * @param argc 命令行参数数量，可选接收条码文本和输出路径。
 * @param argv 由操作系统提供的参数数组，生命期覆盖本函数。
 * @return 图片成功写入时返回零，编码或文件失败时返回一。
 */
int main(int argc, char** argv)
{
    QCoreApplication application(argc, argv); ///< 为参数解码和 PNG 图像插件提供轻量 Qt 运行环境。

    const QStringList arguments = application.arguments(); ///< Qt 已按平台规则解码的命令行参数。
    const std::string barcodeText = arguments.size() > 1
        ? arguments.at(1).toUtf8().toStdString()
        : std::string("AB1234CD"); ///< 未提供数据时使用可直接扫描的默认样本。
    const QString outputPath = arguments.size() > 2
        ? arguments.at(2)
        : QDir::current().filePath(QStringLiteral("barcode.png")); ///< 默认将图片保存到当前工作目录。

    try {
        const barcode::EncodeRequest request{
            barcode::Symbology::Code128,
            barcodeText,
            std::monostate{}}; ///< 示例程序使用支持数字压缩和 A/B/C 切换的 Code128。
        barcode::savePng(request, outputPath.toUtf8().toStdString());
        const QString absolutePath = QFileInfo(outputPath).absoluteFilePath(); ///< 终端提示使用无歧义的绝对路径。
        std::cout << "Barcode saved to: " << absolutePath.toUtf8().constData() << '\n';
        return 0;
    } catch (const std::exception& error) {
        // 运行入口是异常跨出进程前的最后边界，将可读原因写到标准错误。
        std::cerr << "Unable to generate barcode: " << error.what() << '\n';
        return 1;
    }
}
