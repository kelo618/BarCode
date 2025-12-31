#pragma once
#ifndef _EAN_13_H
#define _EAN_13_H
#include "BarCode.h"

namespace BarCode {
	class EAN13 final : public EanBarcode {
	public:
		using EanBarcode::EanBarcode;

	protected:
		void processData(const std::string& code) override {
			if (code.length() != 12) throw std::invalid_argument("需要12位数字");
			fullData = code + calculateCheckDigit(code);
		}

		void generatePattern() override {
			pattern.clear();
			static const std::unordered_map<char, std::string> parityMap = {
				{'0',"LLLLLL"}, {'1',"LLGLGG"}, {'2',"LLGGLG"},
				{'3',"LLGGGL"}, {'4',"LGLLGG"}, {'5',"LGGLLG"},
				{'6',"LGGGLL"}, {'7',"LGLGLG"}, {'8',"LGLGGL"},
				{'9',"LGGLGL"}
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

		int getModuleCenterForDigit(size_t index) const override {
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
			return xBase; // 防御
		}
	private:
		char calculateCheckDigit(const std::string& code) override {
			int sum = 0;
			for (int i = 0; i < 12; ++i) {
				int digit = code[11 - i] - '0';   // 从右往左
				sum += (i % 2 == 0) ? digit * 3 : digit;
			}
			return '0' + ((10 - (sum % 10)) % 10);
		}

		static std::string G_encode(char c) {
			static const std::unordered_map<char, std::string> G = {
				{'0',"0100111"}, {'1',"0110011"}, {'2',"0011011"},
				{'3',"0100001"}, {'4',"0011101"}, {'5',"0111001"},
				{'6',"0000101"}, {'7',"0010001"}, {'8',"0001001"},
				{'9',"0010111"}
			};
			return G.at(c);
		}
	};
}
#endif // !_EAN_13_H