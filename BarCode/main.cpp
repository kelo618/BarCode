#include "ean13.h"
#include "ean8.h"
#include "code128.h"
#include "code39.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace barcode {
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
			case BarcodeType::Code128:
				return std::make_unique<Code128>(size);
			default:
				throw std::invalid_argument("Unsupported barcode type");
			}
		}

		template<typename T>
		static std::unique_ptr<T> create(BarcodeSize size) {
			static_assert(std::is_base_of_v<Barcode, T>, "T must derive from Barcode");
			return std::make_unique<T>(size);
		}
	};
}

namespace {
	bool runSelfTest() {
		using namespace barcode;

		// Valid EAN-13 input should render a non-empty image.
		{
			auto ean13 = BarcodeFactory::create<EAN13>(BarcodeSize::MINIMUM);
			ean13->showLabels(false);
			ean13->encode("590123412345");
			if (ean13->getImage().empty()) return false;
		}

		// Invalid EAN-8 input should throw.
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

		// Code39 should render successfully for valid content.
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
		barcode->encode("wo-yao-qu-sa-niao");
		barcode->show();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
