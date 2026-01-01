#include "ean13.h"
#include "ean8.h"
#include "code128.h"
#include "code39.h"

namespace BarCode {
	class BarcodeFactory {
	public:
		static std::unique_ptr<Barcode> create(BarcodeType type, BarcodeSize size) {
			switch (type) {
			case BarcodeType::EAN13:
				return std::make_unique<EAN13>(size);
			case BarcodeType::EAN8:
				return std::make_unique<EAN8>(size);
			case BarcodeType::Code39:
				return std::make_unique<Code39>(size);
			case BarcodeType::Code93:
				//return std::make_unique<Code128>(size);
			case BarcodeType::Code128:
				//return std::make_unique<Code128>(size);
			default:
				throw std::invalid_argument("Unsupported barcode type");
			}
		}

		template<typename T>
		static std::unique_ptr<T> create(BarcodeSize size) {
			static_assert(std::is_base_of_v<Barcode, T>,
				"T must derive from Barcode");
			return std::make_unique<T>(size);
		};
	};
}


int main() {
	using namespace BarCode;
	try
	{
		auto barcode = BarcodeFactory::create<EAN8>(BarcodeSize::MINIMUM);

		try {
			barcode->encode("7654321");
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
