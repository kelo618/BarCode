#pragma once
#ifndef _EAN_H
#define _EAN_H
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>

#define _VERSION_ONE 2

namespace BarCode {
	// 条码类型枚举（建议扩展为独立类层次结构）
	enum class BarcodeType {
		EAN8,
		EAN13,
		Code128
	};
	// 条形码大小枚举
	enum class BarcodeSize {
		MINIMUM,	// 小尺寸 (窄条)
		STANDARD,	// 中等尺寸
		LARGE		// 大尺寸 (宽条)
	};
	// Code128的字符集
	enum class Code128Subset {
		A = 103,
		B = 104,
		C = 105
	};
#if  _VERSION_ONE == 1
	// 抽象基类
	class Barcode {
	public:
		Barcode() = default;

		Barcode(Barcode&& other) noexcept : barcodeImage(std::move(other.barcodeImage)) {}

		Barcode& operator=(Barcode&& other) noexcept {
			barcodeImage = std::move(other.barcodeImage);
			return *this;
		}

		virtual ~Barcode() = default;

		virtual void generatePattern() = 0;

		virtual void setSizeParameters(BarcodeSize size) = 0;

		virtual void encode(const std::string& code) {
			validateInput(code);
			validateInputFormat(code);
			barcodeData = code + calculateCheckDigit(code);
			generatePattern();
			createImage();
			drawBars(guardLeft, guardRight);
			drawLabels();
		};

		void show(const std::string& name = "BarCode") {
			if (barcodeImage.empty()) throw std::runtime_error("Barcode not generated");
			cv::imshow(name, barcodeImage);
			cv::waitKey(0);
		}

	protected:
		virtual void drawLabels() = 0;

		virtual char calculateCheckDigit(const std::string& code) = 0;

		virtual void validateInputFormat(const std::string& code) = 0;

		virtual void validateInput(const std::string& code) {
			if (code.empty() || code.find_first_not_of("0123456789") != std::string::npos) {
				throw std::invalid_argument("输入包含非数字字符");
			}
		}

		void createImage() {
			const int totalWidth = (quietZoneLeft + quietZoneRight + pattern.length()) * moduleWidth;
			const int totalHeight = barcodeHeight + guardExtension + 40;
			barcodeImage = cv::Mat(totalHeight, totalWidth, CV_8UC1, 255);
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

		static std::string L_encode(char c) {
			static const std::unordered_map<char, std::string> L = {
				{'0',"0001101"}, {'1',"0011001"}, {'2',"0010011"},
				{'3',"0111101"}, {'4',"0100011"}, {'5',"0110001"},
				{'6',"0101111"}, {'7',"0111011"}, {'8',"0110111"},
				{'9',"0001011"}
			};
			return L.at(c);
		}

		bool isGuardPosition(size_t index) const {
			return (index < 3) ||                   // Left guard
				(index >= guardLeft && index < guardRight) ||   // Center guard 
				(index >= pattern.size() - 3);   // Right guard
		}

		void drawBars(size_t _left, size_t _right) {
			const int xStart = quietZoneLeft * moduleWidth;
			for (size_t i = 0; i < pattern.size(); ++i) {
				if (pattern[i] == '1') {
					const int barHeight = isGuardPosition(i) ?
						barcodeHeight + guardExtension : barcodeHeight;

					cv::rectangle(barcodeImage,
						cv::Point(xStart + i * moduleWidth, 0),
						cv::Point(xStart + (i + 1) * moduleWidth - 1, barHeight),
						0, cv::FILLED);
				}
			}
		}

		cv::Mat barcodeImage;		// 最终生成的图像矩阵
		int moduleWidth = 3;		// 单模块宽度（像素）
		int barcodeHeight = 100;	// 条码主体高度（不含文字区域）
		int guardExtension = 5;		// 保护条纵向扩展高度（像素）
		int quietZoneLeft = 11;		// 左侧静区模块数（EAN标准最小值）
		int quietZoneRight = 7;		// 右侧静区模块数
		int guardLeft = 0;
		int guardRight = 0;
		double fontScale = 0.8;		// OpenCV字体缩放系数
		std::string barcodeData;	// 编码后的二进制条空序列
		std::string pattern;		// 图形化模式字符串（如"1010110..."）
	};
#else
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
		}

		virtual void save(const std::string& filename) {
			if (!barcodeImage.empty())
				cv::imwrite(filename, barcodeImage);
		}

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

		cv::Mat barcodeImage;
		int moduleWidth = 4;
		int barHeight = 50;
		int quietZone = 10; // EAN - 13 标准要求：静区 ≥ 9 modules
		int currentX = 0;
		double fontScale = 0;
		int fontThickness = 2;
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
		virtual void addLabels() = 0;
		virtual void renderImage() = 0;
		virtual char calculateCheckDigit(const std::string& code) = 0;

		// EAN通用校验
		virtual void validateInput(const std::string& code) {
			if (code.empty() || code.find_first_not_of("0123456789") != std::string::npos) {
				throw std::invalid_argument("Invalid EAN characters");
			}
		}

		// 尺寸参数设置
		void setSizeParameters(BarcodeSize size) {
			switch (size) {
			case BarcodeSize::MINIMUM:
				moduleWidth = 2; barHeight = 40; quietZone = 9; fontScale = 0.45; fontThickness = 1; break;
			case BarcodeSize::LARGE:
				moduleWidth = 4; barHeight = 80; quietZone = 15; fontScale = 0.7; fontThickness = 2; break;
			default: // STANDARD
				moduleWidth = 3; barHeight = 60; quietZone = 11; fontScale = 0.6; fontThickness = 2; break;
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

		std::string fullData;
		std::string pattern;
		int guardExtension = 8; // guardExtension >= moduleWidth * 2
	};
#endif //  _VERSION_ONE == 1
};
#endif