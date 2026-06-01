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
		elements.reserve(data.size() * 10);
		for (size_t index = 0; index < data.size(); ++index) {
			char c = data[index];
			const std::string& pattern = getTable().at(c); // 'w'/'n'
			for (size_t i = 0; i < pattern.size(); ++i) {
				bool isBar = (i % 2 == 0);
				int width = (pattern[i] == 'w') ? wideModule : narrowModule;
				elements.push_back({ isBar, width });
			}
			// 字符之间只添加一个窄空白，避免末尾多出额外空白模块
			if (index + 1 < data.size()) {
				elements.push_back({ false, 1 });
			}
		}
	}

	/**
	* @brief 在条码下方显示字符标签
	*/
	void Code39::addLabels() {
		if (!_showLabels || fullData.empty()) return;

		int textY = barHeight + static_cast<int>(fontScale * 32) + 10;
		int x = quietZone * moduleWidth;

		for (char c : fullData) {
			const std::string& pattern = getTable().at(c);
			int symbolModules = 0;
			for (char unit : pattern) {
				symbolModules += (unit == 'w') ? wideModule : narrowModule;
			}

			int symbolWidth = symbolModules * moduleWidth;
			if (c != '*') {
				std::string text(1, c);
				int baseline = 0;
				cv::Size textSize = cv::getTextSize(
					text,
					cv::FONT_HERSHEY_SIMPLEX,
					fontScale,
					fontThickness,
					&baseline
				);
				int textX = x + (symbolWidth - textSize.width) / 2;
				cv::putText(
					barcodeImage,
					text,
					cv::Point(textX, textY),
					cv::FONT_HERSHEY_SIMPLEX,
					fontScale,
					cv::Scalar(0),
					fontThickness
				);
			}

			x += (symbolModules + 1) * moduleWidth; // +1 为字符间窄空白
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

	const std::unordered_map<char, std::string>& Code39::getTable()
	{
		/**
		 * @brief Code39 字符对应的编码表
		 * 'n' = 窄条/窄空，'w' = 宽条/宽空
		 */
		static const std::unordered_map<char, std::string> CODE39_TABLE = {
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
		return CODE39_TABLE;
	}

	/**
	* @brief Code39 不使用校验位
	*/
	char Code39::calculateCheckDigit(const std::string& code) { throw std::logic_error("This Code barcode does not use a check digit"); }
}
