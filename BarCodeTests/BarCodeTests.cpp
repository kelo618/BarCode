#include "BarcodeFactory.h"

#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
	int failures = 0;

	void expect(bool condition, const std::string& message) {
		if (!condition) {
			++failures;
			std::cerr << "[FAIL] " << message << std::endl;
		}
	}

	void expectEqual(const std::string& actual, const std::string& expected, const std::string& message) {
		if (actual != expected) {
			++failures;
			std::cerr << "[FAIL] " << message << " expected=" << expected << " actual=" << actual << std::endl;
		}
	}

	char computeEan8CheckDigit(const std::string& code7) {
		static constexpr std::array<int, 7> weights = { 3, 1, 3, 1, 3, 1, 3 };
		int sum = 0;
		for (size_t i = 0; i < code7.size(); ++i) {
			sum += (code7[i] - '0') * weights[i];
		}
		return static_cast<char>('0' + ((10 - (sum % 10)) % 10));
	}

	char computeEan13CheckDigit(const std::string& code12) {
		int sum = 0;
		for (int i = 0; i < 12; ++i) {
			int digit = code12[11 - i] - '0';
			sum += (i % 2 == 0) ? digit * 3 : digit;
		}
		return static_cast<char>('0' + ((10 - (sum % 10)) % 10));
	}

	std::string lEncode(char c) {
		static constexpr std::array<const char*, 10> L = {
			"0001101", "0011001", "0010011", "0111101", "0100011",
			"0110001", "0101111", "0111011", "0110111", "0001011"
		};
		return L[c - '0'];
	}

	std::string gEncode(char c) {
		static constexpr std::array<const char*, 10> G = {
			"0100111", "0110011", "0011011", "0100001", "0011101",
			"0111001", "0000101", "0010001", "0001001", "0010111"
		};
		return G[c - '0'];
	}

	std::string rEncode(char c) {
		static constexpr std::array<const char*, 10> R = {
			"1110010", "1100110", "1101100", "1000010", "1011100",
			"1001110", "1010000", "1000100", "1001000", "1110100"
		};
		return R[c - '0'];
	}

	std::string buildEan8Pattern(const std::string& full8) {
		std::string pattern;
		pattern.reserve(67);
		pattern += "101";
		for (int i = 0; i < 4; ++i) pattern += lEncode(full8[i]);
		pattern += "01010";
		for (int i = 4; i < 8; ++i) pattern += rEncode(full8[i]);
		pattern += "101";
		return pattern;
	}

	std::string buildEan13Pattern(const std::string& full13) {
		static constexpr std::array<const char*, 10> PARITY = {
			"LLLLLL", "LLGLGG", "LLGGLG", "LLGGGL", "LGLLGG",
			"LGGLLG", "LGGGLL", "LGLGLG", "LGLGGL", "LGGLGL"
		};

		std::string pattern;
		pattern.reserve(95);
		pattern += "101";

		const char* parity = PARITY[full13[0] - '0'];
		for (int i = 0; i < 6; ++i) {
			pattern += (parity[i] == 'L') ? lEncode(full13[i + 1]) : gEncode(full13[i + 1]);
		}
		pattern += "01010";
		for (int i = 7; i < 13; ++i) {
			pattern += rEncode(full13[i]);
		}
		pattern += "101";
		return pattern;
	}

	int findFirstBlackColumn(const cv::Mat& image) {
		for (int x = 0; x < image.cols; ++x) {
			for (int y = 0; y < image.rows; ++y) {
				if (image.at<uchar>(y, x) < 128) return x;
			}
		}
		return -1;
	}

	int findLastBlackColumn(const cv::Mat& image) {
		for (int x = image.cols - 1; x >= 0; --x) {
			for (int y = 0; y < image.rows; ++y) {
				if (image.at<uchar>(y, x) < 128) return x;
			}
		}
		return -1;
	}

	void assertQuietZone(const barcode::Barcode& code, const std::string& label) {
		const cv::Mat image = code.getImage();
		const int firstBlack = findFirstBlackColumn(image);
		const int lastBlack = findLastBlackColumn(image);
		expect(firstBlack >= 0 && lastBlack >= 0, label + " should contain black bars");
		if (firstBlack < 0 || lastBlack < 0) return;

		const int required = code.getQuietZoneModules() * code.getModuleWidth();
		const int leftQuiet = firstBlack;
		const int rightQuiet = (image.cols - 1) - lastBlack;

		expect(leftQuiet >= required, label + " left quiet zone too small");
		expect(rightQuiet >= required, label + " right quiet zone too small");
	}

	void testEan8CheckAndPattern() {
		barcode::EAN8 code(barcode::BarcodeSize::STANDARD);
		code.showLabels(false);

		const std::string input = "5512345";
		code.encode(input);

		const std::string expectedFull = input + computeEan8CheckDigit(input);
		expectEqual(code.getEncodedData(), expectedFull, "EAN8 check digit regression");
		expectEqual(code.getPattern(), buildEan8Pattern(expectedFull), "EAN8 pattern regression");
		assertQuietZone(code, "EAN8");
	}

	void testEan13CheckAndPattern() {
		barcode::EAN13 code(barcode::BarcodeSize::STANDARD);
		code.showLabels(false);

		const std::string input = "400638133393";
		code.encode(input);

		const std::string expectedFull = input + computeEan13CheckDigit(input);
		expectEqual(code.getEncodedData(), expectedFull, "EAN13 check digit regression");
		expectEqual(code.getPattern(), buildEan13Pattern(expectedFull), "EAN13 pattern regression");
		assertQuietZone(code, "EAN13");
	}

	void testCode128QuietZone() {
		barcode::Code128 code(barcode::BarcodeSize::STANDARD);
		code.showLabels(false);
		code.encode("ABC1234567890XYZ");
		assertQuietZone(code, "Code128");
	}

	void testSizeProfiles() {
		struct SizeExpectation {
			barcode::BarcodeSize size;
			const char* name;
			int minModule;
			int maxModule;
			int minHeight;
			int maxHeight;
			int minWidth;
			int maxWidth;
		};

		static const std::array<SizeExpectation, 3> EXPECTATIONS = {
			SizeExpectation{ barcode::BarcodeSize::MINIMUM, "MINIMUM", 2, 3, 52, 88, 180, 680 },
			SizeExpectation{ barcode::BarcodeSize::STANDARD, "STANDARD", 2, 4, 78, 140, 280, 920 },
			SizeExpectation{ barcode::BarcodeSize::LARGE, "LARGE", 3, 6, 112, 220, 420, 1400 }
		};

		std::vector<int> widths;
		for (const auto& expected : EXPECTATIONS) {
			barcode::EAN13 code(expected.size);
			code.showLabels(false);
			code.encode("400638133393");

			const int module = code.getModuleWidth();
			const int height = code.getBarHeight();
			const int width = code.getImage().cols;
			widths.push_back(width);

			expect(module >= expected.minModule && module <= expected.maxModule,
				std::string(expected.name) + " module width out of range");
			expect(height >= expected.minHeight && height <= expected.maxHeight,
				std::string(expected.name) + " bar height out of range");
			expect(width >= expected.minWidth && width <= expected.maxWidth,
				std::string(expected.name) + " image width out of range");
		}

		expect(widths[0] < widths[1] && widths[1] < widths[2], "size profiles should increase output width");
	}
}

int main() {
	try {
		testEan8CheckAndPattern();
		testEan13CheckAndPattern();
		testCode128QuietZone();
		testSizeProfiles();
	}
	catch (const std::exception& e) {
		std::cerr << "[FATAL] unexpected exception: " << e.what() << std::endl;
		return 2;
	}

	if (failures == 0) {
		std::cout << "All regression tests passed." << std::endl;
		return 0;
	}

	std::cerr << failures << " regression test(s) failed." << std::endl;
	return 1;
}
