#include "ean13.h"
#include "ean8.h"
#include "code128.h"

namespace BarCode {
	class BarcodeFactory {
	public:
		static std::unique_ptr<Barcode> create(BarcodeType type, BarcodeSize size) {
			switch (type) {
			case BarcodeType::EAN13:
				return std::make_unique<EAN13>(size);
			case BarcodeType::EAN8:
				return std::make_unique<EAN8>(size);
			case BarcodeType::Code128:
				//return std::make_unique<Code128>(size);
			default:
				throw std::invalid_argument("Unsupported barcode type");
			}
		}
	};
}


int main() {
	try
	{
		auto barcode = BarCode::BarcodeFactory::create(
			BarCode::BarcodeType::EAN13,
			BarCode::BarcodeSize::STANDARD
		);

		//auto barcode2 = BarCode::BarcodeFactory::create(
		//	BarCode::BarcodeType::EAN8,
		//	BarCode::BarcodeSize::LARGE
		//);

		// 创建中型Code128-B条码
		//auto code128 = BarCode::BarcodeFactory::create(BarCode::BarcodeType::Code128, BarCode::BarcodeSize::MEDIUM);

		try {
			//code128->encode("ABC-123");
			//code128->show();

			//barcode2->encode("1234567");
			//barcode2->show();
			barcode->encode("931177059902");
			barcode->show();
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}



		//shared_ptr<Code128Barcode> barcode = make_shared<Code128Barcode>();
		//barcode->encode("Hello123");
		//barcode->show();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
