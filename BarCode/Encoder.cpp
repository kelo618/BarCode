/************************************************************
 * File: Encoder.cpp
 * Author: kelo
 * Created: 2026-08-29
 * Description:
 *   实现全部一维码制的无状态编码算法，将规范数据转换为统一条空模块和内部码字。
 *   本文件集中处理 GTIN、Code128 字符集切换、FNC1、Code93 双校验及各码制起止结构，不执行图像操作。
 ************************************************************/

#include "CoreInternal.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace barcode {
namespace {

constexpr char GroupSeparator = '\x1D'; ///< GS1 可变字段在规范元素串中的分隔字节。

/** @brief Code128 的 107 个标准条空宽度模式。 */
const std::array<const char*, 107> Code128Patterns = {
    "212222","222122","222221","121223","121322","131222","122213","122312",
    "132212","221213","221312","231212","112232","122132","122231","113222",
    "123122","123221","223211","221132","221231","213212","223112","312131",
    "311222","321122","321221","312212","322112","322211","212123","212321",
    "232121","111323","131123","131321","112313","132113","132311","211313",
    "231113","231311","112133","112331","132131","113123","113321","133121",
    "313121","211331","231131","213113","213311","213131","311123","311321",
    "331121","312113","312311","332111","314111","221411","431111","111224",
    "111422","121124","121421","141122","141221","112214","112412","122114",
    "122411","142112","142211","241211","221114","413111","241112","134111",
    "111242","121142","121241","114212","124112","124211","411212","421112",
    "421211","212141","214121","412121","111143","111341","131141","114113",
    "114311","411113","411311","113141","114131","311141","411131","211412",
    "211214","211232","2331112"
};

/** @brief EAN 左侧奇校验数字模式。 */
const std::array<const char*, 10> EanL = {
    "0001101", "0011001", "0010011", "0111101", "0100011",
    "0110001", "0101111", "0111011", "0110111", "0001011"
};

/** @brief EAN 左侧偶校验数字模式。 */
const std::array<const char*, 10> EanG = {
    "0100111", "0110011", "0011011", "0100001", "0011101",
    "0111001", "0000101", "0010001", "0001001", "0010111"
};

/** @brief EAN 右侧数字模式。 */
const std::array<const char*, 10> EanR = {
    "1110010", "1100110", "1101100", "1000010", "1011100",
    "1001110", "1010000", "1000100", "1001000", "1110100"
};

/** @brief Code39 基础字符的宽窄模式。 */
const std::unordered_map<char, const char*> Code39Patterns = {
    {'0', "nnnwwnwnn"}, {'1', "wnnwnnnnw"}, {'2', "nnwwnnnnw"}, {'3', "wnwwnnnnn"},
    {'4', "nnnwwnnnw"}, {'5', "wnnwwnnnn"}, {'6', "nnwwwnnnn"}, {'7', "nnnwnnwnw"},
    {'8', "wnnwnnwnn"}, {'9', "nnwwnnwnn"}, {'A', "wnnnnwnnw"}, {'B', "nnwnnwnnw"},
    {'C', "wnwnnwnnn"}, {'D', "nnnnwwnnw"}, {'E', "wnnnwwnnn"}, {'F', "nnwnwwnnn"},
    {'G', "nnnnnwwnw"}, {'H', "wnnnnwwnn"}, {'I', "nnwnnwwnn"}, {'J', "nnnnwwwnn"},
    {'K', "wnnnnnnww"}, {'L', "nnwnnnnww"}, {'M', "wnwnnnnwn"}, {'N', "nnnnwnnww"},
    {'O', "wnnnwnnwn"}, {'P', "nnwnwnnwn"}, {'Q', "nnnnnnwww"}, {'R', "wnnnnnwwn"},
    {'S', "nnwnnnwwn"}, {'T', "nnnnwnwwn"}, {'U', "wwnnnnnnw"}, {'V', "nwwnnnnnw"},
    {'W', "wwwnnnnnn"}, {'X', "nwnnwnnnw"}, {'Y', "wwnnwnnnn"}, {'Z', "nwwnwnnnn"},
    {'-', "nwnnnnwnw"}, {'.', "wwnnnnwnn"}, {' ', "nwwnnnwnn"}, {'$', "nwnwnwnnn"},
    {'/', "nwnwnnnwn"}, {'+', "nwnnnwnwn"}, {'%', "nnnwnwnwn"}, {'*', "nwnnwnwnn"}
};

/** @brief Code93 字符顺序，索引即校验和使用的符号值。 */
const std::string Code93Alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%abcd*";

/** @brief Code93 每个符号的九模块二进制模式。 */
const std::array<const char*, 48> Code93Patterns = {
    "100010100","101001000","101000100","101000010","100101000","100100100",
    "100100010","101010000","100010010","100001010","110101000","110100100",
    "110100010","110010100","110010010","110001010","101101000","101100100",
    "101100010","100110100","100011010","101011000","101001100","101000110",
    "100101100","100010110","110110100","110110010","110101100","110100110",
    "110010110","110011010","101101100","101100110","100110110","100111010",
    "100101110","111010100","111010010","111001010","101101110","101110110",
    "110101110","100100110","111011010","111010110","100110010","101011110"
};

/** @brief Codabar 字符顺序，索引同时用于可选 Mod-16 校验。 */
const std::string CodabarAlphabet = "0123456789-$:/.+ABCD";

/** @brief Codabar 七元素宽位掩码，与字符顺序一一对应。 */
const std::array<int, 20> CodabarEncodings = {
    0x003, 0x006, 0x009, 0x060, 0x012, 0x042, 0x021, 0x024, 0x030, 0x048,
    0x00c, 0x018, 0x045, 0x051, 0x054, 0x015, 0x01a, 0x029, 0x00b, 0x00e
};

/** @brief 判断文本是否为非空数字串。 @param text 待检查文本。 @return 全为数字时为 true。 */
bool isDigits(const std::string& text)
{
    return !text.empty() && std::all_of(text.begin(), text.end(), [](char character) {
        return character >= '0' && character <= '9';
    });
}

/** @brief 把完整二进制模块串压缩成条空运行段。 @param bits 以 1 表示条、0 表示空的模块串。 @return 严格交替的运行段。 */
std::vector<ModuleRun> bitsToRuns(const std::string& bits)
{
    if (bits.empty()) {
        throw std::invalid_argument("Barcode bit pattern is empty");
    }

    std::vector<ModuleRun> runs; ///< 保存压缩后的连续条空段。
    runs.reserve(bits.size() / 2);
    char current = bits.front(); ///< 当前运行段的二进制类型。
    int width = 0; ///< 当前连续段累计的模块宽度。
    for (char bit : bits) {
        if (bit != '0' && bit != '1') {
            throw std::logic_error("Barcode pattern contains a non-binary module");
        }
        if (bit == current) {
            ++width;
            continue;
        }
        runs.push_back(ModuleRun{current == '1', width});
        current = bit;
        width = 1;
    }
    runs.push_back(ModuleRun{current == '1', width});
    return runs;
}

/** @brief 追加运行段并自动合并相邻同类区域。 @param runs 目标容器。 @param isBar 条空类型。 @param modules 正模块宽度。 */
void appendRun(std::vector<ModuleRun>& runs, bool isBar, int modules)
{
    if (modules <= 0) {
        throw std::logic_error("Encoder attempted to append a non-positive run");
    }
    if (!runs.empty() && runs.back().isBar == isBar) {
        runs.back().modules += modules;
    } else {
        runs.push_back(ModuleRun{isBar, modules});
    }
}

/** @brief 返回单个 EAN 数字的索引。 @param digit ASCII 数字。 @return 0 至 9。 */
int digitIndex(char digit)
{
    if (digit < '0' || digit > '9') {
        throw std::invalid_argument("Retail barcode data must contain digits only");
    }
    return digit - '0';
}

/** @brief 构造 EAN-13 或 UPC-A 共用的 95 模块图案。 @param gtin13 完整十三位数据。 @return 二进制模块串。 */
std::string ean13Bits(const std::string& gtin13)
{
    const std::array<const char*, 10> parity = {
        "LLLLLL", "LLGLGG", "LLGGLG", "LLGGGL", "LGLLGG",
        "LGGLLG", "LGGGLL", "LGLGLG", "LGLGGL", "LGGLGL"
    }; ///< 首位数字对应的左侧奇偶校验表。

    std::string bits = "101"; ///< 从左护条开始累积完整符号。
    const char* leftParity = parity[digitIndex(gtin13.front())]; ///< 左侧六位的模式选择。
    for (int index = 0; index < 6; ++index) {
        const int digit = digitIndex(gtin13[static_cast<std::size_t>(index + 1)]); ///< 当前左侧数字。
        bits += leftParity[index] == 'L' ? EanL[digit] : EanG[digit];
    }
    bits += "01010";
    for (int index = 7; index < 13; ++index) {
        bits += EanR[digitIndex(gtin13[static_cast<std::size_t>(index)])];
    }
    bits += "101";
    return bits;
}

/** @brief 编码 EAN-8。 @param input 七位载荷或八位完整值。 @return 不含图像的符号。 */
BarcodeSymbol encodeEan8(const std::string& input)
{
    const std::string normalized = normalizeGtin(GtinKind::Gtin8, input); ///< 带有效校验位的八位数据。
    std::string bits = "101"; ///< EAN-8 左护条及后续模块。
    for (int index = 0; index < 4; ++index) bits += EanL[digitIndex(normalized[static_cast<std::size_t>(index)])];
    bits += "01010";
    for (int index = 4; index < 8; ++index) bits += EanR[digitIndex(normalized[static_cast<std::size_t>(index)])];
    bits += "101";
    return detail::SymbolBuilder::make(Symbology::Ean8, normalized, normalized, normalized.substr(7), bitsToRuns(bits), {},
                              {{0, 3}, {31, 5}, {64, 3}}, 7, 7);
}

/** @brief 编码 EAN-13。 @param input 十二位载荷或十三位完整值。 @return 规范符号。 */
BarcodeSymbol encodeEan13(const std::string& input)
{
    const std::string normalized = normalizeGtin(GtinKind::Gtin13, input); ///< 完整 GTIN-13。
    return detail::SymbolBuilder::make(Symbology::Ean13, normalized, normalized, normalized.substr(12), bitsToRuns(ean13Bits(normalized)), {},
                              {{0, 3}, {45, 5}, {92, 3}}, 11, 7);
}

/** @brief 编码 UPC-A。 @param input 十一位载荷或十二位完整值。 @return UPC-A 语义符号。 */
BarcodeSymbol encodeUpcA(const std::string& input)
{
    const std::string normalized = normalizeGtin(GtinKind::Gtin12, input); ///< 完整 UPC-A。
    const std::string eanEquivalent = '0' + normalized; ///< UPC-A 与前导零 EAN-13 共用模块图案。
    return detail::SymbolBuilder::make(Symbology::UpcA, normalized, normalized, normalized.substr(11), bitsToRuns(ean13Bits(eanEquivalent)), {},
                              {{0, 3}, {45, 5}, {92, 3}}, 9, 9);
}

/** @brief 将 UPC-E 规范值展开成 UPC-A。 @param upce 八位 NS+主体+校验位。 @return 十二位 UPC-A。 */
std::string expandUpce(const std::string& upce)
{
    const char numberSystem = upce[0]; ///< UPC-E 号码系统，仅支持 0 或 1。
    const std::string body = upce.substr(1, 6); ///< 六位压缩主体。
    const char checkDigit = upce[7]; ///< 保留原校验位供展开后验证。
    std::string manufacturer; ///< 展开后的五位厂商代码。
    std::string product; ///< 展开后的五位商品代码。
    const char compression = body[5]; ///< 决定零填充布局的末位。
    if (compression >= '0' && compression <= '2') {
        manufacturer = body.substr(0, 2) + compression + "00";
        product = "00" + body.substr(2, 3);
    } else if (compression == '3') {
        manufacturer = body.substr(0, 3) + "00";
        product = "000" + body.substr(3, 2);
    } else if (compression == '4') {
        manufacturer = body.substr(0, 4) + '0';
        product = "0000" + body.substr(4, 1);
    } else {
        manufacturer = body.substr(0, 5);
        product = "0000" + body.substr(5, 1);
    }
    return std::string(1, numberSystem) + manufacturer + product + checkDigit;
}

/** @brief 规范化 UPC-E 的 6/7/8 位文本。 @param input 调用方文本。 @return NS+主体+校验位八位值。 */
std::string normalizeUpce(const std::string& input)
{
    if (!isDigits(input) || (input.size() != 6 && input.size() != 7 && input.size() != 8)) {
        throw std::invalid_argument("UPC-E accepts 6, 7 or 8 digits");
    }
    std::string candidate; ///< 暂存带号码系统但可能缺少校验位的值。
    if (input.size() == 6) candidate = '0' + input;
    else candidate = input;
    if (candidate[0] != '0' && candidate[0] != '1') {
        throw std::invalid_argument("UPC-E number system must be 0 or 1");
    }

    if (candidate.size() == 7) {
        const std::string provisional = candidate + '0'; ///< 用占位校验位展开得到 UPC-A 载荷。
        const std::string upcaPayload = expandUpce(provisional).substr(0, 11); ///< 展开的十一位载荷。
        candidate.push_back(calculateGtinCheckDigit(upcaPayload));
    }
    static_cast<void>(normalizeGtin(GtinKind::Gtin12, expandUpce(candidate)));
    return candidate;
}

/** @brief 编码 UPC-E。 @param input 六、七或八位文本。 @return UPC-E 模块符号。 */
BarcodeSymbol encodeUpcE(const std::string& input)
{
    const std::string normalized = normalizeUpce(input); ///< 八位规范 UPC-E。
    const std::array<const char*, 10> parityNs0 = {
        "GGGLLL","GGLGLL","GGLLGL","GGLLLG","GLGGLL","GLLGGL","GLLLGG","GLGLGL","GLGLLG","GLLGLG"
    }; ///< NS0 按校验位选择的六位奇偶模式。
    const std::array<const char*, 10> parityNs1 = {
        "LLLGGG","LLGLGG","LLGGLG","LLGGGL","LGLLGG","LGGLLG","LGGGLL","LGLGLG","LGLGGL","LGGLGL"
    }; ///< NS1 使用的互补奇偶模式。
    const int check = digitIndex(normalized[7]); ///< 校验位决定主体的模式序列。
    const char* parity = normalized[0] == '0' ? parityNs0[check] : parityNs1[check]; ///< 当前号码系统的模式。
    std::string bits = "101"; ///< UPC-E 左护条。
    for (int index = 0; index < 6; ++index) {
        const int digit = digitIndex(normalized[static_cast<std::size_t>(index + 1)]); ///< 当前压缩数字。
        bits += parity[index] == 'L' ? EanL[digit] : EanG[digit];
    }
    bits += "010101";
    return detail::SymbolBuilder::make(Symbology::UpcE, normalized, normalized, normalized.substr(7), bitsToRuns(bits), {},
                              {{0, 3}, {45, 6}}, 9, 7);
}

/** @brief 编码 ITF-14。 @param input 十三位载荷或十四位完整值。 @return 带承载条建议的符号。 */
BarcodeSymbol encodeItf14(const std::string& input)
{
    const std::string normalized = normalizeGtin(GtinKind::Gtin14, input); ///< 完整 GTIN-14。
    const std::array<const char*, 10> patterns = {
        "nnwwn", "wnnnw", "nwnnw", "wwnnn", "nnwnw",
        "wnwnn", "nwwnn", "nnnww", "wnnwn", "nwnwn"
    }; ///< Interleaved 2 of 5 每个数字的五元素模式。
    std::vector<ModuleRun> runs; ///< 从起始结构开始累积条空运行段。
    appendRun(runs, true, 1); appendRun(runs, false, 1); appendRun(runs, true, 1); appendRun(runs, false, 1);
    for (std::size_t index = 0; index < normalized.size(); index += 2) {
        const char* bars = patterns[static_cast<std::size_t>(digitIndex(normalized[index]))]; ///< 数字对第一位控制条宽。
        const char* spaces = patterns[static_cast<std::size_t>(digitIndex(normalized[index + 1]))]; ///< 第二位控制空宽。
        for (int element = 0; element < 5; ++element) {
            appendRun(runs, true, bars[element] == 'w' ? 3 : 1);
            appendRun(runs, false, spaces[element] == 'w' ? 3 : 1);
        }
    }
    appendRun(runs, true, 3); appendRun(runs, false, 1); appendRun(runs, true, 1);
    return detail::SymbolBuilder::make(Symbology::Itf14, normalized, normalized, normalized.substr(13), std::move(runs), {}, {}, 10, 10, true);
}

/** @brief 编码 Code39。 @param input 基础字符文本。 @return 含隐式星号起止符的符号。 */
BarcodeSymbol encodeCode39(const std::string& input)
{
    if (input.empty()) throw std::invalid_argument("Code39 data is empty");
    if (input.size() > MaximumVariableInputBytes) throw std::length_error("Code39 data exceeds 1024 bytes");
    for (char character : input) {
        if (character == '*' || Code39Patterns.count(character) == 0) throw std::invalid_argument("Code39 contains an unsupported character");
    }
    const std::string encoded = '*' + input + '*'; ///< 加入 Code39 起止符的内部文本。
    std::vector<ModuleRun> runs; ///< 按宽窄模式生成的条空序列。
    for (std::size_t characterIndex = 0; characterIndex < encoded.size(); ++characterIndex) {
        const char* pattern = Code39Patterns.at(encoded[characterIndex]); ///< 当前字符的九元素模式。
        for (int element = 0; element < 9; ++element) appendRun(runs, element % 2 == 0, pattern[element] == 'w' ? 3 : 1);
        if (characterIndex + 1 < encoded.size()) appendRun(runs, false, 1);
    }
    return detail::SymbolBuilder::make(Symbology::Code39, encoded, input, {}, std::move(runs), {}, {}, 10, 10);
}

/** @brief 将 ASCII 输入转换为 Code93 Full ASCII 内部序列。 @param input 原始 ASCII 字节。 @return 基础字符及 a/b/c/d 移位符。 */
std::string code93Extended(const std::string& input)
{
    std::string converted; ///< 累积可由 Code93 基础表表示的序列。
    converted.reserve(input.size() * 2);
    for (unsigned char character : input) {
        if (character > 127) throw std::invalid_argument("Code93 Full ASCII accepts bytes 0 through 127");
        if (character == 0) converted += "bU";
        else if (character <= 26) { converted.push_back('a'); converted.push_back(static_cast<char>('A' + character - 1)); }
        else if (character <= 31) { converted.push_back('b'); converted.push_back(static_cast<char>('A' + character - 27)); }
        else if (character == 32 || character == 45 || character == 46 || character == 47 ||
                 (character >= 48 && character <= 57) || (character >= 65 && character <= 90)) converted.push_back(static_cast<char>(character));
        else if (character >= 33 && character <= 44) { converted.push_back('c'); converted.push_back(static_cast<char>('A' + character - 33)); }
        else if (character == 58) converted += "cZ";
        else if (character <= 63) { converted.push_back('b'); converted.push_back(static_cast<char>('F' + character - 59)); }
        else if (character == 64) converted += "bV";
        else if (character <= 95) { converted.push_back('b'); converted.push_back(static_cast<char>('K' + character - 91)); }
        else if (character == 96) converted += "bW";
        else if (character <= 122) { converted.push_back('d'); converted.push_back(static_cast<char>('A' + character - 97)); }
        else { converted.push_back('b'); converted.push_back(static_cast<char>('P' + character - 123)); }
    }
    return converted;
}

/** @brief 计算 Code93 C 或 K 校验符。 @param values 已编码符号值。 @param maximumWeight 最大循环权重。 @return 0 至 46 的校验值。 */
int code93Checksum(const std::vector<int>& values, int maximumWeight)
{
    int sum = 0; ///< 从右侧开始累加权重乘积。
    int weight = 1; ///< 当前符号的循环权重。
    for (auto iterator = values.rbegin(); iterator != values.rend(); ++iterator) {
        sum += *iterator * weight;
        weight = weight == maximumWeight ? 1 : weight + 1;
    }
    return sum % 47;
}

/** @brief 编码 Code93。 @param input 用户文本。 @param options 标准或 Full ASCII 模式。 @return 含 C/K 校验码的符号。 */
BarcodeSymbol encodeCode93(const std::string& input, const Code93Options& options)
{
    if (input.empty()) throw std::invalid_argument("Code93 data is empty");
    if (input.size() > MaximumVariableInputBytes) throw std::length_error("Code93 data exceeds 1024 bytes");
    const std::string encoded = options.mode == Code93Mode::FullAscii ? code93Extended(input) : input; ///< 实际查表序列。
    std::vector<int> values; ///< 保存数据和校验码的数值。
    values.reserve(encoded.size() + 2);
    for (char character : encoded) {
        const std::size_t position = Code93Alphabet.find(character); ///< 字符在基础表中的符号值。
        if (position == std::string::npos || character == '*' ||
            (options.mode == Code93Mode::Standard && character >= 'a' && character <= 'd')) {
            throw std::invalid_argument("Code93 contains an unsupported character");
        }
        values.push_back(static_cast<int>(position));
    }
    const int checksumC = code93Checksum(values, 20); ///< 第一校验字符 C。
    values.push_back(checksumC);
    const int checksumK = code93Checksum(values, 15); ///< 包含 C 后计算的第二校验字符 K。
    values.push_back(checksumK);

    std::string bits = Code93Patterns[47]; ///< 星号起始符。
    for (int value : values) bits += Code93Patterns[static_cast<std::size_t>(value)];
    bits += Code93Patterns[47];
    bits += '1';
    const std::string checksumText{Code93Alphabet[static_cast<std::size_t>(checksumC)], Code93Alphabet[static_cast<std::size_t>(checksumK)]}; ///< C/K 的字符表达。
    return detail::SymbolBuilder::make(Symbology::Code93, encoded + checksumText, input, checksumText, bitsToRuns(bits), values, {}, 10, 10);
}

/** @brief Code128 当前活动字符集。 */
enum class CodeSet { A, B, C };

/** @brief 判断当前位置是否有适合 Set C 的偶数数字段。 @param text 数据。 @param offset 起点。 @return 至少四个且为偶数时为 true。 */
bool canUseCodeSetC(const std::string& text, std::size_t offset)
{
    std::size_t count = 0; ///< 从当前位置连续数字的数量。
    while (offset + count < text.size() && std::isdigit(static_cast<unsigned char>(text[offset + count])) != 0) ++count;
    return count >= 4 && count % 2 == 0;
}

/** @brief 将 ASCII 字节转换为 Code128 A/B 码字。 @param character 输入字节。 @param set 当前字符集。 @return 0 至 95 的数据码字。 */
int code128Value(unsigned char character, CodeSet set)
{
    if (set == CodeSet::A && character <= 95) return character <= 31 ? character + 64 : character - 32;
    if (set == CodeSet::B && character >= 32 && character <= 127) return character - 32;
    throw std::invalid_argument("Character is not representable in the active Code128 set");
}

/** @brief 生成 Code128 码字，包含起始、可选初始 FNC1、校验和与停止符。 @param text ASCII 数据。 @param gs1 是否按 GS1 解释 GS 字节。 @return 完整码字数组。 */
std::vector<int> code128Codewords(const std::string& text, bool gs1)
{
    if (text.empty()) throw std::invalid_argument("Code128 data is empty");
    if (text.size() > MaximumVariableInputBytes) throw std::length_error("Code128 data exceeds 1024 bytes");
    for (unsigned char character : text) {
        if (character > 127) {
            throw std::invalid_argument("Code128 accepts ASCII bytes 0 through 127");
        }
    }

    std::vector<int> words; ///< 按编码顺序保存所有码字。
    words.reserve(text.size() + 16);
    CodeSet set = CodeSet::B; ///< 为保持稳定输出默认从 Set B 开始。
    words.push_back(104);
    if (gs1) words.push_back(102);
    std::size_t offset = 0; ///< 当前尚未编码的输入位置。
    while (offset < text.size()) {
        const unsigned char character = static_cast<unsigned char>(text[offset]); ///< 当前输入字节。
        if (gs1 && character == static_cast<unsigned char>(GroupSeparator)) {
            words.push_back(102);
            ++offset;
            continue;
        }
        if (set != CodeSet::C && canUseCodeSetC(text, offset)) {
            words.push_back(99);
            set = CodeSet::C;
            continue;
        }
        if (set == CodeSet::C) {
            if (offset + 1 < text.size() && std::isdigit(character) != 0 &&
                std::isdigit(static_cast<unsigned char>(text[offset + 1])) != 0) {
                words.push_back((text[offset] - '0') * 10 + text[offset + 1] - '0');
                offset += 2;
                continue;
            }
            if (character <= 31) {
                words.push_back(101);
                set = CodeSet::A;
            } else {
                words.push_back(100);
                set = CodeSet::B;
            }
            continue;
        }
        if (character <= 31 && set != CodeSet::A) {
            words.push_back(101);
            set = CodeSet::A;
            continue;
        }
        if (character > 31 && set == CodeSet::A) {
            words.push_back(100);
            set = CodeSet::B;
            continue;
        }
        words.push_back(code128Value(character, set));
        ++offset;
    }

    int checksum = words.front(); ///< Code128 校验和以起始码字作为基值。
    for (std::size_t index = 1; index < words.size(); ++index) checksum += words[index] * static_cast<int>(index);
    words.push_back(checksum % 103);
    words.push_back(106);
    return words;
}

/** @brief 把 Code128 码字转换为条空运行段。 @param words 完整码字。 @return 拼接后的运行段。 */
std::vector<ModuleRun> code128Runs(const std::vector<int>& words)
{
    std::vector<ModuleRun> runs; ///< 保存每个码字依次展开后的运行段。
    runs.reserve(words.size() * 6 + 1);
    for (int word : words) {
        bool isBar = true; ///< 每个 Code128 模式都从条开始并交替。
        for (char width : std::string(Code128Patterns[static_cast<std::size_t>(word)])) {
            appendRun(runs, isBar, width - '0');
            isBar = !isBar;
        }
    }
    return runs;
}

/** @brief 编码普通或 GS1 Code128。 @param text 规范数据。 @param gs1 是否插入 FNC1。 @param hri 人类可读文本。 @return 统一符号。 */
BarcodeSymbol encodeCode128(const std::string& text, bool gs1, const std::string& hri)
{
    const std::vector<int> words = code128Codewords(text, gs1); ///< 包含校验与停止符的完整码字。
    const std::string checksum = std::to_string(words[words.size() - 2]); ///< 校验码字的十进制表示。
    return detail::SymbolBuilder::make(gs1 ? Symbology::Gs1_128 : Symbology::Code128, text, hri, checksum,
                              code128Runs(words), words, {}, 10, 10);
}

/** @brief 判断字符是否为 Codabar 起止符。 @param character 待检查字符。 @return A 至 D 时为 true。 */
bool isCodabarGuard(char character)
{
    return character >= 'A' && character <= 'D';
}

/** @brief 编码 Codabar。 @param input 不含起止符的数据。 @param options 起止和校验配置。 @return 规范符号。 */
BarcodeSymbol encodeCodabar(const std::string& input, const CodabarOptions& options)
{
    if (input.empty()) throw std::invalid_argument("Codabar data is empty");
    if (input.size() > MaximumVariableInputBytes) throw std::length_error("Codabar data exceeds 1024 bytes");
    if (!isCodabarGuard(options.start) || !isCodabarGuard(options.stop)) throw std::invalid_argument("Codabar guards must be A, B, C or D");
    for (char character : input) {
        const std::size_t value = CodabarAlphabet.find(character); ///< 数据字符的 Codabar 数值。
        if (value == std::string::npos || isCodabarGuard(character)) throw std::invalid_argument("Codabar data contains an unsupported character");
    }

    std::string encoded(1, options.start); ///< 从配置起始符开始建立内部文本。
    encoded += input;
    std::string checksumText; ///< 可选 Mod-16 校验字符。
    if (options.checksum == CodabarChecksum::Modulo16) {
        int sum = static_cast<int>(CodabarAlphabet.find(options.start)) + static_cast<int>(CodabarAlphabet.find(options.stop)); ///< 起止符参与补数计算。
        for (char character : input) sum += static_cast<int>(CodabarAlphabet.find(character));
        const int checksumValue = (16 - sum % 16) % 16; ///< 插入终止符前的补数符号值。
        checksumText.assign(1, CodabarAlphabet[static_cast<std::size_t>(checksumValue)]);
        encoded += checksumText;
    }
    encoded.push_back(options.stop);

    std::vector<ModuleRun> runs; ///< 由七元素宽窄掩码组成的条空序列。
    for (std::size_t characterIndex = 0; characterIndex < encoded.size(); ++characterIndex) {
        const std::size_t value = CodabarAlphabet.find(encoded[characterIndex]); ///< 当前字符的表索引。
        const int mask = CodabarEncodings[value]; ///< 宽元素位掩码。
        for (int element = 0; element < 7; ++element) {
            const bool isWide = (mask & (1 << (6 - element))) != 0; ///< 高位对应最左侧元素。
            appendRun(runs, element % 2 == 0, isWide ? 3 : 1);
        }
        if (characterIndex + 1 < encoded.size()) appendRun(runs, false, 1);
    }
    const std::string hri = options.includeGuardsInHri ? encoded : input + checksumText; ///< 根据显示选项选择 HRI。
    return detail::SymbolBuilder::make(Symbology::Codabar, encoded, hri, checksumText, std::move(runs), {}, {}, 10, 10);
}

/** @brief 从请求中取得普通字符串。 @param request 编码请求。 @return 只读字符串引用。 */
const std::string& ordinaryPayload(const EncodeRequest& request)
{
    const std::string* text = std::get_if<std::string>(&request.payload); ///< 检查联合当前保存的载荷类型。
    if (text == nullptr) throw std::invalid_argument("This symbology requires a string payload");
    return *text;
}

} // namespace

/**
 * @brief 分派无状态码制编码器。
 * @param request 目标码制、载荷和选项。
 * @return 完整不可变符号。
 */
BarcodeSymbol encode(const EncodeRequest& request)
{
    switch (request.symbology) {
    case Symbology::Ean8: return encodeEan8(ordinaryPayload(request));
    case Symbology::Ean13: return encodeEan13(ordinaryPayload(request));
    case Symbology::UpcA: return encodeUpcA(ordinaryPayload(request));
    case Symbology::UpcE: return encodeUpcE(ordinaryPayload(request));
    case Symbology::Itf14: return encodeItf14(ordinaryPayload(request));
    case Symbology::Code39: return encodeCode39(ordinaryPayload(request));
    case Symbology::Code93: {
        const Code93Options* options = std::get_if<Code93Options>(&request.options); ///< 可选 Code93 模式。
        return encodeCode93(ordinaryPayload(request), options == nullptr ? Code93Options{} : *options);
    }
    case Symbology::Code128: return encodeCode128(ordinaryPayload(request), false, ordinaryPayload(request));
    case Symbology::Gs1_128: {
        Gs1Message validated; ///< 统一保存结构化、括号或扫描元素串的校验结果。
        if (const Gs1Message* message = std::get_if<Gs1Message>(&request.payload)) {
            validated = validateGs1(message->elements);
        } else if (const std::string* text = std::get_if<std::string>(&request.payload)) {
            validated = parseGs1(*text);
        } else {
            throw std::invalid_argument("GS1-128 requires text or a Gs1Message payload");
        }
        const std::string elementString = gs1ElementString(validated); ///< 包含必要 GS 的规范载荷。
        return encodeCode128(elementString, true, gs1Hri(validated));
    }
    case Symbology::Codabar: {
        const CodabarOptions* options = std::get_if<CodabarOptions>(&request.options); ///< 可选 Codabar 行为。
        return encodeCodabar(ordinaryPayload(request), options == nullptr ? CodabarOptions{} : *options);
    }
    }
    throw std::invalid_argument("Unsupported barcode symbology");
}

/** @brief 解析稳定码制名。 @param name 配置或 CLI 名称。 @return 码制枚举。 */
Symbology parseSymbology(std::string_view name)
{
    std::string normalized; ///< 移除分隔符并转换小写后的比较键。
    for (unsigned char character : name) {
        if (character != '-' && character != '_' && character != ' ') normalized.push_back(static_cast<char>(std::tolower(character)));
    }
    if (normalized == "ean8") return Symbology::Ean8;
    if (normalized == "ean13") return Symbology::Ean13;
    if (normalized == "upca") return Symbology::UpcA;
    if (normalized == "upce") return Symbology::UpcE;
    if (normalized == "itf14") return Symbology::Itf14;
    if (normalized == "code39") return Symbology::Code39;
    if (normalized == "code93") return Symbology::Code93;
    if (normalized == "code128") return Symbology::Code128;
    if (normalized == "gs1128") return Symbology::Gs1_128;
    if (normalized == "codabar") return Symbology::Codabar;
    throw std::invalid_argument("Unknown barcode symbology: " + std::string(name));
}

/** @brief 格式化稳定码制名。 @param symbology 码制枚举。 @return 英文配置名。 */
std::string toString(Symbology symbology)
{
    switch (symbology) {
    case Symbology::Ean8: return "EAN-8";
    case Symbology::Ean13: return "EAN-13";
    case Symbology::UpcA: return "UPC-A";
    case Symbology::UpcE: return "UPC-E";
    case Symbology::Itf14: return "ITF-14";
    case Symbology::Code39: return "Code39";
    case Symbology::Code93: return "Code93";
    case Symbology::Code128: return "Code128";
    case Symbology::Gs1_128: return "GS1-128";
    case Symbology::Codabar: return "Codabar";
    }
    throw std::invalid_argument("Unsupported barcode symbology");
}

} // namespace barcode
