#include "argparse.h"

using namespace std;

void Args::get_args(int argc, char* argv[]) {
	CLI::App app{"Counts barcodes located in sequencing data"};

	app.add_option("-s,--sample_barcodes", sample_barcodes_file, "Sample barcodes csv file");
	app.add_option("-c,--counted_barcodes", counted_barcodes_file, "Building block barcodes csv file");

	CLI11_PARSE(app, argc, argv);
}
