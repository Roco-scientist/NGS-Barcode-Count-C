#ifndef input
#define input

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Sequences {
       public:
	Sequences(){};
	std::string retrieve() {
		std::scoped_lock<std::mutex> lg(mtx);
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
		std::unique_lock<std::mutex> ulock(mtx);
		if (seq_queue.size() < 10000) {
			seq_queue.push(sequence);

		} else {
			ulock.unlock();
			std::this_thread::sleep_for(
			    std::chrono::milliseconds(10));
			ulock.lock();
			seq_queue.push(sequence);
		}
	}

       private:
	std::queue<std::string> seq_queue;
	mutable std::mutex mtx;
};

void read_fastq(std::string* fastq_path, Sequences& sequences);

#endif
