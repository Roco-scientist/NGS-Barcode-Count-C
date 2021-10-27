#include <iostream>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"
#include "info.h"

using namespace std;

int main(int argc, char* argv[]) {
	// Get command line arguments
	CLI::App app{"Counts barcodes located in sequencing data"};

	std::string sample_barcodes_file = "default";
	app.add_option("-s,--sample_barcodes", sample_barcodes_file, "Sample barcodes csv file");

	std::string counted_barcodes_file = "default";
	app.add_option("-c,--counted_barcodes", counted_barcodes_file, "Building block barcodes csv file");

	CLI11_PARSE(app, argc, argv);

	// Get all DNA barcode conversion data
	info::BarcodeConversion barcode_info;
	barcode_info.sample_barcode_conversion(sample_barcodes_file);
	barcode_info.barcode_file_conversion(counted_barcodes_file);
	return 0;
}
