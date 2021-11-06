#include <pthread.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "CLI11.hpp"
#include "info.h"
#include "input.h"
#include "parser.h"

using namespace std;

int main(int argc, char** argv) {
	chrono::steady_clock::time_point start = chrono::steady_clock::now();

	// Get command line arguments
	CLI::App app{
	    "Barcode-Count 0.2.1\nRory Coffey <coffeyrt@gmail.com>\nCounts "
	    "barcodes located in sequencing data\n"};
	app.set_version_flag("-v,--version", "0.2.1");

	std::string sample_barcodes_file;
	app.add_option("-s,--sample_barcodes", sample_barcodes_file,
		       "Sample barcodes csv file")
	    ->check(CLI::ExistingFile);

	std::string counted_barcodes_file;
	app.add_option("-c,--counted_barcodes", counted_barcodes_file,
		       "Building block barcodes csv file")
	    ->check(CLI::ExistingFile);

	std::string format_file;
	app.add_option("-q,--sequence_format", format_file,
		       "Sequence format file")
	    ->required()
	    ->check(CLI::ExistingFile);

	std::string fastq_path;
	app.add_option("-f,--fastq", fastq_path, "Fastq file path")
	    ->required()
	    ->check(CLI::ExistingFile);

	int num_threads;
	app.add_option("-t,--threads", num_threads, "Number of CPU threads")
	    ->default_val(thread::hardware_concurrency());

	bool merge;
	app.add_flag("-m,--merge_output", merge, "Merge output file");

	string outpath;
	app.add_option("-o,--output_dir", outpath, "Output directory")
	    ->default_str("./")
	    ->check(CLI::ExistingPath);

	CLI11_PARSE(app, argc, argv);

	if (outpath.back() != '/') {
		outpath.push_back('/');
	}

	// Get all DNA barcode conversion data
	info::BarcodeConversion barcode_info;
	barcode_info.sample_barcode_conversion(&sample_barcodes_file);
	barcode_info.barcode_file_conversion(&counted_barcodes_file);
	// barcode_info.print();

	// Convert format file
	info::SequenceFormat sequence_format;
	sequence_format.build_regex(&format_file);
	sequence_format.print();

	input::Sequences sequences;
	thread reader([&]() { input::read_fastq(&fastq_path, sequences); });

	info::Results results(&barcode_info.samples_seqs);
	vector<thread> parsers;
	for (int i = 1; i < num_threads; ++i) {
		parsers.push_back(thread([&]() {
			parser::SequenceParser a(sequences, results,
						 barcode_info, sequence_format);
		}));
	}
	// wait for threads
	reader.join();
	for (size_t i = 0; i < parsers.size(); ++i) {
		parsers[i].join();
	}
	results.print_errors();
	cout << "\nParsing time: " << info::time_passed(start) << endl;
	results.to_csv(merge, barcode_info, outpath,
		       sequence_format.barcode_num,
		       sequence_format.random_barcode_included);

	cout << "\nTotal time: " << info::time_passed(start);
	return 0;
}
