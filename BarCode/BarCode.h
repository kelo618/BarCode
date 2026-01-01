#pragma once
#ifndef _EAN_H
#define _EAN_H
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
// EAN / Code 系列统一成一个更高层 Pipeline

namespace BarCode {
	// 条码类型枚举
	enum class BarcodeType {
		EAN8,
		EAN13,
		Code39,
		Code93,
		Code128
	};
	// 条形码大小枚举
	enum class BarcodeSize {
		MINIMUM,	// 小尺寸 (窄条)
		STANDARD,	// 中等尺寸
		LARGE		// 大尺寸 (宽条)
	};
	// 抽象基类
	class Barcode {
	public:
		virtual ~Barcode() = default;

		virtual void encode(const std::string& data) = 0;

		virtual void show(const std::string& barcode_name = "Barcode") {
			if (!barcodeImage.empty()) {
				cv::imshow(barcode_name, barcodeImage);
				cv::waitKey(0);
			}
			else throw std::runtime_error("Barcode image is empty. Please encode data first.");
		}

		virtual void save(const std::string& filename) {
			if (!barcodeImage.empty())
				cv::imwrite(filename, barcodeImage);
			else throw std::runtime_error("Barcode image is empty. Please encode data first.");
		}

		cv::Mat getImage() const { return barcodeImage.clone(); }

		virtual void showLabels(bool) {}
	protected:
		// 基础绘制方法
		virtual	void initializeImage(int width, int height) {
			barcodeImage = cv::Mat(height, width, CV_8UC1, cv::Scalar(255));
		}

		virtual void drawBar(int x, int width, int height) {
			cv::rectangle(barcodeImage,
				cv::Point(x, 0),
				cv::Point(x + width - 1, height - 1),
				cv::Scalar(0), cv::FILLED);
		}

		virtual void renderImage() = 0;

		virtual void addLabels() = 0;

		virtual char calculateCheckDigit(const std::string& code) = 0;
		// 尺寸参数设置
		virtual void setSizeParameters(BarcodeSize size) {
			switch (size) {
			case BarcodeSize::MINIMUM:
				moduleWidth = 3;      // 最小条宽，保证 Code128 可扫描
				barHeight = 50;     // 足够高，保证 EAN/Code 系列可读
				quietZone = 10;     // ISO 推荐 ≥9 modules
				fontScale = 0.45;   // 文字可读
				fontThickness = 1;
				break;
			case BarcodeSize::STANDARD:
				moduleWidth = 4;
				barHeight = 60;
				quietZone = 11;
				fontScale = 0.6;
				fontThickness = 2;
				break;
			case BarcodeSize::LARGE:
				moduleWidth = 6;
				barHeight = 100;
				quietZone = 15;
				fontScale = 0.8;
				fontThickness = 2;
				break;
			}
		}



		std::string fullData;
		cv::Mat barcodeImage;
		int moduleWidth = 4;
		int barHeight = 50;
		int quietZone = 10; // EAN - 13 标准要求：静区 ≥ 9 modules
		double fontScale = 0;
		int fontThickness = 2;
		int guardExtension = 8; // guardExtension >= moduleWidth * 2
	};

	// EAN系列抽象基类
	class EanBarcode : public Barcode {
	public:
		explicit EanBarcode(BarcodeSize size) {
			setSizeParameters(size);
		}

		void encode(const std::string& code) override {
			validateInput(code);
			processData(code);
			generatePattern();
			renderImage();
			addLabels();
		}

	protected:
		virtual void processData(const std::string& code) = 0;
		virtual void generatePattern() = 0;

		// 子类必须提供：返回第 i 位数字的模块中心位置
		virtual int getModuleCenterForDigit(size_t index) const = 0;

		// 统一绘制文字
		void addLabels() {
			const int textY = barHeight + guardExtension + 15;

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
					cv::Scalar(0),
					fontThickness
				);
				};

			for (size_t i = 0; i < fullData.size(); ++i) {
				int centerX = getModuleCenterForDigit(i);
				drawCenteredText(fullData.substr(i, 1), centerX);
			}
		}

		void renderImage() {
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

		// EAN通用校验
		virtual void validateInput(const std::string& code) {
			if (code.empty())
				throw std::invalid_argument("Barcode data is empty");

			if (code.find_first_not_of("0123456789") != std::string::npos) {
				throw std::invalid_argument("Invalid EAN characters");
			}
		}

		// 保护条检测
		bool isLeftGuard(size_t pos) const { return pos < 3; }
		bool isCenterGuard(size_t pos) const { return (pos >= 45 && pos < 50); }
		bool isRightGuard(size_t pos, size_t totalLength) const { return pos >= totalLength - 3; }

		static std::string L_encode(char c) {
			static const std::unordered_map<char, std::string> L = {
				{'0',"0001101"}, {'1',"0011001"}, {'2',"0010011"},
				{'3',"0111101"}, {'4',"0100011"}, {'5',"0110001"},
				{'6',"0101111"}, {'7',"0111011"}, {'8',"0110111"},
				{'9',"0001011"}
			};
			return L.at(c);
		}

		static std::string R_encode(char c) {
			static const std::unordered_map<char, std::string> R = {
				{'0',"1110010"}, {'1',"1100110"}, {'2',"1101100"},
				{'3',"1000010"}, {'4',"1011100"}, {'5',"1001110"},
				{'6',"1010000"}, {'7',"1000100"}, {'8',"1001000"},
				{'9',"1110100"}
			};
			return R.at(c);
		}

		std::string pattern;
	};

	// Code系列抽象基类
	class CodeBarcode :public Barcode {
	public:
		explicit CodeBarcode(BarcodeSize size) {
			setSizeParameters(size);
		}

		void encode(const std::string& data) final {
			validateInput(data);           //  只校验用户输入
			fullData = prepareEncodedData(data);  // 内部转换
			elements.clear();
			buildElements(fullData);           // 编码用完整数据
			renderImage();
			if (_showLabels)
				addLabels();
		}

		void showLabels(bool enable) override {
			_showLabels = enable;
		}
	protected:
		struct CodeElement {
			bool isBar;     // true = 黑条, false = 空白
			int  modules;   // 宽度（以 module 为单位）
		};
		std::vector<CodeElement> elements;

		// 子类必须实现：数据 → elements
		virtual void buildElements(const std::string& data) = 0;
		// Code 系列统一渲染逻辑
		void renderImage() override {
			int totalModules = quietZone * 2;
			for (const auto& e : elements)
				totalModules += e.modules;

			int extraHeight = _showLabels ? static_cast<int>(fontScale * 40) + 10 : 0;
			int totalHeight = barHeight + extraHeight;

			initializeImage(totalModules * moduleWidth, totalHeight);

			int x = quietZone * moduleWidth;

			for (const auto& e : elements) {
				if (e.isBar) {
					drawBar(x, e.modules * moduleWidth, barHeight);
				}
				x += e.modules * moduleWidth;
			}
		}

		void clear() {
			fullData.clear();
			elements.clear();
			barcodeImage.release();
		}

		void setBarWidth(int narrow, int wide) {
			narrowModule = narrow;
			wideModule = wide;
		}

		// Code通用校验
		virtual void validateInput(const std::string& code) {
			if (code.empty())
				throw std::invalid_argument("Barcode data is empty");

			for (char c : code) {
				if (!isValidChar(c))
					throw std::invalid_argument(
						std::string("Invalid character: ") + c
					);
			}
		}

		virtual bool isValidChar(char c) const = 0;

		virtual std::string prepareEncodedData(
			const std::string& userData) const
		{
			return userData;
		}

		bool _showLabels = true;

		int narrowModule = 1;
		int wideModule = 3;
	};
};
#endif //  _VERSION_ONE == 1