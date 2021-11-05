#ifndef parser
#define parser

#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <regex>
#include <vector>

#include "info.h"
#include "input.h"

/**
 * Object which parses the sequencing reads.  Can be spun out to multiple
 * threads.  Takes in sequencing reads from the reader thread through the
 * input::Sequences object.  The info::Results object is used between all
 * SequenceParser threads.
 */
class SequenceParser {
       public:
	SequenceParser(input::Sequences& sequences, info::Results& results,
		       info::BarcodeConversion _barcode_conversion,
		       info::SequenceFormat _sequence_format)
	    : sequences(sequences), results(results) {
		barcode_conversion = _barcode_conversion;
		sequence_format = _sequence_format;
		// Start parsing after construction
		run();
	};
	SequenceParser();

       private:
	// Object which the reader thread deposits sequences
	input::Sequences& sequences;
	// Object to add to and keep track of results
	info::Results& results;
	// Conversion of DNA barcodes to names. Used for sequencing error
	// handling
	info::BarcodeConversion barcode_conversion;
	// Format information which contains the regex used to capture DNA
	// barcodes
	info::SequenceFormat sequence_format;
	// The current sequence which is being parsed.  Passed between methods
	std::string sequence;

	/// Method to fix sequencing errors of the query_sequence given the
	/// known possible subject_sequences
	std::string fix_sequence(std::string& query_sequence,
				 stringset& subject_sequences,
				 size_t max_errors);
	/// Special method for fixing the constant region of the seqquencing
	/// read if the regex was not able to find a match.  If found, returns
	/// true
	bool fix_constant();
	/// Add count to the Results object as long as sequencing errors can be
	/// fixed
	void add_count(std::smatch& barcode_match);
	/// Use the regex to find the DNA barcodes within the sequencing read
	void get_barcodes();
	/// Run parsing which goes run -> get_barcodes -> add_count (try to fix
	/// on sequencing error -> add)
	void run();
};

#endif
