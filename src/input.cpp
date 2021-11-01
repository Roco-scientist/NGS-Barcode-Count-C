#include "input.h"

using namespace std;

void read_fastq(string* fastq_path, Sequences& sequences) {
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
		}
		++line_num;
		if (line_num == 5) {
			line_num = 1;
		}
		if (exit_thread) {
			break;
		}
	}
	sequences.push("finished");
}
