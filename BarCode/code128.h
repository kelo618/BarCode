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
#endif