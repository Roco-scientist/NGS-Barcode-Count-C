#ifndef info
#define info
#include <chrono>
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
typedef std::unordered_map<std::string, std::unordered_set<std::string>>
    string_stringset_map;
typedef std::unordered_map<std::string, string_stringset_map>
    string_string_stringset_map;
typedef std::unordered_map<std::string, unsigned int> string_int_map;
typedef std::unordered_set<std::string> stringset;

/// Separates a comma separated string into comma separated vector of strings
std::vector<std::string> comma_separate(std::string comma_string);

/**
 * Funtion to pull the first <num_colums> data from each pass comma separated
 * string
 */
std::vector<std::string> take_columns(size_t num_columns, std::string row);

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
	bool random_barcode_included = false;
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
	size_t constant_region_size = 0;
	size_t sample_barcode_size = 0;
	size_t avg_counted_barcode_size = 0;

	/// Builds a regex with captures for DNA barcodes from the format file
	void build_regex(std::string* format_path);
	/// A print method created to test building of the algorithm
	void print();
};

typedef std::unordered_map<std::string, string_int_map> string_string_int_map;

/// Whether or not the dataset is single, double, or full synthon enrichment
enum EnrichedType { Single, Double, Full };

/**
 * Object which is passed between threads and holds the results which are added
 * to for each barcode count.
 */
class Results {
       public:
	// The unordered map of results which holds
	// Sample_barcode:counted_barcodes:count
	string_string_int_map results;

	// Same as results for when there is a random barcode included.  Holds
	// Sample_barcode:counted_barcodes:random_barcodes
	string_string_stringset_map results_random;

	Results(std::unordered_set<std::string>* sample_seqs, bool _enrich) {
		new_results(sample_seqs);
		enrich = _enrich;
	};
	Results();

	/// Creates a new Results object with a results map that contains all
	/// sample DNA barcodes
	void new_results(std::unordered_set<std::string>* sample_seqs);
	/// With the given sample barcode and counted barcodes, adds 1 with a
	/// locked mutex
	void add_count(std::string& sample_barcode,
		       std::string& counted_barcodes,
		       std::string& random_barcode);
	/// The following three methods add one to each counted error
	void add_constant_error();
	void add_sample_barcode_error();
	void add_counted_barcode_error();
	void add_duplicate();
	/// Prints for troubleshooting
	void print();
	/// Prints the error results to cout
	void print_errors();
	/// Writes the results to csv files in the format
	/// YYYY-MM-DD_sample_name.csv
	void to_csv(bool merge, BarcodeConversion barcode_conversion,
		    std::string outpath, int barcode_num,
		    bool random_barcode_included);

       private:
	// Four different mutexes to be locked for either counting or keeping
	// track of errors
	std::mutex mtx;
	std::mutex mtx_const;
	std::mutex mtx_samp;
	std::mutex mtx_count;
	std::mutex mtx_dup;
	// The errors which are counted and kept track
	unsigned int correct_counts = 0;
	unsigned int constant_errors = 0;
	unsigned int sample_barcode_errors = 0;
	unsigned int counted_barcode_errors = 0;
	unsigned int duplicates = 0;

	// Used to insert into results_random
	std::unordered_set<std::string> empty_set;

	// set used for what finished barcodes have already been counted for the
	// merge file
	std::unordered_set<std::string> finished_barcodes;
	// Vector of sample names used for ordering sample_barcodes
	std::vector<std::string> sample_names;
	// Ordered with sample_names to write in order
	std::vector<std::string> sample_barcodes;
	// Contains DNA barcode conversion information to convert to names while
	// writing
	BarcodeConversion barcode_conversion;

	// Enrichment variables used when the --enrich flag is called.
	// Calculates the single and double synthon/barcode counts
	bool enrich;  // whether or not calc and output enricment
	string_string_int_map single_enrichment;  // single data holder
	string_string_int_map double_enrichment;  // double data holder
	// Adds the enrichment data when given the sample DNA barcode and the
	// converted counted barcodes
	void add_enriched(std::string& sample_seq,
			  std::string& converted_barcodes, int count);

	// A string used for when there are no samples barcodes included. Placed
	// here to prevent repeated allocation
	std::string no_sample_barcode = "barcode_counts";

	// File header strings used for each of the csv files
	std::string sample_file_header;
	std::string merge_file_header;
	// Output text
	std::string sample_text;
	std::string merge_text;

	/// A submethod used with to csv to write when there isn't a random
	/// barcode included.  Between random barcode and not different
	/// unordered_maps are used.  When random barcode is not included, the
	/// last value holds a int count.  When a random barcode is included,
	/// the last value holds a set of the random barcodes, which can be
	/// counted to find the count
	void write_counts(int index, std::vector<int>& indices, bool merge,
			  EnrichedType enriched_type);
	void write_random(int index, std::vector<int>& indices, bool merge);
	/// If the enrich argument flag is called, the single and
	/// disynthon/barcode enrichment are written to csv
	void write_enriched(bool merge, std::string outpath, int barcode_num);
};

/**
 * Class which holds how many sequencing errors are allowed within each regions.
 * If the max-error arguments are not used, these numbers default to 20% of the
 * length of the barcode/region. This is passed to all parsing threads.
 */
class MaxSeqErrors {
       public:
	// Maximum sequencing areas allowed per sequencing read within the given
	// areas
	int constant_region = 0;  // Any region that doesn't have a barcode
	int sample_barcode = 0;	  // Given sample barcode
	int counted_barcode = 0;  // Each counted barcode, which can be multiple

	// Creates a new MaxSeqErrors object with the argument inputs and with
	// the SequenceFormat object, which contains each region length
	void update(int constant_errors, int sample_errors, int barcode_errors,
		    SequenceFormat sequence_format);

	// Displays region size and errors allowed to cout.
	void print();

       private:
	// Holders of each region size to print out after object creation
	int constant_region_size = 0;
	int sample_barcode_size = 0;
	int counted_barcode_size = 0;
};

/// Creates the current data in the format of YYYY-MM-DD
std::string current_date();

// Returns a string of the hours, minutes, and seconds passed since start
std::string time_passed(std::chrono::steady_clock::time_point start);

#endif
