#include "code128.h"

namespace barcode {
	/**
	* @brief 校验字符是否可用于 Code128（Set B 基线）
	* @param c 待验证字符
	* @return true 可用，false 不可用
	*/
	bool Code128::isValidChar(char c) const {
		unsigned char uc = static_cast<unsigned char>(c);
		return uc >= 0 && uc <= 127;
	}

	/**
	* @brief 准备内部编码数据（Set B baseline）
	* @param userData 用户输入字符串
	* @return 内部编码数据（可能与输入相同）
	*/
	std::string Code128::prepareEncodedData(const std::string& userData) const
	{
		return userData;
	}

	/**
	* @brief 计算校验符（返回 symbol value）
	* @param encoded 内部编码数据
	* @return 校验码 symbol value
	*/
	char Code128::calculateCheckDigit(const std::string& encoded) {
		return 0;
	}

	/**
	* @brief 构建条码元素数组
	* @param data 内部编码数据
	*/
	void Code128::buildElements(const std::string& data) {
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

	/**
	* @brief 在条码下方绘制可读字符
	*/
	void Code128::addLabels() {
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

	/**
	* @brief 判断字符是否为控制字符（需用 Set A）
	* @param c 待判断字符
	* @return true 控制字符
	*/
	bool Code128::isControlChar(char c) const {
		return c >= 0 && c <= 31; // 控制字符需用 Set A
	}

	/**
	* @brief 将 symbol value 转换为条码元素并添加到 elements
	* @param value symbol value（0~106）
	*/
	void Code128::appendPattern(int value) {
		if (value < 0 || value > 106) return;
		const std::string& pattern = CODE128_PATTERNS[value];
		bool isBar = true;
		for (char c : pattern) {
			elements.push_back({ isBar, c - '0' });
			isBar = !isBar;
		}
	}

	/**
	* @brief 将字符转换为 CodeSet 内部 value
	* @param c 字符
	* @param set 当前 CodeSet
	* @return 内部 value
	*/
	int Code128::charToValue(char c, CodeSet set) const {
		if (set == CodeSet::A) {
			if (c >= 0 && c <= 95) return (c <= 31) ? (c + 64) : (c - 32);
		}
		else if (set == CodeSet::B) {
			return c - 32; // ASCII 32~127
		}
		return -1; // Set C handled separately
	}

	/**
	* @brief 判断当前位置是否可切换到 Set C（数字压缩）
	* @param s 数据字符串
	* @param pos 当前索引
	* @return true 可以切换
	*/
	bool Code128::canUseSetC(const std::string& s, size_t pos) {
		size_t count = 0;
		while (pos + count < s.size() && std::isdigit(s[pos + count]))
			count++;

		return count >= 4 && count % 2 == 0;
	}

	const std::array<std::string, 107> Code128::CODE128_PATTERNS = {
	"212222","222122","222221","121223","121322","131222","122213","122312",
	"132212","221213","221312","231212","112232","122132","122231","113222",
	"123122","123221","223211","221132","221231","213212","223112","312131",
	"311222","321122","321221","312212","322112","322211","212123","212321",
	"232121","111323","131123","131321","112313","132113","132311","211313",
	"231113","231311","112133","112331","132131","113123","113321","133121",
	"313121","211331","231131","213113","213311","213131","311123","311321",
	"331121","312113","312311","332111","314111","221411","431111","111224",
	"111422","121124","121421","141122","141221","112214","112412","122114",
	"122411","142112","142211","241211","221114","413111","241112","134111",
	"111242","121142","121241","114212","124112","124211","411212","421112",
	"421211","212141","214121","412121","111143","111341","131141","114113",
	"114311","411113","411311","113141","114131","311141","411131","211412",
	"211214","211232","2331112"
	};
}

