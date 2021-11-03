#ifndef parse
#define parse

#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <vector>
#include <regex>

#include "info.h"
#include "input.h"

class SequenceParser {
       public:
	SequenceParser(input::Sequences& sequences,
		       info::BarcodeConversion _barcode_conversion,
		       info::SequenceFormat _sequence_format)
	    : sequences(sequences) {
		barcode_conversion = _barcode_conversion;
		sequence_format = _sequence_format;
		run();
	};
	SequenceParser();

       private:
	input::Sequences& sequences;
	info::BarcodeConversion barcode_conversion;
	info::SequenceFormat sequence_format;
	std::string sequence;

	std::string fix_sequence(std::string query_sequence, std::vector<std::string> subject_sequences);
	void fix_constant();
	void get_barcodes();
	void run();
};

#endif
