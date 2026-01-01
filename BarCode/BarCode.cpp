#include "BarCode.h"

/**
* @brief 显示条形码图像（调试 / GUI 使用）
* @param barcode_name 显示窗口名称
*
* 若在 encode() 前调用，将抛出异常。
*/
namespace barcode
{
	void Barcode::show(const std::string& barcode_name) {
		if (!barcodeImage.empty()) {
			cv::imshow(barcode_name, barcodeImage);
			cv::waitKey(0);
		}
		else throw std::runtime_error("Barcode image is empty. Please encode data first.");
	}

	/**
	* @brief 将条形码图像保存为文件
	* @param filename 输出文件路径
	*
	* 若在 encode() 前调用，将抛出异常。
	*/
	void Barcode::save(const std::string& filename) {
		if (!barcodeImage.empty())
			cv::imwrite(filename, barcodeImage);
		else throw std::runtime_error("Barcode image is empty. Please encode data first.");
	}

	/**
	* @brief 获取生成的条形码图像
	* @return 条形码图像（深拷贝）
	*/
	cv::Mat Barcode::getImage() const { return barcodeImage.clone(); }

	/**
	* @brief 是否显示可读标签
	* @param enable 是否启用
	*
	* 并非所有条码制都支持标签显示。
	*/
	void Barcode::showLabels(bool enable) {
		_showLabels = enable;
	}

	/// 初始化空白画布
	void Barcode::initializeImage(int width, int height) {
		barcodeImage = cv::Mat(height, width, CV_8UC1, cv::Scalar(255));
	}

	/// 绘制单个黑条
	void Barcode::drawBar(int x, int width, int height) {
		cv::rectangle(barcodeImage,
			cv::Point(x, 0),
			cv::Point(x + width - 1, height - 1),
			cv::Scalar(0), cv::FILLED);
	}

	/**
	* @brief 设置条形码尺寸参数
	* @param size 尺寸枚举
	*/
	void Barcode::setSizeParameters(BarcodeSize size) {
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

	EanBarcode::EanBarcode(BarcodeSize size) {
		setSizeParameters(size);
	}

	/**
	* @brief 执行 EAN 条形码的完整编码流程
	*
	* 编码流程：
	*  1. 输入校验
	*  2. 数据处理（补校验位等）
	*  3. 生成条形码比特模式
	*  4. 渲染条形码图像
	*  5. 绘制可读文本
	*
	* 派生类不应重写此函数，而应实现：
	*  - processData()
	*  - generatePattern()
	*/
	void EanBarcode::encode(const std::string& code) {
		validateInput(code);
		processData(code);
		generatePattern();
		renderImage();
		addLabels();
	}

	void EanBarcode::renderImage() {
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

	void EanBarcode::addLabels() {
		if (!_showLabels) return;
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

	/// EAN通用校验
	void EanBarcode::validateInput(const std::string& code) {
		if (code.empty())
			throw std::invalid_argument("Barcode data is empty");

		if (code.find_first_not_of("0123456789") != std::string::npos) {
			throw std::invalid_argument("EAN barcode accepts digits only (0-9)");
		}
	}

	/// 保护条检测
	bool EanBarcode::isLeftGuard(size_t pos) const { return pos < 3; }

	bool EanBarcode::isCenterGuard(size_t pos) const { return (pos >= 45 && pos < 50); }

	bool EanBarcode::isRightGuard(size_t pos, size_t totalLength) const { return pos >= totalLength - 3; }

	std::string EanBarcode::L_encode(char c) {
		static const std::unordered_map<char, std::string> L = {
			{ '0',"0001101" },{ '1',"0011001" },{ '2',"0010011" },
			{ '3',"0111101" },{ '4',"0100011" },{ '5',"0110001" },
			{ '6',"0101111" },{ '7',"0111011" },{ '8',"0110111" },
			{ '9',"0001011" }
		};
		return L.at(c);
	}

	std::string EanBarcode::R_encode(char c) {
		static const std::unordered_map<char, std::string> R = {
			{ '0',"1110010" },{ '1',"1100110" },{ '2',"1101100" },
			{ '3',"1000010" },{ '4',"1011100" },{ '5',"1001110" },
			{ '6',"1010000" },{ '7',"1000100" },{ '8',"1001000" },
			{ '9',"1110100" }
		};
		return R.at(c);
	}

	/**
	* @brief 构造 Code 系列条形码对象
	* @param size 条形码尺寸规格
	*
	* 构造函数会根据尺寸枚举初始化模块宽度、
	* 条高度、静区等参数。
	*/
	CodeBarcode::CodeBarcode(BarcodeSize size) {
		setSizeParameters(size);
	}

	/**
	* @brief 执行 Code 系列条形码的完整编码流程
	* @param data 用户输入的原始数据
	*
	* 该函数为模板方法，定义了 Code 系列条形码的统一编码流程。
	* 派生类不应重写该函数，而应通过实现钩子函数来自定义行为。
	*
	* 编码流程：
	*  1. validateInput()      —— 校验用户输入
	*  2. prepareEncodedData() —— 内部数据准备（如添加起止符）
	*  3. buildElements()      —— 构建条/空白模块序列
	*  4. renderImage()        —— 渲染条形码
	*  5. addLabels()          —— 绘制可读文本（可选）
	*/
	void CodeBarcode::encode(const std::string& data) {
		validateInput(data);					//  只校验用户输入
		fullData = prepareEncodedData(data);	// 内部转换
		elements.clear();
		buildElements(fullData);				// 编码用完整数据
		renderImage();
		if (_showLabels)
			addLabels();
	}

	/**
	* @brief 将用户输入转换为内部编码数据
	* @param userData 用户输入数据
	* @return 内部编码使用的数据
	*
	* 该函数用于处理如：
	*  - 添加起始/结束符
	*  - 插入校验字符
	*
	* 默认实现直接返回 userData。
	*/
	std::string CodeBarcode::prepareEncodedData(const std::string& userData) const
	{
		return userData;
	}

	/**
	* @brief 校验用户输入数据的合法性
	* @param code 用户输入的原始数据
	*
	* 默认实现会检查数据是否为空，并逐字符调用 isValidChar()。
	* 派生类可根据需要重写该函数。
	*/
	void CodeBarcode::validateInput(const std::string& code) {
		if (code.empty())
			throw std::invalid_argument("Barcode data is empty");

		for (char c : code) {
			if (!isValidChar(c))
				throw std::invalid_argument(
					std::string("Invalid character: ") + c
				);
		}
	}

	/**
	* @brief 根据模块序列渲染条形码图像
	*
	* 该实现为 Code 系列条形码的统一渲染逻辑，
	* 根据 elements 中的模块宽度顺序绘制条与空白。
	*
	* 派生类通常不需要重写该函数。
	*/
	void CodeBarcode::renderImage() {
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

	/**
	* @brief 清空当前编码状态
	*
	* 清除内部编码数据、模块序列和图像缓存，
	* 通常用于重新编码前的状态重置。
	*/
	void CodeBarcode::clear() {
		fullData.clear();
		elements.clear();
		barcodeImage.release();
	}

	/**
	* @brief 设置窄条与宽条的模块宽度
	* @param narrow 窄条模块宽度
	* @param wide   宽条模块宽度
	*
	* 主要用于 Code39 / Code93 等支持宽窄条的码制。
	* 对 Code128 等固定模块宽度的码制可能无效。
	*/
	void CodeBarcode::setBarWidth(int narrow, int wide) {
		narrowModule = narrow;
		wideModule = wide;
	}
}

