#include "input.h"
#include "gzstream.hpp"

using namespace std;

bool hasEnding(std::string const& fullString, std::string const& ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(
				 fullString.length() - ending.length(),
				 ending.length(), ending));
	} else {
		return false;
	}
}

void FastqReader::read() {
	if (hasEnding(*fastq_path, "fastq")) {
		ifstream fastq_file;
		fastq_file.open(*fastq_path);
		if (!fastq_file.is_open()) {
			cerr << *fastq_path << " not found" << endl;
			exit(1);
		}
		for (string line; getline(fastq_file, line);) {
			check_and_post(line);
		}
	} else if (hasEnding(*fastq_path, "fastq.gz")) {
		igzstream in(fastq_path->c_str());
		string line;
		while (getline(in, line)) {
			check_and_post(line);
		}
	} else {
		cerr << "Fastq file needs to end with fastq or fastq.gz";
		exit(1);
	}
	cout << "Total reads:                 " << total_reads << '\r' << endl;
	sequences.push("finished");
}

void FastqReader::check_and_post(string& line) {
	if (line_num == 2) {
		sequences.push(line);
		++total_reads;
		if (total_reads % 1000 == 0) {
			printf("Total reads:                %d\r", total_reads);
			fflush(stdout);
		}
	}
	++line_num;
	if (line_num == 5) {
		line_num = 1;
	}
}
