#include "BarcodeFactory.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {
	bool runSelfTest() {
		using namespace barcode;

		// 1. 验证 EAN-13 合法输入能生成图像
		{
			auto ean13 = BarcodeFactory::create<EAN13>(BarcodeSize::MINIMUM);
			ean13->showLabels(false);
			ean13->encode("590123412345");
			if (ean13->getImage().empty()) return false;
		}

		// 2. 验证 EAN-8 非法输入会抛出异常
		{
			auto ean8 = BarcodeFactory::create<EAN8>(BarcodeSize::MINIMUM);
			bool thrown = false;
			try {
				ean8->encode("ABC1234");
			}
			catch (const std::invalid_argument&) {
				thrown = true;
			}
			if (!thrown) return false;
		}

		// 3. 验证 Code39 合法内容能完成渲染
		{
			auto code39 = BarcodeFactory::create<Code39>(BarcodeSize::MINIMUM);
			code39->showLabels(false);
			code39->encode("ABC-123");
			if (code39->getImage().empty()) return false;
		}

		return true;
	}
}

int main(int argc, char** argv) {
	using namespace barcode;
	try {
		if (argc > 1 && std::string(argv[1]) == "--self-test") {
			if (!runSelfTest()) {
				std::cerr << "Self-test failed" << std::endl;
				return 1;
			}
			std::cout << "Self-test passed" << std::endl;
			return 0;
		}

		auto barcode = BarcodeFactory::create<Code128>(BarcodeSize::MINIMUM);
		barcode->showLabels(false);
		barcode->encode("KSGM6PQ7Q2410S0772");
		barcode->show();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
