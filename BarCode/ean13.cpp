#include "ean13.h"

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
		pattern.clear();
		static const std::unordered_map<char, std::string> parityMap = {
			{ '0',"LLLLLL" },{ '1',"LLGLGG" },{ '2',"LLGGLG" },
			{ '3',"LLGGGL" },{ '4',"LGLLGG" },{ '5',"LGGLLG" },
			{ '6',"LGGGLL" },{ '7',"LGLGLG" },{ '8',"LGLGGL" },
			{ '9',"LGGLGL" }
		};

		pattern = "101"; // 左护条
		const auto& parity = parityMap.at(fullData[0]);

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

		if (index == 0) { // 首位数字（在左护栏外）
			int startGuardWidth = 3 * moduleWidth;
			return xBase - startGuardWidth / 2;
		}
		else if (index >= 1 && index <= 6) { // 左侧6位
			int moduleStart = 3 + (index - 1) * 7;
			return xBase + (moduleStart + 3.5) * moduleWidth;
		}
		else if (index >= 7 && index <= 12) { // 右侧6位
			int moduleStart = 50 + (index - 7) * 7;
			return xBase + (moduleStart + 3.5) * moduleWidth;
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
		static const std::unordered_map<char, std::string> G = {
			{ '0',"0100111" },{ '1',"0110011" },{ '2',"0011011" },
			{ '3',"0100001" },{ '4',"0011101" },{ '5',"0111001" },
			{ '6',"0000101" },{ '7',"0010001" },{ '8',"0001001" },
			{ '9',"0010111" }
		};
		return G.at(c);
	}
}

