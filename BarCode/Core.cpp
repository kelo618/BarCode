/************************************************************
 * File: Core.cpp
 * Author: kelo
 * Created: 2026-08-30
 * Description:
 *   实现不可变 BarcodeSymbol 的受控构造与核心不变量校验，集中保护静区和条空运行段。
 *   编码算法通过未安装的 SymbolBuilder 建立结果，公共调用方只能读取符号。
 ************************************************************/

#include "CoreInternal.h"

#include <stdexcept>
#include <utility>

namespace barcode {

/**
 * @brief 保存已经校验的符号数据并验证模块运行段。
 * @param symbology 符号码制。
 * @param canonicalData 规范编码数据。
 * @param humanReadableText 人类可读文本。
 * @param checksum 校验结果文字。
 * @param runs 条空模块运行段。
 * @param codewords 内部符号值。
 * @param guardRanges 护条范围。
 * @param quietZoneLeftModules 左静区模块数。
 * @param quietZoneRightModules 右静区模块数。
 * @param bearerBarsRecommended 是否建议承载条。
 */
BarcodeSymbol::BarcodeSymbol(Symbology symbology,
                             std::string canonicalData,
                             std::string humanReadableText,
                             std::string checksum,
                             std::vector<ModuleRun> runs,
                             std::vector<int> codewords,
                             std::vector<GuardRange> guardRanges,
                             int quietZoneLeftModules,
                             int quietZoneRightModules,
                             bool bearerBarsRecommended)
    : symbolSymbology(symbology),
      normalizedData(std::move(canonicalData)),
      hriText(std::move(humanReadableText)),
      checksumText(std::move(checksum)),
      moduleRuns(std::move(runs)),
      internalCodewords(std::move(codewords)),
      guards(std::move(guardRanges)),
      leftQuietZoneModules(quietZoneLeftModules),
      rightQuietZoneModules(quietZoneRightModules),
      shouldDrawBearerBars(bearerBarsRecommended)
{
    // 负静区会使渲染尺寸与质量判断出现分歧，因此在符号边界统一拒绝。
    if (leftQuietZoneModules < 0 || rightQuietZoneModules < 0) {
        throw std::invalid_argument("Quiet zone modules must not be negative");
    }

    bool hasPreviousRun = false; ///< 指示当前运行段是否有可比较的前驱。
    bool previousIsBar = false;  ///< 保存前一运行段的条空类型。
    // 运行段必须具有正宽度且严格交替，确保所有输出后端解释出同一图案。
    for (const ModuleRun& run : moduleRuns) {
        if (run.modules <= 0) {
            throw std::invalid_argument("Module run width must be positive");
        }
        if (hasPreviousRun && previousIsBar == run.isBar) {
            throw std::invalid_argument("Adjacent module runs must alternate bars and spaces");
        }
        symbolModuleCount += run.modules;
        previousIsBar = run.isBar;
        hasPreviousRun = true;
    }

    // 可扫描的一维符号至少需要一个具有宽度的运行段。
    if (moduleRuns.empty() || symbolModuleCount <= 0) {
        throw std::invalid_argument("Barcode symbol has no module data");
    }
}

namespace detail {

/**
 * @brief 将编码器已经校验的数据交给私有构造函数。
 * @return 保持全部核心不变量的不可变符号。
 */
BarcodeSymbol SymbolBuilder::make(Symbology symbology,
                                  std::string canonicalData,
                                  std::string humanReadableText,
                                  std::string checksum,
                                  std::vector<ModuleRun> runs,
                                  std::vector<int> codewords,
                                  std::vector<GuardRange> guardRanges,
                                  int quietZoneLeftModules,
                                  int quietZoneRightModules,
                                  bool bearerBarsRecommended)
{
    return BarcodeSymbol(symbology, std::move(canonicalData), std::move(humanReadableText),
                         std::move(checksum), std::move(runs), std::move(codewords),
                         std::move(guardRanges), quietZoneLeftModules, quietZoneRightModules,
                         bearerBarsRecommended);
}

} // namespace detail
} // namespace barcode
