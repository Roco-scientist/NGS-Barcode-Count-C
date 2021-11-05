#include "parser.h"

using namespace std;

string SequenceParser::fix_sequence(string& query_sequence,
				    stringset& subject_sequences,
				    size_t max_errors) {
	size_t best_mismatches = max_errors + 1;
	string none = "None";
	const string* best_match = &none;
	unsigned int mismatches;
	for (auto &subject_sequence : subject_sequences) {
		if (query_sequence.size() < subject_sequence.size()) {
			cout << "fix_sequence error: query_sequence "
			     << query_sequence
			     << " smaller than subject_sequence "
			     << subject_sequence << endl;
			exit(1);
		}
		mismatches = 0;
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
			best_match = &none;
		}
		if (mismatches < best_mismatches) {
			best_mismatches = mismatches;
			best_match = &subject_sequence;
		}
	}
	return *best_match;
}

void SequenceParser::fix_constant() {
	stringset subject_sequences;
	size_t length_diff = sequence.size() - sequence_format.length;
	for (size_t i = 0; i < length_diff; ++i) {
		subject_sequences.insert(sequence.substr(i, sequence_format.length));
	}
	string best_match =
	    fix_sequence(sequence_format.format_string, subject_sequences,
			 sequence_format.constant_size / 5);
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

void SequenceParser::add_count(smatch& barcode_match) {
	string sample_barcode;
	string counted_barcodes;
	unsigned int counted_barcode_index = 0;
	for (size_t i = 1; i < barcode_match.size(); ++i) {
		if (sequence_format.barcodes[i - 1] == "sample") {
			sample_barcode = barcode_match[i].str();
		} else if (sequence_format.barcodes[i - 1] ==
			   "counted_barcode") {
			string counted_barcode = barcode_match[i].str();
			if (barcode_conversion
				.counted_barcodes_seqs[counted_barcode_index]
				.count(counted_barcode) == 0) {
				counted_barcode = fix_sequence(
				    counted_barcode,
				    barcode_conversion.counted_barcodes_seqs
					[counted_barcode_index],
				    counted_barcode.size() / 5);
				if (counted_barcode == "None") {
					results.add_counted_barcode_error();
					return;
				}
			}
			if (counted_barcode_index != 0) {
				counted_barcodes.push_back(',');
			}
			counted_barcodes.append(counted_barcode);
			++counted_barcode_index;
		}
	}
	if (barcode_conversion.samples_seqs.count(sample_barcode) == 0) {
		sample_barcode = fix_sequence(sample_barcode,
					      barcode_conversion.samples_seqs,
					      sample_barcode.size() / 5);
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

