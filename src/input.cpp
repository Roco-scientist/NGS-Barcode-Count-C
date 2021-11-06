#include "input.h"

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
		bool exit_thread = false;
		ifstream fastq_file;
		fastq_file.open(*fastq_path);
		if (!fastq_file.is_open()) {
			cout << *fastq_path << " not found" << endl;
			exit(1);
		}
		unsigned int line_num = 1;
		unsigned int total_reads = 0;
		for (string row; getline(fastq_file, row);) {
			if (line_num == 2) {
				// cout << row << endl;
				sequences.push(row);
				++total_reads;
				if (total_reads % 1000 == 0) {
					printf("Total reads:           %d\r",
					       total_reads);
					fflush(stdout);
				}
			}
			++line_num;
			if (line_num == 5) {
				line_num = 1;
			}
			if (exit_thread) {
				break;
			}
		}
		cout << "Total reads:            " << total_reads << '\r'
		     << endl;
		sequences.push("finished");
	} else if (hasEnding(*fastq_path, "fastq.gz")) {
		cout << "Gzipped files not yet supported.  Unzip then rerun";
		exit(1);
	} else {
		cout << "Fastq file needs to end with fastq or fastq.gz";
		exit(1);
	}
}
