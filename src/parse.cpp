#include "parse.h"

using namespace std;

string SequenceParser::fix_sequence(string query_sequence,
				    stringset subject_sequences) {
	size_t best_mismatches = (query_sequence.size() / 5) + 1;
	string best_match = "None";
	for (auto const &subject_sequence : subject_sequences) {
		if (query_sequence.size() < subject_sequence.size()) {
			cout << "fix_sequence error: query_sequence "
			     << query_sequence
			     << " smaller than subject_sequence "
			     << subject_sequence << endl;
			exit(1);
		}
		unsigned int mismatches = 0;
		for (size_t j = 0; j < query_sequence.size(); ++j) {
			if ((query_sequence[j] != 'N' &&
			     subject_sequence[j] != 'N') &&
			    query_sequence[j] != subject_sequence[j]) {
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
			best_match = subject_sequence;
		}
	}
	return best_match;
}

void SequenceParser::fix_constant() {
	stringset subject_sequences;
	size_t length_diff = sequence.size() - sequence_format.length;
	for (size_t i = 0; i < length_diff; ++i) {
		string subject_seq = sequence.substr(i, sequence_format.length);
		subject_sequences.insert(subject_seq);
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

void SequenceParser::add_count(smatch barcode_match) {
	string sample_barcode;
	vector<string> counted_barcodes;
	for (size_t i = 1; i < barcode_match.size(); ++i) {
		if (sequence_format.barcodes[i - 1] == "sample") {
			sample_barcode = barcode_match[i].str();
		} else if (sequence_format.barcodes[i - 1] ==
			   "counted_barcode") {
			counted_barcodes.push_back(barcode_match[i].str());
		}
	}
	if (barcode_conversion.samples_seqs.count(sample_barcode) == 0) {
		sample_barcode = fix_sequence(sample_barcode,
					      barcode_conversion.samples_seqs);
	}
	if (sample_barcode != "None") {
		results.add_count(sample_barcode, counted_barcodes);
	} else {
		results.add_sample_barcode_error();
	}
}

void SequenceParser::get_barcodes() {
	smatch barcode_match;
	if (regex_search(sequence, barcode_match,
			 sequence_format.format_regex)) {
		add_count(barcode_match);
	} else {
		fix_constant();
		if (regex_search(sequence, barcode_match,
				 sequence_format.format_regex)) {
			add_count(barcode_match);
		} else {
			results.add_constant_error();
		}
	}
}

void SequenceParser::run() {
	int read = 0;
	while (sequence != "finished") {
		++read;
		sequence = sequences.retrieve();
		if (sequence == "finished") {
			break;
		}
		if (sequence != "empty") {
			get_barcodes();
		}
	}
}

