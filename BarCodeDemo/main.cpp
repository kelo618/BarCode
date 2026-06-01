#include "BarcodeFactory.h"

#include <iostream>

int main() {
	using namespace barcode;
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
	try {
		auto barcode = BarcodeFactory::create<EAN8>(BarcodeSize::STANDARD);
		//barcode->showLabels(false);
		barcode->encode("1234578");
		barcode->show("BarCode Demo");
		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << "Demo failed: " << e.what() << std::endl;
		return 1;
	}
}
