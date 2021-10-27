#include "info.h"

using namespace std;

vector<string> take_columns(int num_columns, string row) {
	int pos;
	string token;
	vector<string> data_points;
	int current_column = 1;
	while ((pos = row.find(',')) != string::npos &&
	       current_column <= num_columns) {
		token = row.substr(0, pos);
		data_points.push_back(token);
		row.erase(0, pos + 1);
		current_column++;
	}
	if (current_column == num_columns) {
		data_points.push_back(row);
	} else if (current_column < num_columns) {
		cout << "Not enough columns" << endl;
		exit(1);
	}
	return data_points;
}

void BarcodeConversion::sample_barcode_conversion(const string barcode_path) {
	ifstream barcode_file;
	barcode_file.open(barcode_path);
	if (!barcode_file.is_open()) {
		cout << barcode_path << " not found" << endl;
		exit(1);
	}

	string row;
	std::getline(barcode_file, row);  // The first row should be the header
	for (string row; std::getline(barcode_file, row);) {
		vector<string> row_vec = take_columns(2, row);
		samples_barcode_hash.insert(
		    std::pair<string, string>(row_vec[0], row_vec[1]));
	}
}

void BarcodeConversion::barcode_file_conversion(const string barcode_path) {
	ifstream barcode_file;
	barcode_file.open(barcode_path);
	if (!barcode_file.is_open()) {
		cout << barcode_path << " not found" << endl;
		exit(1);
	}

	unordered_set<int> barcode_num;
	vector<vector<string>> row_info;

	string row;
	std::getline(barcode_file, row);  // The first row should be the header
	for (string row; std::getline(barcode_file, row);) {
		vector<string> row_vec = take_columns(3, row);
		row_info.push_back(row_vec);
		barcode_num.insert(std::stoi(row_vec[2]));
	}

	int total_barcodes;
	for (const auto &num : barcode_num) {
		if (num > total_barcodes) {
			total_barcodes = num;
		}
	}

	unordered_map<string, string> empty_hashmap;
	for (int i = 0; i < total_barcodes; i++) {
		counted_barcodes_hash.push_back(empty_hashmap);
	}

	for (int i = 0; i < row_info.size(); i++) {
		vector<string> row_vec = row_info[i];
		int barcode_num = std::stoi(row_vec[2]) - 1;
		pair<string, string> barcode_conv{row_vec[0], row_vec[1]};
		counted_barcodes_hash[barcode_num].insert(barcode_conv);
	}
}

void BarcodeConversion::print() {
	cout << "Sample barcode conversion" << endl;
	for (const auto &myPair : samples_barcode_hash) {
		std::cout << "Key:[" << myPair.first << "] Value:["
			  << myPair.second << "]\n";
	}
	cout << endl;

	cout << "Counted barcode conversion" << endl;
	for (int i = 0; i < counted_barcodes_hash.size(); i++) {
		cout << endl << "Barcode number" << i + 1 << endl << endl;
		stringmap barcode_hash = counted_barcodes_hash[i];
		for (const auto &myPair : barcode_hash) {
			std::cout << "Key:[" << myPair.first << "] Value:["
				  << myPair.second << "]\n";
		}
	}
}
