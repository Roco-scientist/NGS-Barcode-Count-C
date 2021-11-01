#include <atomic>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"
// #include "argparse.h"
#include "info.h"
#include "input.h"
#include "parse.h"

using namespace std;

int main(int argc, char* argv[]) {
	// argparse::Args args;
	// args.get_args(argc, *argv);
	// Get command line arguments
	CLI::App app{"Counts barcodes located in sequencing data"};

	std::string sample_barcodes_file = "default";
	app.add_option("-s,--sample_barcodes", sample_barcodes_file,
		       "Sample barcodes csv file");

	std::string counted_barcodes_file = "default";
	app.add_option("-c,--counted_barcodes", counted_barcodes_file,
		       "Building block barcodes csv file");

	std::string format_file;
	app.add_option("-q,--sequence_format", format_file,
		       "Sequence format file")
	    ->required();

	std::string fastq_path;
	app.add_option("-f,--fastq", fastq_path, "Fastq file path")->required();

	CLI11_PARSE(app, argc, argv);

	// Get all DNA barcode conversion data
	info::BarcodeConversion barcode_info;
	barcode_info.sample_barcode_conversion(&sample_barcodes_file);
	barcode_info.barcode_file_conversion(&counted_barcodes_file);
	// barcode_info.print();

	// Convert format file
	info::SequenceFormat sequence_format;
	sequence_format.build_regex(&format_file);
	sequence_format.print();

	atomic<bool> exit_thread;
	exit_thread.store(false);
	input::Sequences sequences;
	thread reader([&sequences, &fastq_path]() {
		input::read_fastq(&fastq_path, sequences);
	});
	thread parser([&sequences]() { parse::sequence(sequences); });
	reader.join();
	parser.join();
	return 0;
}
