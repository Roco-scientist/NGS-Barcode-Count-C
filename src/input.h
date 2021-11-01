#ifndef input
#define input

#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <vector>

struct Sequences {
	std::queue<std::string> seq_queue;
	std::mutex mtx;

	std::string retrieve() {
		std::lock_guard<std::mutex> lg(mtx);
		std::string sequence = NULL;
		if (!seq_queue.empty()) {
			sequence = seq_queue.front();
			seq_queue.pop();
		}
		return sequence;
	}

	void push(std::string sequence) {
		std::lock_guard<std::mutex> lg(mtx);
		seq_queue.push(sequence);
	}
};

void read_fastq(std::string* fastq_path, Sequences sequences);

#endif
