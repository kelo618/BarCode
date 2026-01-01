#pragma once
#ifndef _CODE39_H
#define _CODE39_H
#include "BarCode.h"

namespace BarCode {
	class Code39 : public CodeBarcode {
		using CodeBarcode::CodeBarcode;

	protected:
		std::string prepareEncodedData(
			const std::string& userData) const override
		{
			return "*" + userData + "*";
		}
		void buildElements(const std::string& data) override {
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

		void addLabels() override {
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

		bool isValidChar(char c) const override {
			static const std::string charset =
				"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";
			return charset.find(c) != std::string::npos;
		}

		void validateInput(const std::string& data) override {
			CodeBarcode::validateInput(data);

			if (data.find('*') != std::string::npos)
				throw std::invalid_argument(
					"Code39 data must not contain '*'"
				);
		}
	private:
		static inline const std::unordered_map<char, std::string> CODE39_TABLE = {
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

		char calculateCheckDigit(const std::string& code) { throw std::logic_error("This Code barcode does not use a check digit"); };
	};
}
#endif // !_CODE39_H