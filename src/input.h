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
			seq_queue.pop();
		}
		return sequence;
	}

	void push(std::string sequence) {
		std::lock_guard<std::mutex> lg(mtx);
		seq_queue.push(sequence);
	}

	Sequences& operator=(const Sequences& o) {
		if (this != &o) {
			// std::lock(mtx, o.mtx);
			std::lock_guard<std::mutex> lhs_lk(mtx,
							   std::adopt_lock);
			std::lock_guard<std::mutex> rhs_lk(o.mtx,
							   std::adopt_lock);
			seq_queue = o.seq_queue;
		}
		return *this;
	}

       private:
	std::queue<std::string> seq_queue;
	mutable std::mutex mtx;
};

void read_fastq(std::string* fastq_path, Sequences& sequences);

#endif
