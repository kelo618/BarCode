#include "code39.h"

namespace barcode
{
	/**
	* @brief 为条码数据添加起始/结束符
	* @param userData 用户输入数据
	* @return 添加 '*' 包裹后的完整数据
	*/
	std::string Code39::prepareEncodedData(const std::string& userData) const
	{
		return "*" + userData + "*";
	}

	/**
	* @brief 根据数据生成条码元素（宽窄条数组）
	* @param data 已处理的完整数据（含起止符）
	*/
	void Code39::buildElements(const std::string& data) {
		for (char c : data) {
			const std::string& pattern = CODE39_TABLE.at(c); // 'w'/'n'
			for (size_t i = 0; i < pattern.size(); ++i) {
				bool isBar = (i % 2 == 0);
				int width = (pattern[i] == 'w') ? wideModule : narrowModule;
				elements.push_back({ isBar, width });
			}
			// 字符间窄空白
			elements.push_back({ false, 1 });
		}
	}

	/**
	* @brief 在条码下方显示字符标签
	*/
	void Code39::addLabels() {
		if (fullData.empty()) return;

		// 文字显示在条码下方
		int labelHeight = 25; // 可根据 fontScale 调整
		int y = barHeight + labelHeight;
		int x = quietZone * moduleWidth;

		for (char c : fullData) {
			if (c == '*') continue;
			cv::putText(
				barcodeImage,
				std::string(1, c),
				cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX,
				fontScale, cv::Scalar(0), fontThickness);
			x += narrowModule * 9 * moduleWidth; // Code39每字符9模块
		}
	}

	/**
	* @brief 验证字符是否可用于 Code39
	* @param c 待验证字符
	* @return true 可用，false 不可用
	*/
	bool Code39::isValidChar(char c) const {
		static const std::string charset =
			"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";
		return charset.find(c) != std::string::npos;
	}

	/**
	* @brief 校验输入数据合法性
	* @param data 用户输入数据
	* @throw std::invalid_argument 包含 '*' 或非法字符时抛出
	*/
	void Code39::validateInput(const std::string& data) {
		CodeBarcode::validateInput(data);

		if (data.find('*') != std::string::npos)
			throw std::invalid_argument(
				"Code39 data must not contain '*'"
			);
	}

	/**
	* @brief Code39 不使用校验位
	*/
	char Code39::calculateCheckDigit(const std::string& code) { throw std::logic_error("This Code barcode does not use a check digit"); }

	/**
	 * @brief Code39 字符对应的编码表
	 * 'n' = 窄条/窄空，'w' = 宽条/宽空
	 */
	const std::unordered_map<char, std::string> Code39::CODE39_TABLE = {
	{'0', "nnnwwnwnn"}, {'1', "wnnwnnnnw"}, {'2', "nnwwnnnnw"}, {'3', "wnwwnnnnn"},
	{'4', "nnnwwnnnw"}, {'5', "wnnwwnnnn"}, {'6', "nnwwwnnnn"}, {'7', "nnnwnnwnw"},
	{'8', "wnnwnnwnn"}, {'9', "nnwwnnwnn"},
	{'A', "wnnnnwnnw"}, {'B', "nnwnnwnnw"}, {'C', "wnwnnwnnn"}, {'D', "nnnnwwnnw"},
	{'E', "wnnnwwnnn"}, {'F', "nnwnwwnnn"}, {'G', "nnnnnwwnw"}, {'H', "wnnnnwwnn"},
	{'I', "nnwnnwwnn"}, {'J', "nnnnwwwnn"},
	{'K', "wnnnnnnww"}, {'L', "nnwnnnnww"}, {'M', "wnwnnnnwn"}, {'N', "nnnnwnnww"},
	{'O', "wnnnwnnwn"}, {'P', "nnwnwnnwn"}, {'Q', "nnnnnnwww"}, {'R', "wnnnnnwwn"},
	{'S', "nnwnnnwwn"}, {'T', "nnnnwnwwn"},
	{'U', "wwnnnnnnw"}, {'V', "nwwnnnnnw"}, {'W', "wwwnnnnnn"}, {'X', "nwnnwnnnw"},
	{'Y', "wwnnwnnnn"}, {'Z', "nwwnwnnnn"},
	{'-', "nwnnnnwnw"}, {'.', "wwnnnnwnn"}, {' ', "nwwnnnwnn"}, {'$', "nwnwnwnnn"},
	{'/', "nwnwnnnwn"}, {'+', "nwnnnwnwn"}, {'%', "nnnwnwnwn"},
	{'*', "nwnnwnwnn"} // 起始/终止符
	};
}

