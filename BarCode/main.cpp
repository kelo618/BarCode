#include "ean13.h"
#include "ean8.h"
#include "code128.h"
#include "code39.h"


namespace barcode {
	/**
	 * @class BarcodeFactory
	 * @brief 条码工厂类，用于创建不同类型的 Barcode 对象
	 *
	 * 支持类型：
	 *  - EAN-13
	 *  - EAN-8
	 *  - Code39
	 *  - Code128
	 *
	 * 提供静态方法直接创建条码对象，无需用户手动 new。
	 */
	class BarcodeFactory {
	public:
		/**
		 * @brief 根据 BarcodeType 创建对应的条码对象
		 * @param type 条码类型
		 * @param size 条码尺寸
		 * @return 对应条码对象的智能指针
		 * @throw std::invalid_argument 不支持的条码类型
		 */
		static std::unique_ptr<Barcode> create(BarcodeType type, BarcodeSize size) {
			switch (type) {
			case BarcodeType::EAN13:
				return std::make_unique<EAN13>(size);
			case BarcodeType::EAN8:
				return std::make_unique<EAN8>(size);
			case BarcodeType::Code39:
				return std::make_unique<Code39>(size);
			case BarcodeType::Code128:
				return std::make_unique<Code128>(size);
			default:
				throw std::invalid_argument("Unsupported barcode type");
			}
		}

		/**
		 * @brief 模板方法，根据类型 T 创建条码对象
		 * @tparam T 继承自 Barcode 的条码类型
		 * @param size 条码尺寸
		 * @return 对应条码对象的智能指针
		 * @note T 必须继承自 Barcode
		 */
		template<typename T>
		static std::unique_ptr<T> create(BarcodeSize size) {
			static_assert(std::is_base_of_v<Barcode, T>,
				"T must derive from Barcode");
			return std::make_unique<T>(size);
		};
	};
}


int main() {
	using namespace barcode;
	try
	{
		auto barcode = BarcodeFactory::create<Code128>(BarcodeSize::MINIMUM);

		try {
			barcode->showLabels(false);
			barcode->encode("wo-yao-qu-sa-niao");
			barcode->show();
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
