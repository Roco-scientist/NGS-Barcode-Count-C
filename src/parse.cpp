#include "parse.h"

using namespace std;

string SequenceParser::fix_sequence(string query_sequence,
				    vector<string> subject_sequences) {
	size_t best_mismatches = query_sequence.size() / 5;
	string best_match = "None";
	for (size_t i = 0; i < subject_sequences.size(); ++i) {
		if (query_sequence.size() < subject_sequences[i].size()) {
			cout << "fix_sequence error: query_sequence "
			     << query_sequence
			     << " smaller than subject_sequence "
			     << subject_sequences[i] << endl;
			exit(1);
		}
		unsigned int mismatches = 0;
		for (size_t j = 0; j < query_sequence.size(); ++j) {
			if ((query_sequence[j] != 'N' &&
			     subject_sequences[i][j] != 'N') &&
			    query_sequence[j] != subject_sequences[i][j]) {
				++mismatches;
			}
			if (mismatches > best_mismatches) {
				break;
			}
		}
		if (mismatches == best_mismatches) {
			best_match = "None";
		}
		if (mismatches < best_mismatches) {
			best_mismatches = mismatches;
			best_match = subject_sequences[i];
		}
	}
	return best_match;
}

void SequenceParser::fix_constant() {
	vector<string> subject_sequences;
	size_t length_diff = sequence.size() - sequence_format.length;
	for (size_t i = 0; i < length_diff; ++i) {
		string subject_seq =
		    sequence.substr(i, sequence_format.length);
		subject_sequences.push_back(subject_seq);
	}
	string best_match =
	    fix_sequence(sequence_format.format_string, subject_sequences);
	if (best_match != "None") {
		string fixed_sequence;
		for (size_t i = 0; i < best_match.size(); ++i) {
			if (sequence_format.format_string[i] == 'N') {
				fixed_sequence.push_back(best_match[i]);
			} else {
				fixed_sequence.push_back(
				    sequence_format.format_string[i]);
			}
		}
		sequence = fixed_sequence;
	}
}

void SequenceParser::get_barcodes() {
	smatch barcode_match;
	if (regex_search(sequence, barcode_match,
			 sequence_format.format_regex)) {
		// for (size_t i = 1; i < barcode_match.size(); ++i) {
		// 	cout << barcode_match[i].str() << '\t';
		// }
		// cout << endl;
	} else {
		fix_constant();
		if (regex_search(sequence, barcode_match,
				 sequence_format.format_regex)) {
			// cout << "fixed\t";
			// for (size_t i = 1; i < barcode_match.size(); ++i) {
			// 	cout << barcode_match[i].str() << '\t';
			// }
			// cout << endl;
		} else {
			// cout << "No match" << endl;
			return;
		}
	}
}

void SequenceParser::run() {
	sequence = sequences.retrieve();
	while (sequence != "finished") {
		sequence = sequences.retrieve();
		if (sequence == "finished") {
			break;
		}
		if (sequence != "empty") {
			get_barcodes();
		} else {
			cout << "empty" << endl;
		}
	}
}

