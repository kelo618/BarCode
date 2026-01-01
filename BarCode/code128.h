#pragma once
#ifndef _CODE128_H
#define _CODE128_H
#include "BarCode.h"

//namespace BarCode {
//	class Code128 final : public Barcode
//	{
//	public:
//		//	explicit Code128(BarcodeSize size)
//		//	{
//		//		setSizeParameters(size);
//		//		loadEncodingTable();
//		//	}
//
//		//void encode(const std::string& data) override {
//		//	if (data.empty()) throw std::invalid_argument("空数据");
//
//		//	currentSubset = SelectBestSubset(data);
//		//	validateInput(data);
//		//	processData(data);
//		//	calculateCheckSum();
//		//	generatePattern();
//		//	renderBarcode();
//		//}
//
//		//private:
//		//	// 尺寸参数设置
//		//	void setSizeParameters(BarcodeSize size) {
//		//		switch (size) {
//		//		case BarcodeSize::SMALL:
//		//			moduleWidth = 1; barHeight = 30; quietZone = 10; fontScale = 0.3; break;
//		//		case BarcodeSize::LARGE:
//		//			moduleWidth = 3; barHeight = 70; quietZone = 15; fontScale = 0.9; break;
//		//		default:  // MEDIUM
//		//			moduleWidth = 2; barHeight = 50; quietZone = 10; fontScale = 0.6; break;
//		//		}
//		//	}
//
//		//	void loadEncodingTable() {
//		//		// 使用统一键类型：字符串（用于特殊字符）和字符（普通ASCII）
//		//		encodings.emplace("START_A", Encoding(103, "11010000100", { true,  false, false }));
//		//		encodings.emplace("START_B", Encoding(104, "11010010000", { false, true,  false }));
//		//		encodings.emplace("START_C", Encoding(105, "11010011100", { false, false, true }));
//		//		encodings.emplace("STOP", Encoding(106, "11000111010", { true,  true,  true }));
//		//		encodings.emplace("FNC1", Encoding(102, "11110101110", { true,  true,  true }));
//		//		encodings.emplace("FNC2", Encoding(97, "11110101000", { true,  true,  true }));
//		//		encodings.emplace("FNC3", Encoding(96, "11101011100", { true,  true,  true }));
//		//		encodings.emplace("FNC4", Encoding(100, "11110110110", { true,  true,  true }));
//		//		encodings.emplace("ShiftA", Encoding(98, "11110111000", { false, true,  false }));
//		//		encodings.emplace("ShiftB", Encoding(99, "11110110010", { true,  false, false }));
//		//		encodings.emplace(1, Encoding(1, "11001101100", { true,  false, false }));  // SOH
//		//		encodings.emplace(2, Encoding(2, "11001100110", { true,  false, false }));  // STX
//		//		encodings.emplace(0, Encoding(0, "11011001100", { true,  false, false }));  // NUL
//		//		encodings.emplace(3, Encoding(3, "10010011000", { true,  false, false }));  // ETX
//		//		encodings.emplace(4, Encoding(4, "10010001100", { true,  false, false }));  // EOT
//		//		encodings.emplace(5, Encoding(5, "10001001100", { true,  false, false }));  // ENQ
//		//		encodings.emplace(6, Encoding(6, "10011001000", { true,  false, false }));  // ACK
//		//		encodings.emplace(7, Encoding(7, "10011000100", { true,  false, false }));  // BEL
//		//		encodings.emplace(8, Encoding(8, "10001100100", { true,  false, false }));  // BS
//		//		encodings.emplace(9, Encoding(9, "11001001000", { true,  false, false }));  // HT
//		//		encodings.emplace(10, Encoding(10, "11001000100", { true,  false, false }));  // LF
//		//		encodings.emplace(11, Encoding(11, "11000100100", { true,  false, false }));  // VT
//		//		encodings.emplace(12, Encoding(12, "10110011100", { true,  false, false }));  // FF
//		//		encodings.emplace(13, Encoding(13, "10011011100", { true,  false, false }));  // CR
//		//		encodings.emplace(14, Encoding(14, "10011001110", { true,  false, false }));  // SO
//		//		encodings.emplace(15, Encoding(15, "10111001100", { true,  false, false }));  // SI
//		//		encodings.emplace(16, Encoding(16, "10011101100", { true,  false, true }));   // DLE (子集C有效)
//		//		encodings.emplace(17, Encoding(17, "10011100110", { true,  false, true }));   // DC1
//		//		encodings.emplace(18, Encoding(18, "11001110010", { true,  false, true }));   // DC2
//		//		encodings.emplace(19, Encoding(19, "11001011100", { true,  false, true }));   // DC3
//		//		encodings.emplace(20, Encoding(20, "11001001110", { true,  false, true }));   // DC4
//		//		encodings.emplace(21, Encoding(21, "11011100100", { true,  false, true }));   // NAK
//		//		encodings.emplace(22, Encoding(22, "11001110100", { true,  false, true }));   // SYN
//		//		encodings.emplace(23, Encoding(23, "11101101110", { true,  false, true }));   // ETB
//		//		encodings.emplace(24, Encoding(24, "11101001100", { true,  false, true }));   // CAN
//		//		encodings.emplace(25, Encoding(25, "11100101100", { true,  false, true }));   // EM
//		//		encodings.emplace(26, Encoding(26, "11100100110", { true,  false, false }));  // SUB
//		//		encodings.emplace(27, Encoding(27, "11101100100", { true,  false, false }));  // ESC
//		//		encodings.emplace(28, Encoding(28, "11100110100", { true,  false, false }));  // FS
//		//		encodings.emplace(29, Encoding(29, "11100110010", { true,  false, false }));  // GS
//		//		encodings.emplace(30, Encoding(30, "11011011000", { true,  false, false }));  // RS
//		//		encodings.emplace(31, Encoding(31, "11011000110", { true,  false, false }));  // US
//		//		encodings.emplace(' ', Encoding(32, "11000110110", { true,  true,  false }));
//		//		encodings.emplace('!', Encoding(33, "10100011000", { true,  true,  false }));
//		//		encodings.emplace('\"', Encoding(34, "10001011000", { true,  true,  false }));
//		//		encodings.emplace('#', Encoding(35, "10001000110", { true,  true,  false }));
//		//		encodings.emplace('$', Encoding(36, "10110001000", { true,  true,  false }));
//		//		encodings.emplace('%', Encoding(37, "10001101000", { true,  true,  false }));
//		//		encodings.emplace('&', Encoding(38, "10001100010", { true,  true,  false }));
//		//		encodings.emplace('\'', Encoding(39, "11010001000", { true,  true,  false }));
//		//		encodings.emplace('(', Encoding(40, "11000101000", { true,  true,  false }));
//		//		encodings.emplace(')', Encoding(41, "11000100010", { true,  true,  false }));
//		//		encodings.emplace('*', Encoding(42, "10110111000", { true,  true,  false }));
//		//		encodings.emplace('+', Encoding(43, "10110001110", { true,  true,  false }));
//		//		encodings.emplace(',', Encoding(44, "10001101110", { true,  true,  false }));
//		//		encodings.emplace('-', Encoding(45, "10111011000", { true,  true,  false }));
//		//		encodings.emplace('.', Encoding(46, "10111000110", { true,  true,  false }));
//		//		encodings.emplace('/', Encoding(47, "10001110110", { true,  true,  false }));
//		//		encodings.emplace('0', Encoding(48, "11101110110", { true,  true,  true }));   // 子集C有效
//		//		encodings.emplace('1', Encoding(49, "11010001110", { true,  true,  true }));
//		//		encodings.emplace('@', Encoding(64, "11110001010", { true,  true,  false }));
//		//		encodings.emplace('A', Encoding(65, "10100110000", { true,  false, false }));  // 子集A专用
//		//		encodings.emplace('B', Encoding(66, "10100001100", { true,  false, false }));
//		//		encodings.emplace('Z', Encoding(90, "11101100010", { true,  false, false }));
//		//		encodings.emplace('[', Encoding(91, "11100011010", { true,  false, false }));
//		//		encodings.emplace('\\', Encoding(92, "11101111010", { true,  false, false }));
//		//		encodings.emplace(']', Encoding(93, "11001000010", { true,  false, false }));
//		//		encodings.emplace('^', Encoding(94, "11110001010", { true,  false, false }));
//		//		encodings.emplace('_', Encoding(95, "10100110000", { true,  false, false }));
//		//		encodings.emplace('`', Encoding(96, "11101011100", { false, true,  false }));
//		//		encodings.emplace('a', Encoding(97, "11110101000", { false, true,  false }));
//		//		encodings.emplace(127, Encoding(127, "11110010110", { false, true,  false })); // DEL
//		//	}
//		//	// 输入验证（ASCII 0-127）
//		//	void validateInput(const std::string& data) {
//		//		for (char c : data) {
//		//			if (static_cast<unsigned char>(c) > 127) {
//		//				throw std::invalid_argument("包含非ASCII字符");
//		//			}
//		//		}
//		//	}
//
//		//	auto getEncoding(int codeValue) {
//		//		for (const auto& entry : encodings) {
//		//			if (entry.second.value == codeValue) {
//		//				//return entry.second.pattern;
//		//				return entry.second;
//		//			}
//		//		}
//		//		throw std::invalid_argument("无效编码值");
//		//	}
//		//	// 智能子集选择算法
//		//	Code128Subset SelectBestSubset(const std::string& data) {
//		//		size_t maxDigitRun = 0, currentRun = 0;
//		//		for (char c : data) {
//		//			if (isdigit(c)) {
//		//				if (++currentRun > maxDigitRun) maxDigitRun = currentRun;
//		//			}
//		//			else {
//		//				currentRun = 0;
//		//			}
//		//		}
//		//		return (maxDigitRun >= 4) ? Code128Subset::C :
//		//			(hasControlChars(data)) ? Code128Subset::A : Code128Subset::B;
//		//	}
//
//		//	// 数据处理流程
//		//	void processData(const std::string& data) {
//		//		generateCodeSequence(data);
//		//		calculateCheckSum();
//		//	}
//
//		//	// 生成编码序列
//		//	void generateCodeSequence(const std::string& data) {
//		//		codeSequence.clear();
//		//		addStartCharacter();
//		//		encodeDataCharacters(data);
//		//	}
//
//		//	void addStartCharacter() {
//		//		std::string startKey;
//		//		switch (currentSubset) {
//		//		case Code128Subset::A: startKey = "START_A"; break;
//		//		case Code128Subset::B: startKey = "START_B"; break;
//		//		case Code128Subset::C: startKey = "START_C"; break;
//		//		default: throw std::runtime_error("无效的 Code128 子集");
//		//		}
//
//		//		codeSequence.push_back(encodings.at(startKey).pattern);
//		//		checkSum = encodings.at(startKey).value; // 起始字符值计入校验
//		//	}
//
//		//	void encodeDataCharacters(const std::string& data) {
//		//		int weight = 1;
//		//		for (char c : data) {
//		//			handleSubsetSwitching(c);
//		//			const auto& encoding = getEncoding(c);
//		//			codeSequence.push_back(encoding.pattern);
//		//			checkSum += encoding.value * weight++;
//		//		}
//		//	}
//		//	void handleSubsetSwitching(char c) {
//		//		// 示例：自动切换到子集C处理连续数字
//		//		if (currentSubset != Code128Subset::C &&
//		//			isdigit(c) && checkNextDigits(3))
//		//		{
//		//			codeSequence.push_back(getEncoding((int)Code128Subset::C).pattern);
//		//			checkSum += 99 * (codeSequence.size() - 1); // 校验值调整
//		//			currentSubset = Code128Subset::C;
//		//		}
//		//	}
//		//	bool hasControlChars(const std::string& data) const {
//		//		for (char c : data) {
//		//			const unsigned char uc = static_cast<unsigned char>(c);
//		//			if ((uc <= 31) || (uc == 127)) { // 控制字符范围
//		//				return true;
//		//			}
//		//		}
//		//		return false;
//		//	}
//
//		//	// 校验位计算（模103）
//		//	void calculateCheckSum() {
//		//		checkSum %= 103;
//		//		codeSequence.push_back(getEncoding(checkSum).pattern);
//		//		codeSequence.push_back(encodings.at("STOP").pattern);
//		//	}
//
//		//	bool checkNextDigits(int count) {
//		//		// 检查后续字符是否满足数字条件
//		//		// 实际需实现游标跟踪
//		//		return true;
//		//	}
//
//		//	// 动态生成条空图案
//		//	void generatePattern() {
//		//		pattern.clear();
//		//		for (const auto& seq : codeSequence) {
//		//			pattern += seq;
//		//		}
//		//	}
//
//		//	// 图像渲染
//		//	void renderBarcode() {
//		//		const int totalWidth = (2 * quietZone + pattern.size()) * moduleWidth;
//		//		initializeImage(totalWidth, barHeight);
//
//		//		int x = quietZone * moduleWidth;
//		//		for (char c : pattern) {
//		//			if (c == '1') { // '1'代表条
//		//				drawBar(x, moduleWidth, barHeight);
//		//			}
//		//			x += moduleWidth;
//		//		}
//		//	}
//
//		//	// 编码数据结构
//		//	struct Encoding {
//		//		int value;
//		//		std::string pattern;
//		//		std::array<bool, 3> validSubset; // A/B/C
//		//		Encoding() = default;
//		//		Encoding(int v, const std::string& p, std::initializer_list<bool> subsets)
//		//			: value(v), pattern(p) {
//		//			std::vector<bool> temp(subsets); // 转换为 vector<bool>
//		//			std::copy_n(temp.begin(), 3, validSubset.begin()); // 复制到 std::array
//		//		}
//		//	};
//
//		//	std::unordered_map<std::string, Encoding> encodings;
//		//	std::vector<std::string> codeSequence;
//		//	Code128Subset currentSubset;
//		//	int checkSum = 0;
//		//	std::string pattern;
//	};
//}
namespace BarCode {
	class Code128 final : public CodeBarcode {
		using CodeBarcode::CodeBarcode;

	protected:
		enum class Action { OUTPUT, SWITCH_A, SWITCH_B, SWITCH_C };
		enum class CodeSet { A, B, C };
		// =========================
		// 1. 输入合法性校验（Set B）
		// =========================
		bool isValidChar(char c) const override {
			unsigned char uc = static_cast<unsigned char>(c);
			return uc >= 0 && uc <= 127;
		}
		// =========================
		// 2. 准备内部编码数据
		//    （这里只做“语义准备”）
		// =========================
		std::string prepareEncodedData(
			const std::string& userData) const override
		{
			// 对于 baseline Set B：
			// 不做任何转换，直接返回
			return userData;
		}
		// =========================
		// 3. 校验字符计算（返回 symbol value）
		// =========================
		char calculateCheckDigit(const std::string& encoded) override {
			return 0;
		}
		// =========================
		// 4. 构建条码元素
		// =========================
		void buildElements(const std::string& data) override {
			elements.clear();
			validateInput(data);

			CodeSet currentSet = CodeSet::B;      // 默认 Start B
			appendPattern(104);                   // START B
			int checksum = 104;
			int position = 1;

			size_t i = 0;
			while (i < data.size()) {
				char c = data[i];
				Action action = Action::OUTPUT;

				// 判断是否可切换到 Set C
				if (currentSet != CodeSet::C && canUseSetC(data, i)) {
					action = Action::SWITCH_C;
				}
				else if (isControlChar(c) && currentSet != CodeSet::A) {
					action = Action::SWITCH_A;
				}
				else if (!isControlChar(c) && currentSet == CodeSet::A) {
					action = Action::SWITCH_B;  // 控制字符结束，切回 B
				}

				switch (action) {
				case Action::SWITCH_C:
					appendPattern(99); // CODE C
					checksum += 99 * position++;
					currentSet = CodeSet::C;
					continue;

				case Action::SWITCH_A:
					appendPattern(101); // CODE A
					checksum += 101 * position++;
					currentSet = CodeSet::A;
					continue;

				case Action::SWITCH_B:
					appendPattern(100); // CODE B
					checksum += 100 * position++;
					currentSet = CodeSet::B;
					continue;

				case Action::OUTPUT:
					if (currentSet == CodeSet::C) {
						int value = (data[i] - '0') * 10 + (data[i + 1] - '0');
						appendPattern(value);
						checksum += value * position++;
						i += 2;

						if (i < data.size() && !isdigit(data[i])) {
							appendPattern(100); // CODE B
							checksum += 100 * position++;
							currentSet = CodeSet::B;
						}
					}
					else {
						int value = charToValue(c, currentSet);
						appendPattern(value);
						checksum += value * position++;
						i++;
					}
					break;
				}
			}

			// 校验和
			checksum %= 103;
			appendPattern(checksum);

			// STOP
			appendPattern(106);
		}
		// =========================
		// 5. 人类可读文本（可选）
		// =========================
		void addLabels() override {
			if (!_showLabels) return;

			int textY = barHeight + guardExtension + 10;
			int x = quietZone * moduleWidth;

			size_t eIndex = 0; // 元素索引

			for (size_t i = 0; i < fullData.size(); ++i) {
				char c = fullData[i];
				if (c < 32 || c > 126) continue; // 控制字符不显示

				// 累计这个字符对应的总宽度
				int charWidth = 0;

				// 每个字符对应 6~8 个元素
				int patternsPerChar = 6; // Code128 每个字符有 6 个条/空模块对（固定）
				for (int p = 0; p < patternsPerChar && eIndex < elements.size(); ++p) {
					charWidth += elements[eIndex].modules * moduleWidth;
					eIndex++;
				}

				// 居中绘制
				cv::putText(barcodeImage,
					std::string(1, c),
					cv::Point(x + charWidth / 2, textY),
					cv::FONT_HERSHEY_SIMPLEX,
					fontScale,
					cv::Scalar(0),
					fontThickness);

				x += charWidth;
			}
		}
	private:
		static inline const std::array<std::string, 107> CODE128_PATTERNS = {
			/*  0 */ "212222",
			/*  1 */ "222122",
			/*  2 */ "222221",
			/*  3 */ "121223",
			/*  4 */ "121322",
			/*  5 */ "131222",
			/*  6 */ "122213",
			/*  7 */ "122312",
			/*  8 */ "132212",
			/*  9 */ "221213",
			/* 10 */ "221312",
			/* 11 */ "231212",
			/* 12 */ "112232",
			/* 13 */ "122132",
			/* 14 */ "122231",
			/* 15 */ "113222",
			/* 16 */ "123122",
			/* 17 */ "123221",
			/* 18 */ "223211",
			/* 19 */ "221132",
			/* 20 */ "221231",
			/* 21 */ "213212",
			/* 22 */ "223112",
			/* 23 */ "312131",
			/* 24 */ "311222",
			/* 25 */ "321122",
			/* 26 */ "321221",
			/* 27 */ "312212",
			/* 28 */ "322112",
			/* 29 */ "322211",
			/* 30 */ "212123",
			/* 31 */ "212321",
			/* 32 */ "232121",
			/* 33 */ "111323",
			/* 34 */ "131123",
			/* 35 */ "131321",
			/* 36 */ "112313",
			/* 37 */ "132113",
			/* 38 */ "132311",
			/* 39 */ "211313",
			/* 40 */ "231113",
			/* 41 */ "231311",
			/* 42 */ "112133",
			/* 43 */ "112331",
			/* 44 */ "132131",
			/* 45 */ "113123",
			/* 46 */ "113321",
			/* 47 */ "133121",
			/* 48 */ "313121",
			/* 49 */ "211331",
			/* 50 */ "231131",
			/* 51 */ "213113",
			/* 52 */ "213311",
			/* 53 */ "213131",
			/* 54 */ "311123",
			/* 55 */ "311321",
			/* 56 */ "331121",
			/* 57 */ "312113",
			/* 58 */ "312311",
			/* 59 */ "332111",
			/* 60 */ "314111",
			/* 61 */ "221411",
			/* 62 */ "431111",
			/* 63 */ "111224",
			/* 64 */ "111422",
			/* 65 */ "121124",
			/* 66 */ "121421",
			/* 67 */ "141122",
			/* 68 */ "141221",
			/* 69 */ "112214",
			/* 70 */ "112412",
			/* 71 */ "122114",
			/* 72 */ "122411",
			/* 73 */ "142112",
			/* 74 */ "142211",
			/* 75 */ "241211",
			/* 76 */ "221114",
			/* 77 */ "413111",
			/* 78 */ "241112",
			/* 79 */ "134111",
			/* 80 */ "111242",
			/* 81 */ "121142",
			/* 82 */ "121241",
			/* 83 */ "114212",
			/* 84 */ "124112",
			/* 85 */ "124211",
			/* 86 */ "411212",
			/* 87 */ "421112",
			/* 88 */ "421211",
			/* 89 */ "212141",
			/* 90 */ "214121",
			/* 91 */ "412121",
			/* 92 */ "111143",
			/* 93 */ "111341",
			/* 94 */ "131141",
			/* 95 */ "114113",
			/* 96 */ "114311",
			/* 97 */ "411113",
			/* 98 */ "411311",
			/* 99 */ "113141",
			/*100 */ "114131",
			/*101 */ "311141",
			/*102 */ "411131",
			/*103 */ "211412", // Start A
			/*104 */ "211214", // Start B
			/*105 */ "211232", // Start C
			/*106 */ "2331112" // Stop (7 elements)
		};
		// =========================
		// 常量定义
		// =========================
		static constexpr int START_B = 104;
		static constexpr int STOP = 106;

		bool isControlChar(char c) const {
			return c >= 0 && c <= 31; // 控制字符需用 Set A
		}
		// =========================
		// Pattern 转 CodeElement
		// =========================
		void appendSymbol(int symbolValue) {
			const std::string& pattern = CODE128_PATTERNS[symbolValue];

			bool isBar = true;
			for (char c : pattern) {
				elements.push_back({
					isBar,
					c - '0'
					});
				isBar = !isBar;
			}
		}

		static bool canUseSetC(const std::string& s, size_t pos) {
			size_t count = 0;
			while (pos + count < s.size() && std::isdigit(s[pos + count]))
				count++;

			return count >= 4 && count % 2 == 0;
		}

		int charToValue(char c, CodeSet set) const {
			if (set == CodeSet::A) {
				if (c >= 0 && c <= 95) return (c <= 31) ? (c + 64) : (c - 32);
			}
			else if (set == CodeSet::B) {
				return c - 32; // ASCII 32~127
			}
			return -1; // Set C handled separately
		}
		void appendPattern(int value) {
			if (value < 0 || value > 106) return;
			const std::string& pattern = CODE128_PATTERNS[value];
			bool isBar = true;
			for (char c : pattern) {
				elements.push_back({ isBar, c - '0' });
				isBar = !isBar;
			}
		}
	};
}
#endif