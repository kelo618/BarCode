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

		int getModuleCenterForDigit(size_t index) const override {
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

	private:
		char calculateCheckDigit(const std::string& code) override {
			constexpr std::array<int, 7> weights = { 3,1,3,1,3,1,3 };
			int sum = 0;
			for (size_t i = 0; i < 7; ++i) {
				sum += (code[i] - '0') * weights[i];
			}
			return '0' + ((10 - (sum % 10)) % 10);
		}

	};
}
#endif // !_EAN_8_H