#include "parse.h"

using namespace std;

void SequenceParser::run() {
	sequence = sequences.retrieve();
	while (sequence != "finished") {
		sequence = sequences.retrieve();
		cout << sequence << endl;
	}

}
