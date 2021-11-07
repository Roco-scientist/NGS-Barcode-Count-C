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
	cout << "Total reads:            " << total_reads << '\r' << endl;
	sequences.push("finished");
}

void FastqReader::check_and_post(string& line) {
	if (line_num == 2) {
		sequences.push(line);
		++total_reads;
		if (total_reads % 1000 == 0) {
			printf("Total reads:           %d\r", total_reads);
			fflush(stdout);
		}
	}
	++line_num;
	if (line_num == 5) {
		line_num = 1;
	}
}

/**
 *
		gzFile fastq_file = gzopen(fastq_path->c_str(), "r");
		std::vector<char> buffer(256);
		unsigned pos = 0;
		string line = "start";
		while (!line.empty()) {
			for (;;) {
				if (gzgets(fastq_file, &buffer[pos],
					   buffer.size() - pos) == 0) {
					// end-of-file or error
					int err;
					const char* msg =
					    gzerror(fastq_file, &err);
					if (err != Z_OK) {
						cerr << msg << endl;
					}
					break;
				}
				unsigned read = strlen(&buffer[pos]);
				if (buffer[pos + read - 1] == '\n') {
					if (pos + read >= 2 &&
					    buffer[pos + read - 2] == '\r') {
						pos = pos + read - 2;
					} else {
						pos = pos + read - 1;
					}
					break;
				}
				if (read == 0 ||
				    pos + read < buffer.size() - 1) {
					pos = read + pos;
					break;
				}
				pos = buffer.size() - 1;
				buffer.resize(buffer.size() * 2);
			}
			buffer.resize(pos);
			string line(buffer.begin(), buffer.end());
			cout << line << endl;
			*/
