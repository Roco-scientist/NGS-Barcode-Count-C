#ifndef info
#define info
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

typedef std::unordered_map<std::string, std::string> stringmap;
typedef std::unordered_set<std::string> stringset;

struct BarcodeConversion {
	stringmap samples_barcode_hash;
	stringset samples_seqs;
	std::vector<stringmap> counted_barcodes_hash;
	std::vector<stringset> counted_barcodes_seqs;

	void sample_barcode_conversion(std::string* barcode_path);
	void barcode_file_conversion(std::string* barcode_path);
	void print();
};

struct SequenceFormat {
	std::vector<std::string> barcodes;
	bool random_barcode;
	std::regex format_regex;
	std::string format_string;
	unsigned int barcode_num;
	unsigned int length;

	void build_regex(std::string* format_path);
	void print();
};

#endif
