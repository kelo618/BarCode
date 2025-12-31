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

		void addLabels() override {
			const int textY =
				barHeight + guardExtension + 15;   // 稍微靠近条码

			const int xBase = quietZone * moduleWidth;

			auto drawCenteredText = [&](const std::string& text, int centerX) {
				cv::Size size = cv::getTextSize(
					text,
					cv::FONT_HERSHEY_SIMPLEX,
					fontScale,
					fontThickness,
					nullptr
				);
				cv::putText(
					barcodeImage,
					text,
					cv::Point(centerX - size.width / 2, textY),
					cv::FONT_HERSHEY_SIMPLEX,
					fontScale,
					0,
					fontThickness
				);
				};

			// ---------- 首位数字（居中在起始护栏左侧） ----------
			{
				int startGuardWidth = 3 * moduleWidth;
				int centerX = xBase - startGuardWidth / 2;
				drawCenteredText(fullData.substr(0, 1), centerX);
			}

			// ---------- 左侧 6 位 ----------
			for (int i = 0; i < 6; ++i) {
				int moduleStart = 3 + i * 7;  // 左护栏后
				int centerX = xBase + (moduleStart + 3.5) * moduleWidth;
				drawCenteredText(fullData.substr(i + 1, 1), centerX);
			}

			// ---------- 右侧 6 位 ----------
			for (int i = 0; i < 6; ++i) {
				int moduleStart = 50 + i * 7; // 中护栏后
				int centerX = xBase + (moduleStart + 3.5) * moduleWidth;
				drawCenteredText(fullData.substr(i + 7, 1), centerX);
			}
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

		void renderImage() override {
			const int totalWidth = (2 * quietZone + static_cast<int>(pattern.size())) * moduleWidth;
			const int topMargin = 10;
			const int bottomMargin = 10;
			const int totalHeight =
				barHeight + guardExtension + topMargin + bottomMargin;
			initializeImage(totalWidth, totalHeight);

			int baseX = quietZone * moduleWidth;

			for (size_t i = 0; i < pattern.size(); ++i) {
				if (pattern[i] == '1') {
					bool isGuard =
						isLeftGuard(i) ||
						isCenterGuard(i) ||
						isRightGuard(i, pattern.size());

					int height = isGuard
						? barHeight + guardExtension
						: barHeight;

					drawBar(
						baseX + i * moduleWidth,
						moduleWidth,
						height
					);
				}
			}
		}
	};
}
#endif // !_EAN_13_H