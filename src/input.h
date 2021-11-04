#ifndef input
#define input

#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

/**
 * The object which holds the sequencing reads which are passed between the
 * reader thread and the parsing threads.  Includes mutex locks to remain thread
 * safe.
 */
class Sequences {
       public:
	Sequences(){};

	/// Retrieves the first sequencing read for the parsing threads and
	/// deletes from the queue
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

	/// Pushing a sequencing read from the reader thread to the back
	void push(std::string sequence) {
		std::unique_lock<std::mutex> ulock(mtx);
		// If the queue is over 10,000 reads wait in order to not take
		// up too much memory
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
	// The queue which holds all of the sequencing reads
	std::queue<std::string> seq_queue;
	// Mutex to lock for the queue
	mutable std::mutex mtx;
};

/**
 * Read the fastq and post to the Sequences object.  When finished, it posts
 * "finished" in order to kill the parsing threads
 */
void read_fastq(std::string* fastq_path, Sequences& sequences);

#endif
