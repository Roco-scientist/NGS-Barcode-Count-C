#ifndef parse
#define parse

#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <regex>
#include <vector>

#include "info.h"
#include "input.h"

class SequenceParser {
       public:
	SequenceParser(input::Sequences& sequences, info::Results& results,
		       info::BarcodeConversion _barcode_conversion,
		       info::SequenceFormat _sequence_format)
	    : sequences(sequences), results(results) {
		barcode_conversion = _barcode_conversion;
		sequence_format = _sequence_format;
		run();
	};
	SequenceParser();

       private:
	input::Sequences& sequences;
	info::Results& results;
	info::BarcodeConversion barcode_conversion;
	info::SequenceFormat sequence_format;
	std::string sequence;

	std::string fix_sequence(std::string& query_sequence,
				 stringset& subject_sequences,
				 size_t max_errors);
	void fix_constant();
	void add_count(std::smatch& barcode_match);
	void get_barcodes();
	void run();
};

#endif
