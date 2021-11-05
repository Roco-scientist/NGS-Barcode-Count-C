#include "parser.h"

using namespace std;

string SequenceParser::fix_sequence(string& query_sequence,
				    stringset& subject_sequences,
				    size_t max_errors) {
	// Allow for a better match at max errors
	size_t best_mismatches = max_errors + 1;
	string none = "None";  // Create a None string to point to
	// Creating a pointer for best match so that a string isn't continually
	// reallocated
	const string* best_match = &none;
	// Initiate mismatches so it isn't continually initiated
	unsigned int mismatches;
	for (auto& subject_sequence : subject_sequences) {
		if (query_sequence.size() < subject_sequence.size()) {
			cout << "fix_sequence error: query_sequence "
			     << query_sequence
			     << " smaller than subject_sequence "
			     << subject_sequence << endl;
			exit(1);
		}
		mismatches = 0;
		// Iterate through characters from the query sequence and the
		// subject sequence to get the number of mismatches
		for (size_t j = 0; j < query_sequence.size(); ++j) {
			if ((query_sequence[j] != 'N' &&
			     subject_sequence[j] != 'N') &&
			    query_sequence[j] != subject_sequence[j]) {
				++mismatches;
			}
			// If mismatches are more than the best so far, break
			// early
			if (mismatches > best_mismatches) {
				break;
			}
		}
		// If two sequences are the best match, then there are no best
		// matches so far
		if (mismatches == best_mismatches) {
			best_match = &none;
		}
		// If there are fewer mismatches, a pointer to the current
		// subject sequence is set as best match
		if (mismatches < best_mismatches) {
			best_mismatches = mismatches;
			best_match = &subject_sequence;
		}
	}
	return *best_match;
}

void SequenceParser::fix_constant() {
	stringset subject_sequences;
	// Get the length differene between the sequencing read and the sequence
	// format in order to stop iterations before running out of the sequence
	// read
	size_t length_diff = sequence.size() - sequence_format.length;
	// Create stringset with every sequence read offset by one with the
	// length of the sequence format.  This is to have all possible subject
	// sequences to query from the seqeunce read
	for (size_t i = 0; i < length_diff; ++i) {
		subject_sequences.insert(
		    sequence.substr(i, sequence_format.length));
	}
	// Find the best match between the constant region (format_string) and
	// all possible sequences in the sequencing read
	string best_match =
	    fix_sequence(sequence_format.format_string, subject_sequences,
			 sequence_format.constant_size / 5);
	// If a match is found, proceed and replace the N's in the format string
	// with the barcodes from the sequencing read
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
	} else {
		sequence = best_match;
	}
}

void SequenceParser::add_count(smatch& barcode_match) {
	string sample_barcode;
	// A comma separated string to hold all of the counted barcodes.  This
	// is later used as the key for a unordered_map in order to count
	// occurances
	string counted_barcodes;
	// Start the barcode indexes and add every time a counted barcode is
	// found
	unsigned int counted_barcode_index = 0;
	// Iterate through all regex captured DNA barcodes
	for (size_t i = 1; i < barcode_match.size(); ++i) {
		// If the captured barcoe is "sample" set that varaible. Here
		// the sequence_format.barcodes holds the barcode names in the
		// same order as the captures. Since the first regex match is
		// the full string, then the barcode captures start, 1 needs to
		// be subtracted
		if (sequence_format.barcodes[i - 1] == "sample") {
			sample_barcode = barcode_match[i].str();
		} else if (sequence_format.barcodes[i - 1] ==
			   "counted_barcode") {
			// If it is a counted barcode, check/fix for sequence
			// errors, then push this to the counted barcodes string
			string counted_barcode = barcode_match[i].str();
			if (barcode_conversion
				.counted_barcodes_seqs[counted_barcode_index]
				.count(counted_barcode) == 0) {
				counted_barcode = fix_sequence(
				    counted_barcode,
				    barcode_conversion.counted_barcodes_seqs
					[counted_barcode_index],
				    counted_barcode.size() / 5);
				// If after trying to fix there was not a good
				// match, count the error
				if (counted_barcode == "None") {
					results.add_counted_barcode_error();
					return;
				}
			}
			// Push the counted barcode to the back of the counted
			// barcode string
			if (counted_barcode_index != 0) {
				counted_barcodes.push_back(',');
			}
			counted_barcodes.append(counted_barcode);
			++counted_barcode_index;
		}
	}
	// If sample barcode is not within known sample barcodes, try to fix
	if (barcode_conversion.samples_seqs.count(sample_barcode) == 0) {
		sample_barcode = fix_sequence(sample_barcode,
					      barcode_conversion.samples_seqs,
					      sample_barcode.size() / 5);
	}
	// If the sample barcode could be fixed or is good, and all other
	// barcodes are good (checked previously), add the count, otherwise
	// record the sample barcode error
	if (sample_barcode != "None") {
		results.add_count(sample_barcode, counted_barcodes);
	} else {
		results.add_sample_barcode_error();
	}
}

void SequenceParser::get_barcodes() {
	smatch barcode_match;
	// If constant region matches regex add the count, otherwise fix the
	// constant region
	if (regex_search(sequence, barcode_match,
			 sequence_format.format_regex)) {
		add_count(barcode_match);
	} else {
		fix_constant();
		if (sequence != "None") {
			if (regex_search(sequence, barcode_match,
					 sequence_format.format_regex)) {
				add_count(barcode_match);
			}
		} else {
			// Record an error if the constnat region could not be
			// fixed
			results.add_constant_error();
		}
	}
}

void SequenceParser::run() {
	int read = 0;
	// After the reader thread finishes the fastq file, it inserts
	// "finished" to tell the parser threads to stop.  This will be after
	// all sequences are parsed
	while (sequence != "finished") {
		++read;
		sequence = sequences.retrieve();
		if (sequence == "finished") {
			break;
		}
		// Empty is returned when the queue is empty but the reader
		// thread is not finished.  If this thread got a sequence
		// proceed
		if (sequence != "empty") {
			get_barcodes();
		}
	}
}

