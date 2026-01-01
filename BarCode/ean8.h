#pragma once
#ifndef _EAN_8_H
#define _EAN_8_H
#include "BarCode.h"

namespace barcode {
	/**
	* @class EAN8
	* @brief EAN-8 条形码具体实现类
	*
	* 特点：
	*  - 长度固定为 7 位输入，自动计算校验位
	*  - 继承 EanBarcode，实现 EAN-8 特有编码逻辑
	*  - 标签文字居中显示（可通过 showLabels 控制）
	*/
	class EAN8 final : public EanBarcode {
	public:
		/**
		 * @brief 使用指定条码尺寸创建 EAN-8 对象
		 * @param size 条码尺寸枚举（MINIMUM/STANDARD/LARGE）
		 */
		using EanBarcode::EanBarcode;

	protected:
		/**
		 * @brief 处理输入数据，计算校验位
		 * @param code 用户输入的 7 位数字
		 * @throw std::invalid_argument 输入长度不为 7 时抛出
		 */
		void processData(const std::string& code) override;

		/**
		 * @brief 生成条形码的比特模式（'0' 空白, '1' 黑条）
		 *
		 * 左侧 4 位使用 L 码，右侧 4 位使用 R 码
		 * 护条和中间分隔符按照 EAN-8 标准绘制
		 */
		void generatePattern() override;

		/**
		* @brief 获取指定数字的模块中心 X 坐标
		* @param index 数字在 fullData 中的索引 (0~7)
		* @return 模块中心像素坐标
		*/
		int getModuleCenterForDigit(size_t index) const override;

		/**
		* @brief 计算输入 7 位数字的校验位
		* @param code 7 位数字字符串
		* @return 校验位字符 '0'~'9'
		*/
		char calculateCheckDigit(const std::string& code) override;
	};
}
#endif // !_EAN_8_H