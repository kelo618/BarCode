/**
 * @file Barcode.h
 * @brief 条形码生成库的核心接口定义
 *
 * 本文件定义了条形码生成库的公共抽象接口，包括：
 *  - Barcode      ：所有条形码类型的抽象基类
 *  - EanBarcode   ：EAN-8 / EAN-13 系列的公共基类
 *  - CodeBarcode  ：Code39 / Code93 / Code128 系列的公共基类
 *
 * 设计目标：
 *  - 提供统一、稳定的条形码编码与渲染接口
 *  - 支持多种条码制的扩展
 *  - 适合封装为动态链接库（DLL / SO）
 *
 * 说明：
 *  - 本头文件定义接口，具体实现位于对应的 .cpp 文件中
 *  - 使用 OpenCV 仅作为内部图像实现手段
 *
 * @author
 * @date
 */

#pragma once
#ifndef _EAN_H
#define _EAN_H
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>

 /**
  * @namespace BarCode
  * @brief 条形码生成库的命名空间
  *
  * 所有条形码相关类型、枚举和类均定义在此命名空间中，
  * 用于避免与用户工程中的类型发生命名冲突。
  */
namespace barcode {
	/**
	 * @enum BarcodeType
	 * @brief 支持的条形码类型
	 */
	enum class BarcodeType {
		EAN8,
		EAN13,
		Code39,
		Code128
	};
	/**
	 * @enum BarcodeSize
	 * @brief 条形码尺寸规格
	 *
	 * 该枚举用于控制条形码的整体尺寸（模块宽度、条高度、静区等）。
	 * 不严格等同于各标准的物理尺寸，仅保证在常见场景下具有良好的可扫描性。
	 */
	enum class BarcodeSize {
		MINIMUM,	///< 最小可扫描尺寸
		STANDARD,	///< 标准尺寸（推荐）
		LARGE		///< 大尺寸（远距离或高分辨率场景）
	};
	/**
	 * @class Barcode
	 * @brief 所有条形码类型的抽象基类
	 *
	 * Barcode 定义了条形码生成的统一接口，包括：
	 *  - 数据编码
	 *  - 图像渲染
	 *  - 图像获取、显示与保存
	 *
	 * 设计说明：
	 *  - 不关心具体条码制（EAN / Code 等）
	 *  - encode() 由派生类实现具体编码流程
	 *  - renderImage() / addLabels() 由派生类定制
	 */
	class Barcode {
	public:
		/// 虚析构函数，确保通过基类指针正确析构派生类
		virtual ~Barcode() = default;

		/**
		* @brief 对输入数据进行编码并生成条形码图像
		* @param data 用户输入的原始数据
		*
		* 若输入不合法，应抛出 std::invalid_argument 异常。
		*/
		virtual void encode(const std::string& data) = 0;

		/**
		 * @brief 显示条形码图像（调试 / GUI 使用）
		 * @param barcode_name 显示窗口名称
		 *
		 * 若在 encode() 前调用，将抛出异常。
		 */
		virtual void show(const std::string& barcode_name = "Barcode");

		/**
		* @brief 将条形码图像保存为文件
		* @param filename 输出文件路径
		*
		* 若在 encode() 前调用，将抛出异常。
		*/
		virtual void save(const std::string& filename);

		/**
		* @brief 获取生成的条形码图像
		* @return 条形码图像（深拷贝）
		*/
		cv::Mat getImage() const;

		/**
		* @brief 是否显示可读标签
		* @param enable 是否启用
		*
		* 并非所有条码制都支持标签显示。
		*/
		virtual void showLabels(bool);
	protected:
		/* ===== 绘制基础 ===== */

		/// 初始化空白画布
		virtual	void initializeImage(int width, int height);
		/// 绘制单个黑条
		virtual void drawBar(int x, int width, int height);

		/* ===== 渲染流程钩子 ===== */

		/// 子类实现：根据编码结果绘制条形码
		virtual void renderImage() = 0;
		/// 子类实现：绘制可读标签
		virtual void addLabels() = 0;
		/// 子类实现：校验位计算（EAN / Code128 等）
		virtual char calculateCheckDigit(const std::string& code) = 0;

		/**
		* @brief 设置条形码尺寸参数
		* @param size 尺寸枚举
		*/
		virtual void setSizeParameters(BarcodeSize size);

	protected:
		/* ===== 编码结果 ===== */

		std::string fullData;			///< 编码后（内部使用）的完整数据

		/* ===== 图像 ===== */

		cv::Mat barcodeImage;

		/* ===== 尺寸参数 ===== */

		int moduleWidth = 4;			///< 单模块像素宽度
		int barHeight = 50;				///< 条高度
		int quietZone = 10;				///< 静区宽度（模块数）

		/* ===== 文本渲染 ===== */

		double fontScale = 0;
		int fontThickness = 2;

		/* ===== EAN 专用 ===== */

		int guardExtension = 8;			///< 护条延伸高度

		bool _showLabels = true;	///< 是否显示可读标签
	};

	/**
	 * @class EanBarcode
	 * @brief EAN 系列条形码的抽象基类
	 *
	 * 适用于：
	 *  - EAN-8
	 *  - EAN-13
	 *
	 * 提供：
	 *  - 输入校验
	 *  - 公共渲染逻辑
	 *  - 数字标签的居中绘制
	 */
	class EanBarcode : public Barcode {
	public:
		explicit EanBarcode(BarcodeSize size);
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
		void encode(const std::string& code) final;

	protected:
		// =========================
		// 子类必须提供的 EAN 规则
		// =========================

		/**
		 * @brief 处理用户输入数据（如补校验位、拆分结构等）
		 *
		 * 该函数只负责数据准备，不应涉及具体的条形码绘制。
		 */
		virtual void processData(const std::string& code) = 0;

		/**
		 * @brief 根据处理后的数据生成条形码比特模式
		 *
		 * 生成结果应写入 pattern 字符串，
		 * 其中 '1' 表示黑条，'0' 表示空白。
		 */
		virtual void generatePattern() = 0;

		/**
		* @brief 获取指定数字对应条形区域的模块中心位置
		* @param index 数字在 fullData 中的索引
		* @return 模块中心的 x 坐标（像素）
		*
		* 该函数用于实现数字标签的精确居中绘制，
		* 不同 EAN 规格（EAN-8 / EAN-13）应给出不同实现。
		*/
		virtual int getModuleCenterForDigit(size_t index) const = 0;

	protected:
		// =========================
		// EAN 系列公共实现
		// =========================

		void renderImage() override;

		void addLabels() override;

		/// EAN通用校验
		virtual void validateInput(const std::string& code);

	protected:
		// =========================
		// 工具函数
		// =========================

		/// 保护条检测
		bool isLeftGuard(size_t pos) const;
		bool isCenterGuard(size_t pos) const;
		bool isRightGuard(size_t pos, size_t totalLength) const;

		static std::string L_encode(char c);

		/**
		 * @brief R 码表编码
		 * @param c 数字字符 '0'~'9'
		 * @return 7位编码字符串
		 */
		static std::string R_encode(char c);

	protected:
		// =========================
		// 状态
		// =========================

		std::string pattern;
	};

	/**
	 * @class CodeBarcode
	 * @brief Code 系列条形码的抽象基类
	 *
	 * 适用于以下 Code 系列条形码：
	 *  - Code 39
	 *  - Code 128
	 *
	 * 设计特点：
	 *  - 使用“模块（module）序列”描述条形码结构
	 *  - 将编码逻辑（buildElements）与渲染逻辑解耦
	 *  - 提供统一的渲染实现，派生类只需关注编码规则
	 *
	 * 编码流程（模板方法）：
	 *  1. 校验用户输入数据
	 *  2. 生成内部编码数据（如添加起止符）
	 *  3. 构建条/空白模块序列
	 *  4. 渲染条形码图像
	 *  5. （可选）绘制可读标签
	 *
	 * 派生类应实现：
	 *  - buildElements()
	 *  - isValidChar()
	 *  - （必要时）prepareEncodedData()
	 */
	class CodeBarcode :public Barcode {
	public:
		/**
		* @brief 构造 Code 系列条形码对象
		* @param size 条形码尺寸规格
		*
		* 构造函数会根据尺寸枚举初始化模块宽度、
		* 条高度、静区等参数。
		*/
		explicit CodeBarcode(BarcodeSize size);


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
		void encode(const std::string& data) final;

	protected:
		// =========================
		// Code 元素结构
		// =========================

		/**
		 * @struct CodeElement
		 * @brief Code 系列条形码的基本绘制单元
		 *
		 * 每个 CodeElement 表示一段连续的条或空白，
		 * 其宽度以模块（module）为单位。
		 */
		struct CodeElement {
			bool isBar;     ///< 是否为黑条（true=黑条，false=空白）
			int  modules;   ///< 宽度（模块数量）
		};

	protected:
		// =========================
		// 子类必须实现
		// =========================

		/**
		* @brief 构建条形码的模块序列
		* @param data 已准备好的完整编码数据
		*
		* 派生类必须实现该函数，将编码后的数据转换为
		* 一系列 CodeElement（条 / 空白）序列。
		*
		* 该函数只负责“编码模型”，不应直接进行图像绘制。
		*/
		virtual void buildElements(const std::string& data) = 0;

		/**
		 * @brief 判断字符是否为当前码制支持的合法字符
		 * @param c 待检查字符
		 * @return true 表示合法，false 表示非法
		 */
		virtual bool isValidChar(char c) const = 0;

	protected:
		// =========================
		// 可选扩展点
		// =========================

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
		virtual std::string prepareEncodedData(
			const std::string& userData) const;

		/**
		* @brief 校验用户输入数据的合法性
		* @param code 用户输入的原始数据
		*
		* 默认实现会检查数据是否为空，并逐字符调用 isValidChar()。
		* 派生类可根据需要重写该函数。
		*/
		virtual void validateInput(const std::string& code);

	protected:
		// =========================
		// Code 系列通用实现
		// =========================

		/**
		 * @brief 根据模块序列渲染条形码图像
		 *
		 * 该实现为 Code 系列条形码的统一渲染逻辑，
		 * 根据 elements 中的模块宽度顺序绘制条与空白。
		 *
		 * 派生类通常不需要重写该函数。
		 */
		void renderImage() override;

		/**
		* @brief 清空当前编码状态
		*
		* 清除内部编码数据、模块序列和图像缓存，
		* 通常用于重新编码前的状态重置。
		*/
		void clear();

		/**
		 * @brief 设置窄条与宽条的模块宽度
		 * @param narrow 窄条模块宽度
		 * @param wide   宽条模块宽度
		 *
		 * 主要用于 Code39 / Code93 等支持宽窄条的码制。
		 * 对 Code128 等固定模块宽度的码制可能无效。
		 */
		void setBarWidth(int narrow, int wide);

	protected:
		// =========================
		// 状态
		// =========================

		std::vector<CodeElement> elements;

		int narrowModule = 1;		///< 窄条模块宽度
		int wideModule = 3;			///< 宽条模块宽度
	};
};

#endif //  _VERSION_ONE == 1