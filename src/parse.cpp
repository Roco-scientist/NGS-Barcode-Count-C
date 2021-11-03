#include "parse.h"

using namespace std;

void SequenceParser::get_barcodes() {
	smatch barcode_match;
	if (regex_search(sequence, barcode_match,
			 sequence_format.format_regex)) {
		for (size_t i = 1; i < barcode_match.size(); ++i) {
			cout << barcode_match[i].str() << '\t';
		}
		cout << endl;
	} else {
		cout << "No match" << endl;
	}
}

void SequenceParser::run() {
	sequence = sequences.retrieve();
	while (sequence != "finished") {
		sequence = sequences.retrieve();
		if (sequence != "empty") {
			get_barcodes();
		} else {
			cout << "empty" << endl;
		}
	}
}

