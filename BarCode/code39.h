#pragma once
#ifndef _CODE39_H
#define _CODE39_H
#include "BarCode.h"

namespace barcode {
	/**
	 * @class Code39
	 * @brief Code 39 条形码生成类
	 *
	 * Code 39 是一种广泛应用的字母数字条码，支持数字、字母及部分特殊符号。
	 * 特性：
	 *  - 每字符 9 模块（宽/窄条）
	 *  - 自动添加起始/结束符 '*'
	 *  - 可选择显示或隐藏数字/字符标签 (_showLabels)
	 *  - 不支持校验位
	 */
	class BARCODE_API Code39 : public CodeBarcode {
		using CodeBarcode::CodeBarcode;

	protected:
		/**
		 * @brief 为条码数据添加起始/结束符
		 * @param userData 用户输入数据
		 * @return 添加 '*' 包裹后的完整数据
		 */
		std::string prepareEncodedData(
			const std::string& userData) const override;

		/**
		 * @brief 根据数据生成条码元素（宽窄条数组）
		 * @param data 已处理的完整数据（含起止符）
		 */
		void buildElements(const std::string& data) override;

		/**
		 * @brief 在条码下方显示字符标签
		 */
		void addLabels() override;

		/**
		 * @brief 验证字符是否可用于 Code39
		 * @param c 待验证字符
		 * @return true 可用，false 不可用
		 */
		bool isValidChar(char c) const override;

		/**
		 * @brief 校验输入数据合法性
		 * @param data 用户输入数据
		 * @throw std::invalid_argument 包含 '*' 或非法字符时抛出
		 */
		void validateInput(const std::string& data) override;

	private:
		static const std::unordered_map<char, std::string>& getTable();

		/**
		 * @brief Code39 不使用校验位
		 */
		char calculateCheckDigit(const std::string& code) override;
	};
}
#endif // !_CODE39_H
