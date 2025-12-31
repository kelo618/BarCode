#pragma once
#ifndef _EAN_8_H
#define _EAN_8_H
#include "BarCode.h"

namespace BarCode {
	class EAN8 final : public EanBarcode {
	public:
		using EanBarcode::EanBarcode;

	protected:
		void processData(const std::string& code) override {
			if (code.length() != 7) throw std::invalid_argument("EAN-8需要7位输入");
			fullData = code + calculateCheckDigit(code);
		}

		void generatePattern() override {
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

		void addLabels() override {
			const int textY = barHeight + guardExtension + 20;
			const int xBase = quietZone * moduleWidth;

			// 左侧4位（起始位置：左护条3模块）
			for (int i = 0; i < 4; ++i) {
				cv::putText(barcodeImage, fullData.substr(i, 1),
					cv::Point(xBase + (3 + i * 7) * moduleWidth, textY),
					cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(0), fontThickness);
			}

			// 右侧4位（起始位置：左护条3 + 左侧4位*7 + 中间护条5）
			for (int i = 4; i < 8; ++i) {
				cv::putText(barcodeImage, fullData.substr(i, 1),
					cv::Point(xBase + (3 + 4 * 7 + 5 + (i - 4) * 7) * moduleWidth, textY),
					cv::FONT_HERSHEY_SIMPLEX, fontScale, cv::Scalar(0), fontThickness);
			}
		}

		virtual char calculateCheckDigit(const std::string& code) override {
			constexpr std::array<int, 7> weights = { 3,1,3,1,3,1,3 };
			int sum = 0;
			for (size_t i = 0; i < 7; ++i) {
				sum += (code[i] - '0') * weights[i];
			}
			return '0' + ((10 - (sum % 10)) % 10);
		}

	private:
		void renderImage() {
			const int totalWidth = (2 * quietZone + pattern.size()) * moduleWidth;
			const int totalHeight = barHeight + guardExtension + 40;
			initializeImage(totalWidth, totalHeight);

			int x = quietZone * moduleWidth;
			for (size_t i = 0; i < pattern.size(); ++i) {
				if (pattern[i] == '1') {
					const int height = isGuardPosition(i) ?
						barHeight + guardExtension : barHeight;
					drawBar(x + i * moduleWidth, moduleWidth, height);
				}
			}
		}

		bool isGuardPosition(size_t index) const {
			return (index < 3) ||                   // 左护条
				(index >= 31 && index < 36) ||    // 中间护条（4字符×7模块 + 3）
				(index >= pattern.size() - 3);    // 右护条
		}
	};
}
#endif // !_EAN_8_H