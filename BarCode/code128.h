#pragma once
#ifndef _CODE128_H
#define _CODE128_H
#include "BarCode.h"
#include <array>

namespace barcode {
	/**
	 * @class Code128
	 * @brief Code 128 条形码生成类（支持 Set A/B/C）
	 *
	 * 特性：
	 *  - 支持 Set A/B/C 自动切换
	 *  - 自动计算校验和
	 *  - 支持可选显示可读标签 (_showLabels)
	 *  - 基于元素数组生成条码，可与 OpenCV 绘图结合
	 */
	class BARCODE_API Code128 final : public CodeBarcode {
		using CodeBarcode::CodeBarcode;

	protected:
		/**
		 * @brief 校验字符是否可用于 Code128（Set B 基线）
		 * @param c 待验证字符
		 * @return true 可用，false 不可用
		 */
		bool isValidChar(char c) const override;

		/**
		 * @brief 准备内部编码数据（Set B baseline）
		 * @param userData 用户输入字符串
		 * @return 内部编码数据（可能与输入相同）
		 */
		std::string prepareEncodedData(
			const std::string& userData) const override;

		/**
		* @brief 计算校验符（返回 symbol value）
		* @param encoded 内部编码数据
		* @return 校验码 symbol value
		*/
		char calculateCheckDigit(const std::string& encoded) override;

		/**
		 * @brief 构建条码元素数组
		 * @param data 内部编码数据
		 */
		void buildElements(const std::string& data) override;

		/**
		* @brief 在条码下方绘制可读字符
		*/
		void addLabels() override;

	private:
		// =========================
		// 内部类型定义
		// =========================
		enum class Action { OUTPUT, SWITCH_A, SWITCH_B, SWITCH_C };
		enum class CodeSet { A, B, C };

		// =========================
		// 常量定义
		// =========================

		static constexpr int START_B = 104;								///< 默认起始符
		static constexpr int STOP = 106;								///< 停止符

		// =========================
		// 内部工具函数
		// =========================

		/**
		* @brief 判断字符是否为控制字符（需用 Set A）
		* @param c 待判断字符
		* @return true 控制字符
		*/
		bool isControlChar(char c) const;

		/**
		* @brief 将 symbol value 转换为条码元素并添加到 elements
		* @param value symbol value（0~106）
		*/
		void appendPattern(int value);

		/**
		 * @brief 将字符转换为 CodeSet 内部 value
		 * @param c 字符
		 * @param set 当前 CodeSet
		 * @return 内部 value
		 */
		int charToValue(char c, CodeSet set) const;

		/**
		 * @brief 判断当前位置是否可切换到 Set C（数字压缩）
		 * @param s 数据字符串
		 * @param pos 当前索引
		 * @return true 可以切换
		 */
		static bool canUseSetC(const std::string& s, size_t pos);

		static const std::array<std::string, 107>& getTable();
	};
}
#endif
