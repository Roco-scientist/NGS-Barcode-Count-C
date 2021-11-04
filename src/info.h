#ifndef info
#define info
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef std::unordered_map<std::string, std::string> string_string_map;
typedef std::unordered_map<std::string, unsigned int> string_int_map;
typedef std::unordered_set<std::string> stringset;

/**
 * Funtion to pull the first <num_colums> data from each pass comma separated
 * string
 */
std::vector<std::string> take_columns(int num_columns, std::string row);

struct BarcodeConversion {
	// Holds the conversion of DNA barcode to sample name
	string_string_map samples_barcode_hash;
	// Holds all of the sample DNA sequences for error checking
	stringset samples_seqs;
	// Holds the conversion of counted barcode to name
	std::vector<string_string_map> counted_barcodes_hash;
	// Holds all counted barcode DNA seqeunces for error checking
	std::vector<stringset> counted_barcodes_seqs;

	/**
	 * Pulls in the given sample barcode file and pushes the data into an
	 * unodered_map. This map is later used to convert DNA barcode to sample
	 * name. Additionally, an unordered_set of sequences is created to error
	 * correct for any sequencing errors.
	 */
	void sample_barcode_conversion(std::string* barcode_path);

	/**
	 * Pulls in the given counted barcodes file and pushes the data into an
	 * unodered_map for each sequential barcode. These maps are later used
	 * to convert DNA barcode to sample name. Additionally, unordered_sets
	 * of the sequences are created to error correct for any sequencing
	 * errors.
	 */
	void barcode_file_conversion(std::string* barcode_path);

	/// Prints BarcodeConversion information for troubleshooting
	void print();
};

/**
 * A struct which pulls information from the sequence format file and creates a
 * regex for searching sequences reads. This regex finds each barcode.
 * Additional information is included within this struct to aid in error
 * handling etc.
 */
struct SequenceFormat {
	// A vector containing the barcode type in the order they are found in
	// the format file.  This is used to zip iterate with the regex captures
	// found
	std::vector<std::string> barcodes;
	// Whether or not a random barcode is included.  Not yet integrated
	bool random_barcode;
	// The regex with captures which finds all of the DNA barcodes in the
	// sequencing read
	std::regex format_regex;
	// A string where the captures are replaced with N's. Used for error
	// handling
	std::string format_string;
	// The number of counted barcodes. DEL is typically 2-4
	unsigned int barcode_num;
	// The length of the format string
	size_t length;
	// The length of the format string withouth N's used for % error
	size_t constant_size = 0;

	/// Builds a regex with captures for DNA barcodes from the format file
	void build_regex(std::string* format_path);
	/// A print method created to test building of the algorithm
	void print();
};

typedef std::unordered_map<std::string, string_int_map> string_string_int_map;

/**
 * Object which is passed between threads and holds the results which are added
 * to for each barcode count.
 */
class Results {
       public:
	// The unordered map of results which holds
	// Sample_barcode:counted_barcodes:count
	string_string_int_map results;

	Results(std::unordered_set<std::string>* sample_seqs) {
		new_results(sample_seqs);
	};
	Results();

	/// Creates a new Results object with a results map that contains all
	/// sample DNA barcodes
	void new_results(std::unordered_set<std::string>* sample_seqs);
	/// With the given sample barcode and counted barcodes, adds 1 with a
	/// locked mutex
	void add_count(std::string sample_barcode,
		       std::string counted_barcodes);
	/// The following three methods add one to each counted error
	void add_constant_error();
	void add_sample_barcode_error();
	void add_counted_barcode_error();
	/// Prints for troubleshooting
	void print();
	/// Prints the error results to cout
	void print_errors();
	/// Writes the results to csv files in the format
	/// YYYY-MM-DD_sample_name.csv
	void to_csv(bool merge, BarcodeConversion barcode_conversion,
		    std::string outpath, int barcode_num);

       private:
	// Four different mutexes to be locked for either counting or keeping
	// track of errors
	std::mutex mtx;
	std::mutex mtx_const;
	std::mutex mtx_samp;
	std::mutex mtx_count;
	// The errors which are counted and kept track
	unsigned int correct_counts = 0;
	unsigned int constant_errors = 0;
	unsigned int sample_barcode_errors = 0;
	unsigned int counted_barcode_errors = 0;
};

/// Creates the current data in the format of YYYY-MM-DD
std::string current_date();

#endif
