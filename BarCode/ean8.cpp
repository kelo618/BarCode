#include "ean8.h"

namespace barcode {
	/**
	* @brief 处理输入数据，计算校验位
	* @param code 用户输入的 7 位数字
	* @throw std::invalid_argument 输入长度不为 7 时抛出
	*/
	void EAN8::processData(const std::string& code) {
		if (code.length() != 7) throw std::invalid_argument("EAN-8需要7位输入");
		fullData = code + calculateCheckDigit(code);
	}

	/**
	* @brief 生成条形码的比特模式（'0' 空白, '1' 黑条）
	*
	* 左侧 4 位使用 L 码，右侧 4 位使用 R 码
	* 护条和中间分隔符按照 EAN-8 标准绘制
	*/
	void EAN8::generatePattern() {
		pattern.clear();
		pattern += "101"; // 左护条

		// 左侧4位（L编码）
		for (int i = 0; i < 4; ++i) {
			pattern += L_encode(fullData[i]);
		}

		pattern += "01010"; // 中间护条

		// 右侧4位（R编码）
		for (int i = 4; i < 8; ++i) {
			pattern += R_encode(fullData[i]);
		}

		pattern += "101"; // 右护条
	}

	/**
	* @brief 获取指定数字的模块中心 X 坐标
	* @param index 数字在 fullData 中的索引 (0~7)
	* @return 模块中心像素坐标
	*/
	int EAN8::getModuleCenterForDigit(size_t index) const {
		const int xBase = quietZone * moduleWidth;

		if (index < 4) { // 左侧4位
			int moduleStart = 3 + index * 7;
			return xBase + (moduleStart + 3.5) * moduleWidth;
		}
		else { // 右侧4位
			int moduleStart = 36 + (index - 4) * 7; // 中护栏后
			return xBase + (moduleStart + 3.5) * moduleWidth;
		}
	}

	/**
	* @brief 计算输入 7 位数字的校验位
	* @param code 7 位数字字符串
	* @return 校验位字符 '0'~'9'
	*/
	char EAN8::calculateCheckDigit(const std::string& code) {
		constexpr std::array<int, 7> weights = { 3,1,3,1,3,1,3 };
		int sum = 0;
		for (size_t i = 0; i < 7; ++i) {
			sum += (code[i] - '0') * weights[i];
		}
		return '0' + ((10 - (sum % 10)) % 10);
	}

}
