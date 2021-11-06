#include "info.h"

using namespace std;

vector<string> take_columns(int num_columns, string row) {
	size_t pos;		     // keep track of the column position
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
	if (barcode_path->empty()) {
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
	// Get the rest of the rows and comma separate
	for (string row; getline(barcode_file, row);) {
		vector<string> row_vec = take_columns(2, row);
		samples_barcode_hash.insert(
		    pair<string, string>(row_vec[0], row_vec[1]));
		samples_seqs.insert(row_vec[0]);
	}
}

void BarcodeConversion::barcode_file_conversion(string *barcode_path) {
	if (barcode_path->empty()) {
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
	int total_barcodes = 0;
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
	for (size_t i = 0; i < row_info.size(); i++) {
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
	for (size_t i = 0; i < counted_barcodes_hash.size(); i++) {
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
	// Collect all lines and make into a string without new line separators
	for (string row; getline(format_file, row);) {
		if (row.front() != '#') {
			format_data.append(row);
		}
	}

	regex digit_search("[0-9]+");  // regex to pull out numbers

	// Regex which finds all barcode types within the format string above
	regex barcode_search(R"((\{\d+\})|(\[\d+\])|(\(\d+\))|N+|[ATGC]+)",
			     regex::icase);

	// setup for a regex iterator through the format string
	auto words_begin = sregex_iterator(format_data.begin(),
					   format_data.end(), barcode_search);
	auto words_end = sregex_iterator();

	barcode_num = 0;
	string regex_string;
	// Find all matches in format string and create a new regex string with
	// capture groups for each DNA barcode.  Also create a string which does
	// not have the captures and replaces all barcodes with N's.  This is
	// used to correct for sequencing errors within the constant regions
	for (sregex_iterator i = words_begin; i != words_end; ++i) {
		bool barcode = false;
		smatch match = *i;
		string match_str = match.str();
		// [#] indicates a sample barcode
		if (match_str.find('[') != string::npos) {
			barcode = true;
			barcodes.push_back("sample");
		}
		// (#) indicates a random barcode
		if (match_str.find('(') != string::npos) {
			barcode = true;
			barcodes.push_back("random");
			random_barcode_included = true;
		}
		// {#} indicates a counted barcode
		if (match_str.find('{') != string::npos) {
			barcode = true;
			++barcode_num;
			barcodes.push_back("counted_barcode");
		}
		// If a barcode was found, create a regex captures of [ATGCN]{#}
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
			// If a barcode was not found, but a string of N's were
			// found, place a non-capture [ATGCN]{# of N's}
			regex_string.append("[ATGCN]{");
			regex_string.append(to_string(match_str.size()));
			regex_string.push_back('}');
			format_string.append(match_str);
		} else {
			// If a string of A,T,G,or C's found, insert those into
			// the regex string for the constant regions
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
	for (size_t i = 1; i < barcodes.size(); ++i) {
		cout << ", " << barcodes[i];
	}
	cout << endl;
	for (size_t i = 0; i < length; ++i) {
		cout << '-';
	}
	cout << endl << endl;
}

void Results::new_results(unordered_set<string> *sample_seqs) {
	// Create a results map with sample:counted_barcodes:count with the last
	// two items empty
	string_int_map empty_int_map;
	string_stringset_map empty_string_map;
	if (sample_seqs->empty()) {
		results.insert(pair<string, string_int_map>(no_sample_barcode,
							    empty_int_map));
		results_random.insert(pair<string, string_stringset_map>(
		    no_sample_barcode, empty_string_map));
	} else {
		for (const auto &sample_seq : *sample_seqs) {
			results.insert(pair<string, string_int_map>(
			    sample_seq, empty_int_map));
			results_random.insert(
			    pair<string, string_stringset_map>(
				sample_seq, empty_string_map));
		}
	}
};

void Results::add_count(string &sample_barcode, string &counted_barcodes,
			string &random_barcode) {
	lock_guard<mutex> lg(mtx);
	if (sample_barcode.empty()) {
		sample_barcode = no_sample_barcode;
	}
	if (random_barcode.empty()) {
		results[sample_barcode].try_emplace(counted_barcodes, 0);
		++results[sample_barcode][counted_barcodes];
		++correct_counts;
	} else {
		results_random[sample_barcode].try_emplace(counted_barcodes,
							   empty_set);
		if (!results_random[sample_barcode][counted_barcodes]
			 .insert(random_barcode)
			 .second) {
			add_duplicate();
		} else {
			++correct_counts;
		}
	}
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

void Results::add_duplicate() {
	lock_guard<mutex> lg(mtx_dup);
	++duplicates;
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
	cout << "Duplicates:             " << duplicates << endl;
};

void Results::to_csv(bool merge, BarcodeConversion _barcode_conversion,
		     string outpath, int barcode_num,
		     bool random_barcode_included) {
	barcode_conversion = _barcode_conversion;
	// Create outfile start
	string file_start = outpath;
	file_start.append(info::current_date());
	file_start.push_back('_');

	// If there wasn't a sample barcode file, fill with 'barcode_count'
	if (barcode_conversion.samples_barcode_hash.empty()) {
		sample_barcodes.push_back(no_sample_barcode);
		sample_names.push_back(no_sample_barcode);
	} else {
		// Otherwise create vectors of sample barcodes and sample names.
		// These are used to sort in the alphabetical order of the
		// sample names
		for (auto &[sample_barcode, sample_name] :
		     barcode_conversion.samples_barcode_hash) {
			sample_barcodes.push_back(sample_barcode);
			sample_names.push_back(sample_name);
		}
	}

	// Create indexes for the sample_barcodes in the alphabetical order of
	// the sample names
	vector<int> indices(sample_names.size());
	iota(indices.begin(), indices.end(), 0);
	sort(indices.begin(), indices.end(), [&](int A, int B) -> bool {
		return sample_names[A] < sample_names[B];
	});

	// Create the file header for the merged file and the sample files.
	// These all start with Barcode_1, Barcode_2...Barcode_N
	string header = "Barcode_1";
	for (auto i = 1; i < barcode_num; ++i) {
		header.append(",Barcode_");
		header.append(to_string(i + 1));
	}

	// Craete the ofstream merge_file object and if merge was called within
	// the arguments, add a header to the merge file
	ofstream merge_file;
	if (merge) {
		string merge_path = file_start;
		merge_path.append("_counts.all.csv");
		merge_file.open(merge_path);
		string merge_header = header;
		for (auto const index : indices) {
			merge_header.push_back(',');
			merge_header.append(sample_names[index]);
		}
		merge_file << merge_header << endl;
	}

	// Create the header for all of the sample count files.  These will be
	// Barcode_1..Barcode_N, Count
	string sample_header = header;
	sample_header.append(",Count");

	// Write the sample files in the order of sample names.  This is doen in
	// order because of the merge file columns need to always be in the same
	// order
	for (auto const index : indices) {
		// Create the sample file path
		string sample_name = sample_names[index];
		string sample_file_path = file_start;
		sample_file_path.append(sample_name);
		sample_file_path.append("_counts.csv");
		cout << "Writing " << sample_file_path << endl;

		// Create the sample file, open, and add the header
		ofstream sample_file;
		sample_file.open(sample_file_path);
		sample_file << sample_header << endl;

		// Writing is different depending on if a random barcode is
		// included.  This is because without the random barcode, the
		// last value in the map is the count.  With the random barcode,
		// the last value holds a set of random barcodes.  This set is
		// counted to find the count
		if (random_barcode_included) {
			write_random(sample_file, merge_file, index, indices,
				     merge);
		} else {
			write_counts(sample_file, merge_file, index, indices,
				     merge);
		}
		sample_file.close();
	}
	merge_file.close();
};

void Results::write_counts(ofstream &sample_file, ofstream &merge_file,
			   int index, vector<int> &indices, bool merge) {
	// For the sample, iterate through the counted_barcodes->counts
	for (auto const &[barcodes, count] : results[sample_barcodes[index]]) {
		string converted_barcode;
		// If there isn't a counted_barcode conversion file, do not
		// convert
		if (barcode_conversion.counted_barcodes_hash.empty()) {
			converted_barcode = barcodes;
		} else {
			// Else convert by comma splitting the counted_barcodes
			// string, convert, then reassemble into a new comma
			// separated string
			size_t pos = 0;
			int barcode_index = 0;
			string barcodes_tmp = barcodes;
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
		}

		// Add the comma separated barcodes and the count to the file
		sample_file << converted_barcode << ',' << count << endl;
		// If merege is called, check to see if the barcodes have
		// already been added to the merge file.  If not, add the
		// counted barcodes, then itereate through each sample and add
		// the count for each in order
		if (merge) {
			if (finished_barcodes.insert(barcodes).second) {
				merge_file << converted_barcode;
				for (auto const index : indices) {
					results[sample_barcodes[index]].emplace(
					    barcodes, 0);
					merge_file
					    << ','
					    << results[sample_barcodes[index]]
						      [barcodes];
				}
				merge_file << endl;
			}
		}
	}
};

void Results::write_random(ofstream &sample_file, ofstream &merge_file,
			   int index, vector<int> &indices, bool merge) {
	// For the sample, iterate through the counted_barcodes->counts
	for (auto const &[barcodes, random_barcodes] :
	     results_random[sample_barcodes[index]]) {
		string converted_barcode;
		// If there isn't a counted_barcode conversion file, do not
		// convert
		if (barcode_conversion.counted_barcodes_hash.empty()) {
			converted_barcode = barcodes;
		} else {
			// Else convert by comma splitting the counted_barcodes
			// string, convert, then reassemble into a new comma
			// separated string
			auto count = random_barcodes.size();
			size_t pos = 0;
			int barcode_index = 0;
			string barcodes_tmp = barcodes;
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
		}

		// Add the comma separated barcodes and the count to the file
		sample_file << converted_barcode << ',' << count << endl;
		// If merege is called, check to see if the barcodes have
		// already been added to the merge file.  If not, add the
		// counted barcodes, then itereate through each sample and add
		// the count for each in order
		if (merge) {
			if (finished_barcodes.insert(barcodes).second) {
				merge_file << converted_barcode;
				for (auto const index : indices) {
					results_random[sample_barcodes[index]]
					    .emplace(barcodes, empty_set);
					merge_file
					    << ','
					    << results_random
						   [sample_barcodes[index]]
						   [barcodes]
						       .size();
				}
				merge_file << endl;
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
	string month = to_string(ltm->tm_mon + 1);
	// If month is only a single number, zero pad so that it will be 06
	// instead of 6
	if (month.size() == 1) {
		string zero_pad = "0";
		zero_pad.append(month);
		month = zero_pad;
	}
	date.append(month);
	date.push_back('-');
	string day = to_string(ltm->tm_mday);
	// If day is only a single number, zero pad so that it will be 06
	// instead of 6
	if (day.size() == 1) {
		string zero_pad = "0";
		zero_pad.append(day);
		day = zero_pad;
	}
	date.append(day);
	return date;
}

string time_passed(chrono::steady_clock::time_point start) {
	// Get total time and output to cout
	chrono::steady_clock::time_point finish = chrono::steady_clock::now();
	string milliseconds = to_string(
	    chrono::duration_cast<chrono::milliseconds>(finish - start)
		.count() %
	    1000);
	// 0 padd milliseconds so it can be used as a fraction seconds
	while (milliseconds.size() < 3) {
		string millisecondes_temp = "0";
		millisecondes_temp.append(milliseconds);
		milliseconds = millisecondes_temp;
	}
	int total_seconds_passed =
	    chrono::duration_cast<chrono::seconds>(finish - start).count();
	int seconds = total_seconds_passed % 60;
	int minutes = (total_seconds_passed % 3600) / 60;
	int hours = total_seconds_passed / 3600;
	string time_passed;
	if (hours > 0) {
		time_passed.append(to_string(hours));
		time_passed.append(" hours ");
	}
	if (minutes > 0) {
		time_passed.append(to_string(minutes));
		time_passed.append(" minutes ");
	}
	time_passed.append(to_string(seconds));
	time_passed.push_back('.');
	time_passed.append(milliseconds);
	time_passed.append(" seconds\n");
	return time_passed;
}

