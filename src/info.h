#ifndef info
#define info
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <mutex>
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
	string_string_map samples_barcode_hash;
	stringset samples_seqs;
	std::vector<string_string_map> counted_barcodes_hash;
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
	void print();
};

struct SequenceFormat {
	std::vector<std::string> barcodes;
	bool random_barcode;
	std::regex format_regex;
	std::string format_string;
	unsigned int barcode_num;
	size_t length;
	size_t constant_size = 0;

	void build_regex(std::string* format_path);
	/// A print method created to test building of the algorithm
	void print();
};

typedef std::unordered_map<std::string, string_int_map> string_string_int_map;

class Results {
       public:
	string_string_int_map results;

	Results(std::unordered_set<std::string>* sample_seqs) {
		new_results(sample_seqs);
	};
	Results();
	void new_results(std::unordered_set<std::string>* sample_seqs);
	void add_count(std::string sample_barcode, std::vector<std::string> counted_barcodes);
	void add_constant_error();
	void add_sample_barcode_error();
	void add_counted_barcode_error();
	void print();
	void print_errors();

       private:
	std::mutex mtx;
	std::mutex mtx_const;
	std::mutex mtx_samp;
	std::mutex mtx_count;
	unsigned int correct_counts = 0;
	unsigned int constant_errors = 0;
	unsigned int sample_barcode_errors = 0;
	unsigned int counted_barcode_errors = 0;
};

#endif
