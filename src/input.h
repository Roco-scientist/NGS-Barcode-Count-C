#ifndef input
#define input

#include <fstream>
#include <vector>

struct FastqLineReader {
	bool test;
	size_t line_num;
	size_t total_reads;
	std::string raw_sequence_read;
	std::vector<std::string> sequences;
	bool exit;

	void read_lines(std::string* fastq_path);
};

#endif
