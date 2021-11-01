#ifndef input
#define input

#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <vector>

class Sequences {
       public:
	Sequences(){};
	std::string retrieve() {
		std::lock_guard<std::mutex> lg(mtx);
		std::string sequence = "empty";
		if (!seq_queue.empty()) {
			sequence = seq_queue.front();
			if (sequence != "finished") {
				seq_queue.pop();
			}
		}
		return sequence;
	}

	void push(std::string sequence) {
		std::lock_guard<std::mutex> lg(mtx);
		seq_queue.push(sequence);
	}

       private:
	std::queue<std::string> seq_queue;
	mutable std::mutex mtx;
};

void read_fastq(std::string* fastq_path, Sequences& sequences);

#endif
