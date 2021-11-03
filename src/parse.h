#ifndef parse
#define parse

#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <vector>

#include "input.h"

class SequenceParser {
       public:
	SequenceParser(input::Sequences& sequences) : sequences(sequences) {
		run();
	};
	SequenceParser();

       private:
	input::Sequences& sequences;
	std::string sequence;
	void run();
};

#endif
