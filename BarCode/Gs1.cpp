/************************************************************
 * File: Gs1.cpp
 * Author: kelo
 * Created: 2026-08-29
 * Description:
 *   实现 GS1 AI 字典匹配、字段语义校验、关联约束以及三种消息表示之间的转换。
 *   规则表按 GS1 Syntax Dictionary 的 AI 分组固化，解析失败时不会返回部分消息或猜测可变字段边界。
 ************************************************************/

#include "Core.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace barcode {
namespace {

constexpr char GroupSeparator = '\x1D'; ///< GS1 可变长度字段使用的 ASCII GS 分隔符。

/** @brief GS1 字段长度、字符集和分隔约束的内部描述。 */
struct AiRule {
    int minimumLength;        ///< 字段允许的最短字符数。
    int maximumLength;        ///< 字段允许的最长字符数。
    bool numericOnly;         ///< 是否只接受十进制数字。
    bool fixedLength;         ///< 是否无需 FNC1/GS 终止字段。
};

/** @brief 将 AI 选择器与字段约束绑定为只读数据行。 */
struct RuleEntry {
    const char* selector; ///< 逗号分隔的精确值、数字范围或 n 通配模式。
    AiRule rule;          ///< 命中选择器后采用的字段约束。
};

/** @brief 查询固化的 GS1 AI 规则。 @param ai 不含括号的数字 AI。 @return 匹配规则。 */
AiRule ruleFor(const std::string& ai);

/** @brief 判断文本是否由 ASCII 数字组成。 @param text 待检查文本。 @return 全为数字且非空时为 true。 */
bool isDigits(const std::string& text)
{
    return !text.empty() && std::all_of(text.begin(), text.end(), [](char character) {
        return character >= '0' && character <= '9';
    });
}

/** @brief 判断单个选择器是否匹配 AI。 @param ai 实际 AI。 @param token 精确值、闭区间或 n 通配模式。 @return 命中时为 true。 */
bool matchesToken(std::string_view ai, std::string_view token)
{
    const std::size_t dash = token.find('-'); ///< 可选数字闭区间分隔位置。
    if (dash != std::string_view::npos) {
        const std::string_view first = token.substr(0, dash); ///< 区间下界文本。
        const std::string_view last = token.substr(dash + 1); ///< 区间上界文本。
        if (ai.size() != first.size() || first.size() != last.size()) return false;
        return ai >= first && ai <= last;
    }
    if (ai.size() != token.size()) return false;
    for (std::size_t index = 0; index < token.size(); ++index) {
        if (token[index] != 'n' && token[index] != ai[index]) return false;
        if (token[index] == 'n' && (ai[index] < '0' || ai[index] > '9')) return false;
    }
    return true;
}

/** @brief 判断逗号分隔选择器是否包含 AI。 @param ai 实际 AI。 @param selector 数据表选择器。 @return 任一分支命中时为 true。 */
bool matchesSelector(std::string_view ai, std::string_view selector)
{
    std::size_t offset = 0; ///< 当前尚未比较的选择器位置。
    while (offset <= selector.size()) {
        const std::size_t comma = selector.find(',', offset); ///< 当前分支终点。
        const std::size_t end = comma == std::string_view::npos ? selector.size() : comma; ///< 排他终点。
        if (matchesToken(ai, selector.substr(offset, end - offset))) return true;
        if (comma == std::string_view::npos) break;
        offset = comma + 1;
    }
    return false;
}

/** @brief 校验 YYMMDD 日期字段的月份和日期。 @param value 六位日期，日期 00 表示当月末日。 */
void validateDate(const std::string& value)
{
    if (value.size() != 6 || !isDigits(value)) {
        throw std::invalid_argument("GS1 date must use YYMMDD digits");
    }

    const int month = std::stoi(value.substr(2, 2)); ///< 日期中的月份。
    const int day = std::stoi(value.substr(4, 2)); ///< 日期中的日；00 具有 GS1 月末语义。
    if (month < 1 || month > 12 || day < 0 || day > 31) {
        throw std::invalid_argument("GS1 date contains an invalid month or day");
    }
}

/** @brief 判断规则是否为可变长度字段。 @param rule 已匹配规则。 @return 需要分隔符时为 true。 */
bool isVariable(const AiRule& rule)
{
    return !rule.fixedLength;
}

/** @brief 校验单个 AI 值的长度、字符集和已知语义。 @param element 待校验元素。 @param rule 匹配规则。 */
void validateElement(const AiElement& element, const AiRule& rule)
{
    const int length = static_cast<int>(element.value.size()); ///< 字段实际字节长度。
    if (length < rule.minimumLength || length > rule.maximumLength) {
        throw std::invalid_argument("GS1 AI " + element.ai + " has an invalid value length");
    }
    if (rule.numericOnly && !isDigits(element.value)) {
        throw std::invalid_argument("GS1 AI " + element.ai + " accepts digits only");
    }

    // 非纯数字字段限制为可打印 ASCII，避免把 GS 或控制字符混入字段内部。
    if (!rule.numericOnly) {
        for (char character : element.value) {
            const unsigned char byte = static_cast<unsigned char>(character); ///< 保留无符号字节以稳定检查高位字符。
            if (byte < 32 || byte > 126 || character == '(' || character == ')' || character == GroupSeparator) {
                throw std::invalid_argument("GS1 AI " + element.ai + " contains an unsupported character");
            }
        }
    }

    // GTIN 类字段使用统一 Mod-10 算法验证完整校验位。
    if (element.ai == "00") {
        const std::string payload = element.value.substr(0, 17);
        if (calculateGtinCheckDigit(payload) != element.value.back()) {
            throw std::invalid_argument("GS1 AI 00 has an invalid SSCC check digit");
        }
    } else if (element.ai == "01" || element.ai == "02") {
        static_cast<void>(normalizeGtin(GtinKind::Gtin14, element.value));
    }

    // 生产、包装、保质期和失效日期共享 YYMMDD 语义。
    if (element.ai == "11" || element.ai == "12" || element.ai == "13" ||
        element.ai == "15" || element.ai == "16" || element.ai == "17" || element.ai == "7006") {
        validateDate(element.value);
    }
}

/** @brief 检查消息级 AI 关联约束。 @param elements 已通过字段校验的元素。 */
void validateAssociations(const std::vector<AiElement>& elements)
{
    std::unordered_set<std::string> identifiers; ///< 保存消息内实际出现的 AI，便于常数时间关联查询。
    for (const AiElement& element : elements) {
        if (!identifiers.insert(element.ai).second) {
            throw std::invalid_argument("GS1 message contains duplicate AI " + element.ai);
        }
    }

    // AI 01 标识贸易项目，AI 02 标识容器内项目，两者不能描述同一层级。
    if (identifiers.count("01") != 0 && identifiers.count("02") != 0) {
        throw std::invalid_argument("GS1 AI 01 and AI 02 are mutually exclusive");
    }

    // 容器内项目与数量必须成对出现，防止生成缺少业务含义的物流标签。
    if ((identifiers.count("02") != 0) != (identifiers.count("37") != 0)) {
        throw std::invalid_argument("GS1 AI 02 and AI 37 must be used together");
    }
}

/** @brief 从当前位置匹配最长合法 AI。 @param text 元素串。 @param offset AI 起点。 @return 已匹配 AI。 */
std::string matchAi(const std::string& text, std::size_t offset)
{
    // AI 最长四位，优先最长匹配避免 310x 等规则被两位前缀截断。
    for (std::size_t length : std::array<std::size_t, 3>{4, 3, 2}) {
        if (offset + length > text.size()) {
            continue;
        }
        const std::string candidate = text.substr(offset, length); ///< 当前长度的候选 AI。
        try {
            static_cast<void>(ruleFor(candidate));
            return candidate;
        } catch (const std::invalid_argument&) {
            // 当前候选没有规则时缩短 AI 继续尝试，不把查询异常暴露为最终错误。
        }
    }
    throw std::invalid_argument("GS1 element string starts with an unknown AI");
}

/**
 * @brief 查询固化的 GS1 AI 规则。
 * @param ai 不含括号的数字 AI。
 * @return 字段长度、字符集和分隔属性。
 */
AiRule ruleFor(const std::string& ai)
{
    static const RuleEntry rules[] = {
        {"00", {18, 18, true, true}}, {"01,02,03", {14, 14, true, true}},
        {"10", {1, 20, false, false}}, {"11-13,15-17", {6, 6, true, true}},
        {"20", {2, 2, true, true}}, {"21,22", {1, 20, false, false}},
        {"235", {1, 28, false, false}}, {"240,241", {1, 30, false, false}},
        {"242", {1, 6, true, false}}, {"243", {1, 20, false, false}},
        {"250,251", {1, 30, false, false}}, {"253", {13, 30, false, false}},
        {"254", {1, 20, false, false}}, {"255", {13, 25, true, false}},
        {"30,37", {1, 8, true, false}}, {"3100-3699", {6, 6, true, true}},
        {"390n,392n", {1, 15, true, false}}, {"391n,393n", {4, 18, true, false}},
        {"394n", {4, 4, true, true}}, {"395n", {6, 6, true, true}},
        {"400,401,403", {1, 30, false, false}}, {"402", {17, 17, true, true}},
        {"410-417", {13, 13, true, true}}, {"420", {1, 20, false, false}},
        {"421", {4, 12, false, false}}, {"423", {3, 15, true, false}},
        {"422,424-426", {3, 3, true, true}}, {"427", {1, 3, false, false}},
        {"4300,4301,4310,4311,4320", {1, 35, false, false}},
        {"4302-4306,4312-4316", {1, 70, false, false}},
        {"4307,4317", {2, 2, false, true}}, {"4308,4309,4319", {1, 30, false, false}},
        {"4318", {1, 20, false, false}}, {"4321-4323", {1, 1, true, true}},
        {"4324,4325", {10, 10, true, true}}, {"4326", {6, 6, true, true}},
        {"4330-4333", {6, 7, false, false}}, {"7001", {13, 13, true, true}},
        {"7002", {1, 30, false, false}}, {"7003", {10, 10, true, true}},
        {"7004", {1, 4, true, false}}, {"7005", {1, 12, false, false}},
        {"7006", {6, 6, true, true}}, {"7007", {6, 12, true, false}},
        {"7008", {1, 3, false, false}}, {"7009", {1, 10, false, false}},
        {"7010", {1, 2, false, false}}, {"7011", {6, 10, true, false}},
        {"7020,7021", {1, 20, false, false}}, {"7022,7023", {1, 30, false, false}},
        {"703n", {4, 30, false, false}}, {"7040", {4, 4, false, true}},
        {"7041", {1, 4, false, false}}, {"710-717", {1, 20, false, false}},
        {"723n", {3, 30, false, false}}, {"7240", {1, 20, false, false}},
        {"7241", {2, 2, true, true}}, {"7242", {1, 25, false, false}},
        {"7250-7259", {1, 40, false, false}}, {"8001", {14, 14, true, true}},
        {"8002", {1, 20, false, false}}, {"8003", {14, 30, false, false}},
        {"8004", {1, 30, false, false}}, {"8005", {6, 6, true, true}},
        {"8006", {18, 18, true, true}}, {"8007", {1, 34, false, false}},
        {"8008", {8, 12, true, false}}, {"8010", {1, 30, false, false}},
        {"8011", {1, 12, true, false}}, {"8012,8013", {1, 30, false, false}},
        {"8017,8018", {18, 18, true, true}}, {"8019", {1, 10, true, false}},
        {"8020", {1, 25, false, false}}, {"8110,8112,8200", {1, 70, false, false}},
        {"8111", {4, 4, true, true}}, {"90", {1, 30, false, false}},
        {"91-99", {1, 90, false, false}}
    }; ///< 固定修订 AI 规则的唯一数据表。

    for (const RuleEntry& entry : rules) {
        if (matchesSelector(ai, entry.selector)) return entry.rule;
    }

    throw std::invalid_argument("Unknown GS1 Application Identifier: " + ai);
}

/**
 * @brief 校验结构化元素。
 * @param elements 调用方元素集合。
 * @return 保留顺序的规范消息。
 */
Gs1Message validateElements(std::vector<AiElement> elements)
{
    if (elements.empty()) {
        throw std::invalid_argument("GS1 message must contain at least one AI");
    }

    // 每个字段先独立匹配官方分组规则，消息级关联在字段全部有效后执行。
    for (const AiElement& element : elements) {
        if (element.ai.size() < 2 || element.ai.size() > 4 || !isDigits(element.ai)) {
            throw std::invalid_argument("GS1 AI must contain two to four digits");
        }
        const AiRule rule = ruleFor(element.ai); ///< 当前 AI 的固化字典规则。
        validateElement(element, rule);
    }
    validateAssociations(elements);

    return Gs1Message{std::move(elements)};
}

/**
 * @brief 解析括号 HRI 表示。
 * @param text 以括号 AI 开始的输入。
 * @return 规范消息。
 */
Gs1Message parseBracketed(const std::string& text)
{
    std::vector<AiElement> elements; ///< 按文本顺序累积解析结果。
    std::size_t offset = 0; ///< 当前尚未消费的文本位置。
    while (offset < text.size()) {
        if (text[offset] != '(') {
            throw std::invalid_argument("GS1 bracketed text must start every AI with '('");
        }
        const std::size_t closing = text.find(')', offset + 1); ///< 当前 AI 的右括号位置。
        if (closing == std::string::npos) {
            throw std::invalid_argument("GS1 bracketed text has an unterminated AI");
        }
        const std::string ai = text.substr(offset + 1, closing - offset - 1); ///< 不含括号的 AI。
        const std::size_t next = text.find('(', closing + 1); ///< 下一字段起点或文本末尾。
        const std::size_t valueEnd = next == std::string::npos ? text.size() : next; ///< 当前字段值终点。
        elements.push_back(AiElement{ai, text.substr(closing + 1, valueEnd - closing - 1)});
        offset = valueEnd;
    }

    return validateElements(std::move(elements));
}

/**
 * @brief 解析含 GS 分隔符的元素串。
 * @param text 可带 ]C1 前缀的扫描数据。
 * @return 规范消息。
 */
Gs1Message parseElementString(const std::string& text)
{
    const std::string payload = text.rfind("]C1", 0) == 0 ? text.substr(3) : text; ///< 移除可选扫描标识符后的载荷。
    if (payload.empty()) {
        throw std::invalid_argument("GS1 element string is empty");
    }

    std::vector<AiElement> elements; ///< 按编码顺序保存解析字段。
    std::size_t offset = 0; ///< 当前 AI 起点。
    while (offset < payload.size()) {
        if (payload[offset] == GroupSeparator) {
            throw std::invalid_argument("GS1 element string contains an empty field");
        }
        const std::string ai = matchAi(payload, offset); ///< 最长匹配得到的 AI。
        const AiRule rule = ruleFor(ai); ///< 当前字段长度和分隔规则。
        offset += ai.size();

        std::size_t valueEnd = offset; ///< 当前字段值的排他终点。
        if (rule.fixedLength) {
            valueEnd = offset + static_cast<std::size_t>(rule.maximumLength);
            if (valueEnd > payload.size()) {
                throw std::invalid_argument("GS1 fixed-length field is truncated");
            }
        } else {
            const std::size_t separator = payload.find(GroupSeparator, offset); ///< 可变字段终止 GS。
            valueEnd = separator == std::string::npos ? payload.size() : separator;
        }

        elements.push_back(AiElement{ai, payload.substr(offset, valueEnd - offset)});
        offset = valueEnd;
        if (offset < payload.size() && payload[offset] == GroupSeparator) {
            ++offset;
        }
    }

    return validateElements(std::move(elements));
}

/** @brief 生成括号 HRI。 @param message 已校验消息。 @return 括号文本。 */
std::string formatBracketed(const Gs1Message& message)
{
    const Gs1Message validated = validateElements(message.elements); ///< 防止调用方绕过结构化校验。
    std::string result; ///< 累积最终 HRI。
    for (const AiElement& element : validated.elements) {
        result += '(' + element.ai + ')' + element.value;
    }
    return result;
}

/** @brief 生成编码元素串。 @param message 已校验消息。 @return 带必要 GS 的文本。 */
std::string formatElementString(const Gs1Message& message)
{
    const Gs1Message validated = validateElements(message.elements); ///< 确保字段规则和关联约束仍成立。
    std::string result; ///< 累积 AI、字段值和必要 GS。
    for (std::size_t index = 0; index < validated.elements.size(); ++index) {
        const AiElement& element = validated.elements[index]; ///< 当前需要序列化的元素。
        const AiRule rule = ruleFor(element.ai); ///< 判断是否需要字段终止符。
        result += element.ai + element.value;
        if (isVariable(rule) && index + 1 < validated.elements.size()) {
            result.push_back(GroupSeparator);
        }
    }
    return result;
}

} // namespace

/** @brief 校验结构化 GS1 元素。 @param elements 调用方字段。 @return 规范消息。 */
Gs1Message validateGs1(std::vector<AiElement> elements)
{
    return validateElements(std::move(elements));
}

/** @brief 按指定或自动格式解析 GS1 文本。 @param text 输入文本。 @param format 解释方式。 @return 规范消息。 */
Gs1Message parseGs1(std::string_view text, Gs1InputFormat format)
{
    const std::string value(text); ///< 内部解析需要稳定的拥有型字符串。
    const Gs1InputFormat resolved = format == Gs1InputFormat::Auto
        ? (!value.empty() && value.front() == '(' ? Gs1InputFormat::Bracketed : Gs1InputFormat::ElementString)
        : format; ///< 自动模式只依据第一个字符选择无歧义语法。
    return resolved == Gs1InputFormat::Bracketed ? parseBracketed(value) : parseElementString(value);
}

/** @brief 生成括号 HRI。 @param message 已校验消息。 @return 可打印文本。 */
std::string gs1Hri(const Gs1Message& message)
{
    return formatBracketed(message);
}

/** @brief 生成含必要分隔符的元素串。 @param message 已校验消息。 @return 编码载荷。 */
std::string gs1ElementString(const Gs1Message& message)
{
    return formatElementString(message);
}

} // namespace barcode
