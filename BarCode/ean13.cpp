#include "ean13.h"
#include <array>

namespace barcode {
	/**
	* @brief 处理输入数据，生成完整条码数据（包含校验位）
	* @param code 12位数字字符串
	* @throw std::invalid_argument 输入长度不是12时抛出异常
	*/
	void EAN13::processData(const std::string& code) {
		if (code.length() != 12) throw std::invalid_argument("需要12位数字");
		fullData = code + calculateCheckDigit(code);
	}

	/**
	* @brief 生成条形码编码模式（pattern），包含护栏、中间条和左右编码
	*/
	void EAN13::generatePattern() {
		static constexpr std::array<const char*, 10> PARITY = {
			"LLLLLL", "LLGLGG", "LLGGLG", "LLGGGL", "LGLLGG",
			"LGGLLG", "LGGGLL", "LGLGLG", "LGLGGL", "LGGLGL"
		};

		pattern.clear();
		pattern.reserve(95);
		pattern = "101"; // 左护条

		const int first = fullData[0] - '0';
		if (first < 0 || first > 9) {
			throw std::invalid_argument("Invalid EAN-13 lead digit");
		}
		const char* parity = PARITY[first];

		// 左侧数据
		for (int i = 0; i < 6; ++i) {
			pattern += (parity[i] == 'L') ? L_encode(fullData[i + 1]) : G_encode(fullData[i + 1]);
		}

		pattern += "01010"; // 中间护条

		// 右侧数据
		for (int i = 7; i < 13; ++i) {
			pattern += R_encode(fullData[i]);
		}

		pattern += "101"; // 右护条
	}

	/**
	* @brief 获取数字在条形码上的中心位置（x坐标）
	* @param index 数字索引，0~12（0为首位，不显示在条码内）
	* @return 对应模块中心 x 坐标
	*/
	int EAN13::getModuleCenterForDigit(size_t index) const {
		const int xBase = quietZone * moduleWidth;
		const int idx = static_cast<int>(index);

		if (idx == 0) { // 首位数字（在左护栏外）
			int startGuardWidth = 3 * moduleWidth;
			return xBase - startGuardWidth / 2;
		}
		else if (idx >= 1 && idx <= 6) { // 左侧6位
			int moduleStart = 3 + (idx - 1) * 7;
			return xBase + ((moduleStart * 2 + 7) * moduleWidth) / 2;
		}
		else if (idx >= 7 && idx <= 12) { // 右侧6位
			int moduleStart = 50 + (idx - 7) * 7;
			return xBase + ((moduleStart * 2 + 7) * moduleWidth) / 2;
		}
		return xBase;
	}

	/**
	* @brief 计算 EAN-13 校验位
	* @param code 12 位数字字符串
	* @return 校验位字符
	*/
	char EAN13::calculateCheckDigit(const std::string& code) {
		int sum = 0;
		for (int i = 0; i < 12; ++i) {
			int digit = code[11 - i] - '0';   // 从右往左
			sum += (i % 2 == 0) ? digit * 3 : digit;
		}
		return '0' + ((10 - (sum % 10)) % 10);
	}

	/**
	* @brief G 码表编码
	* @param c 数字字符 '0'~'9'
	* @return 7位编码字符串
	*/
	std::string EAN13::G_encode(char c) {
		static constexpr std::array<const char*, 10> G = {
			"0100111", "0110011", "0011011", "0100001", "0011101",
			"0111001", "0000101", "0010001", "0001001", "0010111"
		};
		int idx = c - '0';
		if (idx < 0 || idx > 9) {
			throw std::invalid_argument("Invalid EAN digit");
		}
		return G[idx];
	}
}
