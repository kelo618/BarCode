/************************************************************
 * File: Core.h
 * Author: kelo
 * Created: 2026-08-30
 * Description:
 *   定义 BarCode 纯 C++17 核心的唯一公共接口，覆盖码制请求、GTIN、GS1 与不可变符号值。
 *   本文件只依赖标准库，渲染器、应用和 C ABI 均通过这些无状态函数共享同一编码语义。
 ************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace barcode {

/** @brief 可由核心编码器生成的一维条码制。 */
enum class Symbology {
    Ean8, Ean13, UpcA, UpcE, Itf14, Code39, Code93, Code128, Gs1_128, Codabar
};

/** @brief 明确 GTIN 完整号码长度，避免仅凭输入位数推断类型。 */
enum class GtinKind : std::size_t { Gtin8 = 8, Gtin12 = 12, Gtin13 = 13, Gtin14 = 14 };

/** @brief Code93 输入字符的解释方式。 */
enum class Code93Mode { Standard, FullAscii };

/** @brief Codabar 可选校验字符策略。 */
enum class CodabarChecksum { None, Modulo16 };

/** @brief GS1 文本入口的解释方式。 */
enum class Gs1InputFormat {
    Auto,          ///< 以左括号开头时按 HRI 解析，否则按元素串解析。
    Bracketed,     ///< 解析形如 (01)...(10)... 的 HRI 文本。
    ElementString  ///< 解析含 ASCII GS 分隔符的扫描元素串。
};

/** @brief 描述一个不含括号和分隔符的 GS1 Application Identifier 字段。 */
struct AiElement {
    std::string ai;    ///< 两至四位 AI 数字标识。
    std::string value; ///< 按对应 AI 语义保存的字段内容。
};

/** @brief 保存经过解析和校验且保持原始顺序的 GS1 元素集合。 */
struct Gs1Message {
    std::vector<AiElement> elements; ///< 按编码顺序排列的 AI/值集合。
};

/** @brief Code93 的码制专用选项。 */
struct Code93Options {
    Code93Mode mode = Code93Mode::Standard; ///< 默认只接受标准字符集。
};

/** @brief Codabar 的起止符、校验字符和 HRI 显示选项。 */
struct CodabarOptions {
    char start = 'A';                              ///< 起始符，只允许 A 至 D。
    char stop = 'B';                               ///< 终止符，只允许 A 至 D。
    CodabarChecksum checksum = CodabarChecksum::None; ///< 可选 Mod-16 校验。
    bool includeGuardsInHri = false;               ///< 是否在 HRI 中显示起止符。
};

/** @brief 所有码制专用选项的类型安全联合。 */
using SymbologyOptions = std::variant<std::monostate, Code93Options, CodabarOptions>;

/** @brief 一次无状态编码请求。 */
struct EncodeRequest {
    Symbology symbology = Symbology::Code128;     ///< 目标码制。
    std::variant<std::string, Gs1Message> payload; ///< 普通文本或结构化 GS1 消息。
    SymbologyOptions options;                      ///< 与码制匹配的可选行为。
};

/** @brief 连续条或空白的逻辑宽度，宽度单位为最窄模块。 */
struct ModuleRun {
    bool isBar = false; ///< true 表示前景条，false 表示背景空白。
    int modules = 0;    ///< 连续区域宽度，必须为正整数模块数。
};

/** @brief 条高不同于普通数据条的模块区间。 */
struct GuardRange {
    int firstModule = 0; ///< 区间起始模块，包含该位置。
    int moduleCount = 0; ///< 区间覆盖的连续模块数量。
};

namespace detail {
class SymbolBuilder;
}

/** @brief 编码器生成的不可变值；构造权仅属于核心实现，可安全并发只读共享。 */
class BarcodeSymbol final {
public:
    /** @brief 返回符号码制。 @return 编码时使用的码制。 */
    Symbology symbology() const noexcept { return symbolSymbology; }
    /** @brief 返回规范编码数据。 @return 包含自动校验位的数据。 */
    const std::string& canonicalData() const noexcept { return normalizedData; }
    /** @brief 返回可打印 HRI。 @return 不包含机器起止结构的显示文本。 */
    const std::string& humanReadableText() const noexcept { return hriText; }
    /** @brief 返回校验结果文字。 @return 无校验时为空。 */
    const std::string& checksum() const noexcept { return checksumText; }
    /** @brief 返回条空运行段。 @return 不含静区的只读序列。 */
    const std::vector<ModuleRun>& runs() const noexcept { return moduleRuns; }
    /** @brief 返回内部符号值。 @return 不适用时为空。 */
    const std::vector<int>& codewords() const noexcept { return internalCodewords; }
    /** @brief 返回护条范围。 @return 需要延长条高的模块区间。 */
    const std::vector<GuardRange>& guardRanges() const noexcept { return guards; }
    /** @brief 返回左静区要求。 @return 最窄模块数。 */
    int quietZoneLeftModules() const noexcept { return leftQuietZoneModules; }
    /** @brief 返回右静区要求。 @return 最窄模块数。 */
    int quietZoneRightModules() const noexcept { return rightQuietZoneModules; }
    /** @brief 返回数据区总宽度。 @return 不含静区的模块数。 */
    int dataModuleCount() const noexcept { return symbolModuleCount; }
    /** @brief 判断是否建议绘制 ITF-14 承载条。 @return 建议绘制时为 true。 */
    bool bearerBarsRecommended() const noexcept { return shouldDrawBearerBars; }

private:
    friend class detail::SymbolBuilder;

    /** @brief 保存已经校验的完整符号数据；调用方必须通过 encode() 获得实例。 */
    BarcodeSymbol(Symbology symbology,
                  std::string canonicalData,
                  std::string humanReadableText,
                  std::string checksum,
                  std::vector<ModuleRun> runs,
                  std::vector<int> codewords,
                  std::vector<GuardRange> guardRanges,
                  int quietZoneLeftModules,
                  int quietZoneRightModules,
                  bool bearerBarsRecommended);

    Symbology symbolSymbology;                 ///< 符号采用的码制。
    std::string normalizedData;                ///< 已补齐并验证校验位的规范数据。
    std::string hriText;                       ///< 打印时使用的人类可读文字。
    std::string checksumText;                  ///< 校验字符或数值的稳定文字表达。
    std::vector<ModuleRun> moduleRuns;          ///< 不含静区的条空连续段。
    std::vector<int> internalCodewords;         ///< 便于审计校验和及字符集切换的符号值。
    std::vector<GuardRange> guards;             ///< 零售码护条在数据模块中的范围。
    int leftQuietZoneModules = 0;               ///< 左侧标准静区模块数。
    int rightQuietZoneModules = 0;              ///< 右侧标准静区模块数。
    int symbolModuleCount = 0;                  ///< 构造时汇总的数据区模块数。
    bool shouldDrawBearerBars = false;          ///< 是否建议渲染承载条。
};

/** 可变长度码制的规范化输入安全上限。 */
inline constexpr std::size_t MaximumVariableInputBytes = 1024;
/** @brief 无状态编码请求。 @param request 目标码制、载荷和选项。 @return 不可变符号。 */
BarcodeSymbol encode(const EncodeRequest& request);
/** @brief 计算 GS1 Mod-10 校验位。 @param payload 不含校验位的数字载荷。 @return ASCII 校验数字。 */
char calculateGtinCheckDigit(std::string_view payload);
/** @brief 补齐或验证明确类型的 GTIN。 @param kind 完整类型。 @param digits 载荷或完整号码。 @return 完整 GTIN。 */
std::string normalizeGtin(GtinKind kind, std::string_view digits);
/** @brief 校验结构化 GS1 元素。 @param elements AI 字段。 @return 保持输入顺序的规范消息。 */
Gs1Message validateGs1(std::vector<AiElement> elements);
/** @brief 解析 GS1 文本。 @param text HRI 或元素串。 @param format 解释方式。 @return 规范消息。 */
Gs1Message parseGs1(std::string_view text, Gs1InputFormat format = Gs1InputFormat::Auto);
/** @brief 生成 GS1 括号 HRI。 @param message 已校验消息。 @return 可打印文本。 */
std::string gs1Hri(const Gs1Message& message);
/** @brief 生成含必要 ASCII GS 的元素串。 @param message 已校验消息。 @return 编码载荷。 */
std::string gs1ElementString(const Gs1Message& message);
/** @brief 将稳定英文码制名转换为枚举。 @param name 名称。 @return 对应码制。 */
Symbology parseSymbology(std::string_view name);
/** @brief 返回稳定英文码制名。 @param symbology 码制。 @return 配置名称。 */
std::string toString(Symbology symbology);

} // namespace barcode
