#pragma once
#ifndef _EAN_13_H
#define _EAN_13_H
#include "BarCode.h"

namespace barcode {
	/**
	* @class EAN13
	* @brief EAN-13 条形码生成类
	*
	* EAN-13 是国际上广泛使用的商品条形码，编码12位数字数据 + 1位校验位。
	* 该类继承自 EanBarcode，提供完整的条码生成和可选标签显示功能。
	*
	* 功能特性：
	*  - 支持自动计算校验位
	*  - 支持 L/G/R 编码规则
	*  - 可选择显示或隐藏数字标签 (_showLabels)
	*/
	class EAN13 final : public EanBarcode {
	public:
		using EanBarcode::EanBarcode;

	protected:
		/**
		 * @brief 处理输入数据，生成完整条码数据（包含校验位）
		 * @param code 12位数字字符串
		 * @throw std::invalid_argument 输入长度不是12时抛出异常
		 */
		void processData(const std::string& code) override;

		/**
		* @brief 生成条形码编码模式（pattern），包含护栏、中间条和左右编码
		*/
		void generatePattern() override;

		/**
		* @brief 获取数字在条形码上的中心位置（x坐标）
		* @param index 数字索引，0~12（0为首位，不显示在条码内）
		* @return 对应模块中心 x 坐标
		*/
		int getModuleCenterForDigit(size_t index) const override;

	private:
		/**
		* @brief 计算 EAN-13 校验位
		* @param code 12 位数字字符串
		* @return 校验位字符
		*/
		char calculateCheckDigit(const std::string& code) override;

		/**
		 * @brief G 码表编码
		 * @param c 数字字符 '0'~'9'
		 * @return 7位编码字符串
		 */
		static std::string G_encode(char c);
	};
}
#endif // !_EAN_13_H