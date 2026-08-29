/************************************************************
 * File: CoreInternal.h
 * Author: kelo
 * Created: 2026-08-30
 * Description:
 *   定义核心编码实现可见的受控符号构造器，保持公共构造入口关闭。
 *   该头文件不安装，只有编码实现能将已验证运行段交给 BarcodeSymbol。
 ************************************************************/

#pragma once

#include "Core.h"

namespace barcode::detail {

/** @brief 为编码实现提供唯一的私有符号构造桥接。 */
class SymbolBuilder final {
public:
    /** @brief 构造并校验不可变符号。 @return 保持核心不变量的符号。 */
    static BarcodeSymbol make(Symbology symbology,
                              std::string canonicalData,
                              std::string humanReadableText,
                              std::string checksum,
                              std::vector<ModuleRun> runs,
                              std::vector<int> codewords,
                              std::vector<GuardRange> guardRanges,
                              int quietZoneLeftModules,
                              int quietZoneRightModules,
                              bool bearerBarsRecommended = false);
};

} // namespace barcode::detail
