/************************************************************
 * File: Gtin.cpp
 * Author: kelo
 * Created: 2026-08-29
 * Description:
 *   实现 GTIN Mod-10 校验位计算、输入长度判定和已有校验位验证。
 *   算法从载荷右侧开始交替使用 3 与 1 权重，供所有零售和物流码制复用。
 ************************************************************/

#include "Core.h"

#include <algorithm>
#include <stdexcept>

namespace barcode {
namespace {

/** @brief 判断文本是否完全由 ASCII 十进制数字组成。 @param text 待检查文本。 @return 非空且全为数字时为 true。 */
bool isDigits(std::string_view text)
{
    // GTIN 不接受区域数字或符号，逐字节比较可保持跨平台一致性。
    return !text.empty() && std::all_of(text.begin(), text.end(), [](char character) {
        return character >= '0' && character <= '9';
    });
}

} // namespace

/**
 * @brief 按 GS1 Mod-10 权重计算载荷校验位。
 * @param payload 不含校验位的数字载荷。
 * @return 单个 ASCII 数字校验位。
 */
char calculateGtinCheckDigit(std::string_view payload)
{
    // 空值和非数字内容不能形成 GTIN，统一在计算前拒绝。
    if (!isDigits(payload)) {
        throw std::invalid_argument("GTIN payload must contain digits only");
    }

    int weightedSum = 0; ///< 保存从右向左加权后的十进制总和。
    bool useTripleWeight = true; ///< 最右侧载荷位固定使用权重 3。
    for (auto iterator = payload.rbegin(); iterator != payload.rend(); ++iterator) {
        const int digit = *iterator - '0'; ///< 当前 ASCII 数字的整数值。
        weightedSum += digit * (useTripleWeight ? 3 : 1);
        useTripleWeight = !useTripleWeight;
    }

    // 补到下一个十的倍数得到标准校验位。
    return static_cast<char>('0' + ((10 - weightedSum % 10) % 10));
}

/**
 * @brief 补齐或验证指定长度的 GTIN。
 * @param kind 完整 GTIN 类型。
 * @param digits 载荷或完整值。
 * @return 可直接编码的完整 GTIN。
 */
std::string normalizeGtin(GtinKind kind, std::string_view digits)
{
    // 枚举底层值就是完整号码长度，避免复制不同码制的长度表。
    const std::size_t completeLength = static_cast<std::size_t>(kind); ///< 目标完整长度。
    if (!isDigits(digits)) {
        throw std::invalid_argument("GTIN must contain digits only");
    }

    // 少一位表示调用方要求自动计算，其余非完整长度均属于输入错误。
    if (digits.size() == completeLength - 1) {
        return std::string(digits) + calculateGtinCheckDigit(digits);
    }
    if (digits.size() != completeLength) {
        throw std::invalid_argument("GTIN length does not match the requested kind");
    }

    // 完整值必须与重新计算结果一致，不能静默改写调用方提供的校验位。
    const std::string payload(digits.substr(0, completeLength - 1)); ///< 去掉已有校验位的载荷。
    const char expectedCheckDigit = calculateGtinCheckDigit(payload); ///< 由载荷得到的标准校验位。
    if (digits.back() != expectedCheckDigit) {
        throw std::invalid_argument("GTIN check digit is invalid");
    }

    return std::string(digits);
}

} // namespace barcode
