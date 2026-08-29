/************************************************************
 * File: Image.h
 * Author: kelo
 * Created: 2026-08-30
 * Description:
 *   定义条码符号到本地 PNG 文件的最小渲染接口。
 *   调用方只需提供编码请求和目标路径，Qt 图像对象不会泄漏到公共边界。
 ************************************************************/

#pragma once

#include "Core.h"

#include <string>

namespace barcode {

/** @brief 描述 PNG 中最窄模块、条高和纵向留白。 */
struct ImageOptions {
    int moduleWidth = 3;       ///< 最窄条或空的像素宽度，必须大于零。
    int barHeight = 100;       ///< 普通数据条的像素高度，必须大于零。
    int verticalQuietModules = 4; ///< 图像上下两侧按模块计算的留白。
};

/**
 * @brief 编码、渲染并原子保存 PNG 文件。
 * @param request 码制、输入数据和码制专用选项。
 * @param filename 目标文件的 UTF-8 路径；父目录必须已存在。
 * @param options 最窄模块、条高和纵向留白选项。
 * @note 编码、PNG 写入或提交失败时抛出异常，原目标文件不会被部分覆盖。
 */
void savePng(const EncodeRequest& request,
             const std::string& filename,
             const ImageOptions& options = {});

} // namespace barcode
