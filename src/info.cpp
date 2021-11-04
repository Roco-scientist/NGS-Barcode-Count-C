#include "info.h"

using namespace std;

vector<string> take_columns(int num_columns, string row) {
	int pos;		     // keep track of the column position
	string data;		     // column,row data holder
	vector<string> data_points;  // vector to contain all data points
	int current_column = 1;	     // start at column 1
	// iterate through each comma point and pull out the data.  Stop when
	// there are no more commas or the num_columns is reached
	while ((pos = row.find(',')) != string::npos &&
	       current_column <= num_columns) {
		data = row.substr(0, pos);
		data_points.push_back(data);
		// remove the data from the string up to the comma
		row.erase(0, pos + 1);
		current_column++;
	}
	// If the the last column was the num_columns then push the last string
	// data of the row
	if (current_column == num_columns) {
		data_points.push_back(row);
	} else if (current_column < num_columns) {  // If there are not enough
						    // columns, exit with error
		cout << "Not enough columns" << endl;
		exit(1);
	}
	return data_points;
}

void BarcodeConversion::sample_barcode_conversion(string *barcode_path) {
	if (*barcode_path == "default") {
		return;
	}
	ifstream barcode_file;
	barcode_file.open(*barcode_path);
	if (!barcode_file.is_open()) {
		cout << *barcode_path << " not found" << endl;
		exit(1);
	}

	string row;
	getline(barcode_file, row);  // The first row should be the header
	for (string row; getline(barcode_file, row);) {
		vector<string> row_vec = take_columns(2, row);
		samples_barcode_hash.insert(
		    pair<string, string>(row_vec[0], row_vec[1]));
		samples_seqs.insert(row_vec[0]);
	}
}

void BarcodeConversion::barcode_file_conversion(string *barcode_path) {
	if (*barcode_path == "default") {
		return;
	}
	// Open the file and handle not found errors
	ifstream barcode_file;
	barcode_file.open(*barcode_path);
	if (!barcode_file.is_open()) {
		cout << *barcode_path << " not found" << endl;
		exit(1);
	}

	// keep track of number of barcodes, used later
	unordered_set<int> barcode_num;
	// hold all comma separated data to insert into hashmaps
	vector<vector<string>> row_info;

	string row;
	getline(barcode_file, row);  // The first row should be the header
	// Collect all comma separated rows and keep track of how many barcodes
	for (string row; getline(barcode_file, row);) {
		vector<string> row_vec = take_columns(3, row);
		row_info.push_back(row_vec);
		barcode_num.insert(stoi(row_vec[2]));
	}

	// Get total number of counted barcodes per sequence. ie for DEL this
	// can be 3 or 4
	int total_barcodes;
	for (const auto &num : barcode_num) {
		if (num > total_barcodes) {
			total_barcodes = num;
		}
	}

	// For each counted barcode in a sequence, create empty hashmaps and
	// hashsets
	string_string_map empty_hashmap;
	stringset empty_hashset;
	for (int i = 0; i < total_barcodes; i++) {
		counted_barcodes_hash.push_back(empty_hashmap);
		counted_barcodes_seqs.push_back(empty_hashset);
	}

	// Insert info into vectors of hashmaps and hashsets while using the
	// index as the counted barcode number
	for (int i = 0; i < row_info.size(); i++) {
		vector<string> row_vec = row_info[i];
		int barcode_num = stoi(row_vec[2]) - 1;
		pair<string, string> barcode_conv{row_vec[0], row_vec[1]};
		counted_barcodes_hash[barcode_num].insert(barcode_conv);
		counted_barcodes_seqs[barcode_num].insert(row_vec[0]);
	}
}

void BarcodeConversion::print() {
	cout << "Sample barcode conversion" << endl;
	for (const auto &[key, value] : samples_barcode_hash) {
		cout << "Key:[" << key << "] Value:[" << value << "]\n";
	}
	cout << endl;

	cout << "Counted barcode conversion" << endl;
	for (int i = 0; i < counted_barcodes_hash.size(); i++) {
		cout << endl << "Barcode number" << i + 1 << endl << endl;
		string_string_map barcode_hash = counted_barcodes_hash[i];
		for (const auto &myPair : barcode_hash) {
			cout << "Key:[" << myPair.first << "] Value:["
			     << myPair.second << "]\n";
		}
	}
}

void SequenceFormat::build_regex(string *format_path) {
	// Open the file and handle not found errors
	ifstream format_file;
	format_file.open(*format_path);
	if (!format_file.is_open()) {
		cout << *format_path << " not found" << endl;
		exit(1);
	}

	string format_data;
	string row;
	// Collect all comma separated rows and keep track of how many barcodes
	for (string row; getline(format_file, row);) {
		if (row.front() != '#') {
			format_data.append(row);
		}
	}

	//  cout << format_data << endl;

	regex digit_search("[0-9]+");

	regex barcode_search(R"((\{\d+\})|(\[\d+\])|(\(\d+\))|N+|[ATGC]+)",
			     regex::icase);

	smatch matches;
	auto words_begin = sregex_iterator(format_data.begin(),
					   format_data.end(), barcode_search);
	auto words_end = sregex_iterator();

	barcode_num = 0;
	string regex_string;
	for (sregex_iterator i = words_begin; i != words_end; ++i) {
		bool barcode = false;
		smatch match = *i;
		string match_str = match.str();
		if (match_str.find('[') != string::npos) {
			barcode = true;
			barcodes.push_back("sample");
		}
		if (match_str.find('(') != string::npos) {
			barcode = true;
			barcodes.push_back("random");
		}
		if (match_str.find('{') != string::npos) {
			barcode = true;
			++barcode_num;
			string group_name = "counted_barcode";
			// group_name.append(to_string(barcode_num));
			barcodes.push_back(group_name);
		}
		if (barcode) {
			smatch digits_match;
			regex_search(match_str, digits_match, digit_search);
			string digits = digits_match.str();
			regex_string.append("([ATGCN]{");
			regex_string.append(digits);
			regex_string.append("})");
			for (int i = 0; i < stoi(digits); ++i) {
				format_string.push_back('N');
			}
		} else if (match_str.find('N') != string::npos) {
			regex_string.append("[ATGCN]{");
			regex_string.append(to_string(match_str.size()));
			regex_string.push_back('}');
			format_string.append(match_str);
		} else {
			regex_string.append(match_str);
			format_string.append(match_str);
			constant_size += match_str.size();
		}
	}
	format_regex.assign(regex_string, regex::icase);
	length = format_string.length();
};

void SequenceFormat::print() {
	cout << "-SEQUENCE FORMAT-" << endl;
	cout << format_string << endl;
	cout << "Length: " << length << endl << "Barcodes: " << barcodes[0];
	for (int i = 1; i < barcodes.size(); ++i) {
		cout << ", " << barcodes[i];
	}
	cout << endl;
	for (int i = 0; i < length; ++i) {
		cout << '-';
	}
	cout << endl << endl;
}

void Results::new_results(unordered_set<string> *_sample_seqs) {
	unordered_set<string> &sample_seqs = *_sample_seqs;
	string_int_map empty_map;
	for (const auto &sample_seq : sample_seqs) {
		results.insert(
		    pair<string, string_int_map>(sample_seq, empty_map));
	}
};

void Results::add_count(string sample_barcode, string counted_barcodes) {
	lock_guard<mutex> lg(mtx);
	results[sample_barcode].try_emplace(counted_barcodes, 0);
	++results[sample_barcode][counted_barcodes];
	++correct_counts;
};

void Results::add_constant_error() {
	lock_guard<mutex> lg(mtx_const);
	++constant_errors;
}

void Results::add_sample_barcode_error() {
	lock_guard<mutex> lg(mtx_samp);
	++sample_barcode_errors;
}

void Results::add_counted_barcode_error() {
	lock_guard<mutex> lg(mtx_count);
	++counted_barcode_errors;
}

void Results::print() {
	for (auto &[sample, barcodes] : results) {
		for (auto &[barcode, value] : barcodes) {
			cout << sample << '\t' << barcode << '\t' << value
			     << endl;
		}
	}
};

void Results::print_errors() {
	cout << "Counted:                " << correct_counts << endl;
	cout << "Constant region errors: " << constant_errors << endl;
	cout << "Sample barcode errors:  " << sample_barcode_errors << endl;
	cout << "Counted barcode errors: " << counted_barcode_errors << endl;
};

void Results::to_csv(bool merge, BarcodeConversion barcode_conversion,
		     string outpath, int barcode_num) {
	// Create outfile start
	string file_start = outpath;
	file_start.append(info::current_date());
	file_start.push_back('_');

	vector<string> sample_barcodes;
	vector<string> sample_names;
	for (auto &[sample_barcode, sample_name] :
	     barcode_conversion.samples_barcode_hash) {
		sample_barcodes.push_back(sample_barcode);
		sample_names.push_back(sample_name);
	}
	vector<int> indices(sample_names.size());
	iota(indices.begin(), indices.end(), 0);
	sort(indices.begin(), indices.end(), [&](int A, int B) -> bool {
		return sample_names[A] < sample_names[B];
	});

	string header = "Barcode_1";
	for (auto i = 1; i < barcode_num; ++i) {
		header.append(",Barcode_");
		header.append(to_string(i + 1));
	}
	ofstream merge_file;
	if (merge) {
		string merge_path = file_start;
		merge_path.append("merged.csv");
		merge_file.open(merge_path);
		string merge_header = header;
		for (auto const index : indices) {
			merge_header.push_back(',');
			merge_header.append(sample_names[index]);
		}
		merge_file << merge_header << endl;
	}

	string sample_header = header;
	sample_header.append(",Count");

	unordered_set<string> finished_barcodes;
	for (auto const index : indices) {
		string sample_name = sample_names[index];
		string sample_file_path = file_start;
		sample_file_path.append(sample_name);
		sample_file_path.append(".csv");
		cout << "Writing " << sample_file_path << endl;

		ofstream sample_file;
		sample_file.open(sample_file_path);
		sample_file << sample_header << endl;
		for (auto const &[barcodes, count] :
		     results[sample_barcodes[index]]) {
			size_t pos = 0;
			int barcode_index = 0;
			string barcodes_tmp = barcodes;
			string converted_barcode;
			while ((pos = barcodes_tmp.find(',')) != string::npos) {
				string barcode = barcodes_tmp.substr(0, pos);
				string barcode_name =
				    barcode_conversion
					.counted_barcodes_hash[barcode_index]
							      [barcode];
				converted_barcode.append(barcode_name);
				converted_barcode.push_back(',');
				barcodes_tmp.erase(0, pos + 1);
				++barcode_index;
			}
			string barcode_name =
			    barcode_conversion
				.counted_barcodes_hash[barcode_index]
						      [barcodes_tmp];
			converted_barcode.append(barcode_name);

			sample_file << converted_barcode << ',' << count
				    << endl;
			if (merge) {
				if (finished_barcodes.count(barcodes) == 0) {
					merge_file << converted_barcode;
					for (auto const index : indices) {
						results[sample_barcodes[index]]
						    .emplace(barcodes, 0);
						merge_file
						    << ','
						    << results[sample_barcodes
								   [index]]
							      [barcodes];
					}
					merge_file << endl;
					finished_barcodes.insert(barcodes);
				}
			}
		}
	}
};

string current_date() {
	time_t now = time(0);
	tm *ltm = localtime(&now);
	string date;
	date.append(to_string(ltm->tm_year + 1900));
	date.push_back('-');
	date.append(to_string(ltm->tm_mon + 1));
	date.push_back('-');
	date.append(to_string(ltm->tm_mday));
	return date;
}
