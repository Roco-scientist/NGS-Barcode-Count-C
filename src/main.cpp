#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include "CLI11.hpp"
// #include "argparse.h"
#include "info.h"
#include "input.h"
#include "parse.h"

using namespace std;

int main(int argc, char **argv) {
	chrono::steady_clock::time_point start = chrono::steady_clock::now();
	// argparse::Args args;
	// args.get_args(argc, *argv);
	// Get command line arguments
	string sample_barcodes_file = "../test_del/sample_barcode_file.csv";
	string counted_barcodes_file = "../test_del/test.bb_barcodes.csv";
	string format_file = "../test_del/test.scheme.txt";
	// string fastq_path = "../test_del/test.double.fastq";
	string fastq_path = "../test_del/test_10000000.fastq";
	int num_threads = 8;

	CLI::App app{"Counts barcodes located in sequencing data"};

	// std::string sample_barcodes_file = "default";
	// app.add_option("-s,--sample_barcodes", sample_barcodes_file,
	// 	       "Sample barcodes csv file");

	// std::string counted_barcodes_file = "default";
	// app.add_option("-c,--counted_barcodes", counted_barcodes_file,
	// 	       "Building block barcodes csv file");

	// std::string format_file;
	// app.add_option("-q,--sequence_format", format_file,
	// 	       "Sequence format file")->required();

	// std::string fastq_path;
	// app.add_option("-f,--fastq", fastq_path, "Fastq file
	// path")->required();

	// CLI11_PARSE(app, argc, argv);
	// app.parse(argc, argv);

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
	thread reader([&]() { input::read_fastq(&fastq_path, sequences); });

	info::Results results(&barcode_info.samples_seqs);
	vector<thread> parsers;
	for (int i = 1; i < num_threads; ++i) {
		parsers.push_back(thread([&]() {
			parse::SequenceParser a(sequences, results,
						barcode_info, sequence_format);
		}));
	}
	// wait for threads
	reader.join();
	for (int i = 0; i < parsers.size(); ++i) {
		parsers[i].join();
	}
	results.print_errors();
	chrono::steady_clock::time_point finish = chrono::steady_clock::now();
	string milliseconds = to_string(chrono::duration_cast<chrono::milliseconds>(finish - start).count() % 1000);
	while (milliseconds.size() < 3) {
		string millisecondes_temp = "0";
		millisecondes_temp.append(milliseconds);
		milliseconds = millisecondes_temp;
	}
	int total_seconds_passed = chrono::duration_cast<chrono::seconds>(finish - start).count();
	int seconds = total_seconds_passed % 60;
	int minutes = (total_seconds_passed % 3600) / 60;
	int hours = total_seconds_passed / 3600;
	cout << "Total time: ";
	if (hours > 0) {
		cout << hours << " hours ";
	}
	if (minutes > 0) {
		cout << minutes << " minutes ";
	}
	cout << seconds << "." << milliseconds << " seconds" << endl;
	return 0;
}
