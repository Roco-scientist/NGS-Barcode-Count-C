#ifndef info
#define info
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

typedef std::unordered_map<std::string, std::string> stringmap;
typedef std::unordered_set<std::string> stringset;

struct BarcodeConversion {
	stringmap samples_barcode_hash;
	stringset samples_seqs;
	std::vector<stringmap> counted_barcodes_hash;
	std::vector<stringset> counted_barcodes_seqs;

	void sample_barcode_conversion(const std::string barcode_path);
	void barcode_file_conversion(const std::string barcode_path);
	void print();
};

struct SequenceFormat {
	std::string regex_string;
	bool random_barcode;
	std::regex format_regex;
	std::string format_string;
	std::string regions_string;
	int barcode_num;
	int length;
	void build_regex(const std::string format_path);
};

#endif
